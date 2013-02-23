
/*

    collada.cpp

    class for importing collada files


*/


#include <sstream>

#include "xml.h"
#include "collada.h"
#include "log.h"

void Collada::read(File &file)
{
    XML xml = XML::readfile(file);

    NodeMap::iterator p;

    // make sure there is a state to assemble against

    if (!state) throw(1);

    // find the geometry info in the collada file

    NodeMap library_geometries = xml.get("library_geometries");

    NodeMap geometry = library_geometries[0].get("geometry");

    for (p=geometry.begin();p!=geometry.end();++p)
    {
        // check wether skinning info exists

        XML *skinxml = NULL;

        NodeMap skin = xml.get("library_controllers")[0].get("controller")[0].get("skin");

        if (skin[0]["source"]=="#"+(p->second)["id"])
        {
            // read skinning info

            skinxml = &skin[0];

            // note that the skin import code is a hack
            // it will only work in certain special cases!

            readSkin(skinxml,xml);
        }

        // read each geometry object

        readGeometry( (p->second), skinxml );
    }



}

void Collada::readGeometry(XML &geoxml, XML *skinxml)
{
    int i,j,k,l;

    int source_enabled;
    int source_total;

    std::string source[3];
    float *source_array[3]={0,0,0};
    int source_stride[3];
    int source_count[3];
    int source_offset[3];

    int poly_count;
    int poly_count_total;

    int *poly_vcount;
    int *poly_p;

    int finaldata_total;
    int finaldata_elementlength;

    int meshcount=0;

    NodeMap::iterator p,p2,p3,p4;

    // create a new geometry object for storage in the geometry map

    Geometry geometry;

    int *skin_v=0;

    if (skinxml)
    {
        // if there is skinning data, fill skin_v

        int skin_count;
        int skin_vcount;
        int skin_skip;

        std::stringstream skin_count_stream ( skinxml->get("vertex_weights")[0]["count"] );
        skin_count_stream >> skin_count;

        skin_v = new int[skin_count];
        std::stringstream skin_vcount_stream ( skinxml->get("vertex_weights")[0].get("vcount")[0].value() );
        std::stringstream skin_v_stream ( skinxml->get("vertex_weights")[0].get("v")[0].value() );

        for(i=0;i<skin_count;i++)
        {
            skin_vcount_stream >> skin_vcount;

            if(skin_vcount>0)
            {

                skin_v_stream >> skin_v[i]; // only save 1 joint per vertex
                skin_v_stream >> skin_skip; // skip weight

                for(j=1;j<skin_vcount;j++)
                {
                    skin_v_stream >> skin_skip; // skip additional joints
                    skin_v_stream >> skin_skip; // skip additional weights
                }

            }
            else
            {
                skin_v[i]=0;
            }
        }
    }

    // retrieve the polylists

    NodeMap meshmap = geoxml.get("mesh");

    NodeMap polylist = meshmap[0].get("polylist");

    // iterate through the polylists

    for (p=polylist.begin();p!=polylist.end();++p)
    {
        source_enabled = 0;
        source_total = 0;

        poly_count = 0;
        poly_count_total = 0;

        finaldata_total = 0;
        finaldata_elementlength = 0;



        // for each polylist, create a mesh for storage in the mesh map

        Mesh mesh;

        // store the material name for the polylist

        mesh.material = (p->second)["material"];

        // initialize the array

        mesh.array.usestate = this->state;

        // using the polylist entries, identify the source entries

        NodeMap inputs = (p->second).get("input");

        for(p2=inputs.begin();p2!=inputs.end();++p2)
        {
            if ((p2->second)["semantic"]=="VERTEX")
            {
                std::stringstream ofss((p2->second)["offset"]);
                ofss >> source_offset[0];

                NodeMap vertices = meshmap[0].get("vertices");
                for (p3=vertices.begin();p3!=vertices.end();++p3)
                if("#"+(p3->second)["id"]==(p2->second)["source"])
                {
                    NodeMap vertexInputs=(p3->second).get("input");
                    for(p4=vertexInputs.begin();p4!=vertexInputs.end();++p4)
                    {
                        if((p4->second)["semantic"]=="POSITION")
                        {
                            source_enabled|=1;     // set flag
                            source_total++;        // count number of sources for calculating memory usage

                            source[0]=(p4->second)["source"];
                        }
                    }


                }
            }

            if ((p2->second)["semantic"]=="NORMAL")
            {
                std::stringstream ofss((p2->second)["offset"]);
                ofss >> source_offset[1];

                source_enabled|=2;  // set flag
                source_total++;     // count number of sources for calculating memory usage

                source[1] = (p2->second)["source"];
            }

            if ((p2->second)["semantic"]=="TEXCOORD")
            {
                std::stringstream ofss((p2->second)["offset"]);
                ofss >> source_offset[2];

                source_enabled|=4;  // set flag
                source_total++;     // count number of sources for calculating memory usage

                source[2] = (p2->second)["source"];
            }

        }

        // find  the source entries

        NodeMap sources = meshmap[0].get("source");

        // extract the data from the source entries

        for(p2=sources.begin();p2!=sources.end();++p2)
        for(i=0;i<3;i++)
        if (source_enabled&(1<<i)) // check flag
        if ("#"+(p2->second)["id"]==source[i])
        {
            std::string arrayText = (p2->second).get("float_array")[0].value();

            NodeMap accessor = (p2->second).get("technique_common")[0].get("accessor");

            std::stringstream countStream (accessor[0]["count"]);
            std::stringstream strideStream(accessor[0]["stride"]);

            countStream >> source_count[i];
            strideStream >> source_stride[i];

            std::stringstream arrayStream(arrayText);

            source_array[i]=new float[source_count[i]*source_stride[i]];
            for (j=0;j<source_count[i]*source_stride[i];j++) arrayStream >> source_array[i][j];
        }

        // from the polylist, extract the poly data

        std::stringstream polycountStream((p->second)["count"]);
        polycountStream >> poly_count;

        std::stringstream vcountStream((p->second).get("vcount")[0].value());
        std::stringstream pStream((p->second).get("p")[0].value());

        poly_vcount = new int[poly_count];

        for(i=0;i<poly_count;i++)
        {
            vcountStream >> poly_vcount[i];
            poly_count_total += poly_vcount[i];
            if (poly_vcount[i]==3) finaldata_total+=3;
            else if(poly_vcount[i]==4) finaldata_total+=6; // quads will be assembled from 2 triangles
            else throw(1);
        }

        poly_p = new int[poly_count_total*source_total];

        for(i=0;i<poly_count_total*source_total;i++) pStream >> poly_p[i];

        // all data for this polylist has been fetched,
        // assemble the polylist into a nice vertex array format
        // matching the renderstate to be used

        for (i=0;i<mesh.array.usestate->getSymcount();i++) finaldata_elementlength+=mesh.array.usestate->getLength(i);

        mesh.array.data = new float[finaldata_total*finaldata_elementlength];
        mesh.array.elements = finaldata_total;

        int dataindex=0;
        int sourceindex=0;

        const int assemblerlist[6]={0,1,2,0,2,3};    // 3 entries for triangle, 6 entries for quad
        const int assemblerlistend[2]={3,6};         // end of triangle, end of quad in list

        // loop through source arrays (in this order: vertex, normal, texcoord)

        for(i=0;i<mesh.array.usestate->getSymcount();i++)
        {
            if (source_enabled&(1<<i)) // for this symbol, there is a source in the file
            {

                sourceindex=0;

                // loop through polygons

                for(j=0;j<poly_count;j++)
                {
                    // loop through vertices,
                    // break quads up into tris

                    for(l=0;l<assemblerlistend[poly_vcount[j]-3];l++)
                    {
                        // loop through the components of this vertex / normal

                        for(k=0;k<mesh.array.usestate->getLength(i);k++)
                        {
                            if (k<source_stride[i])
                            {
                                // this component is contained in the source

                                int t1=poly_p[ (sourceindex+assemblerlist[l])*source_total + source_offset[i] ];

                                mesh.array.data[dataindex]=source_array[i][t1*source_stride[i]+k];
                                dataindex++;
                            }
                            else
                            {
                                // the format given by the render state has to many components,
                                // this component is not contained in the source
                                // default to 1.0

                                mesh.array.data[dataindex]=1.0f;
                                dataindex++;
                            }
                        }
                    }
                    sourceindex+=poly_vcount[j];
                }
            }
            else if (mesh.array.usestate->getLength(i)==1&&skin_v)
            {
                // assume this symbol to be a joint index for skinning
                // and copy the skin_v table, if any has been created

                sourceindex = 0;

                for(j=0;j<poly_count;j++)
                {
                    // loop through vertices,
                    // break quads up into tris

                    for(l=0;l<assemblerlistend[poly_vcount[j]-3];l++)
                    {

                        int t1=poly_p[ (sourceindex+assemblerlist[l])*source_total + source_offset[0] ];

                        mesh.array.data[dataindex]=*(float*)&skin_v[t1]; // conversion to float

                        if(*(int*)&mesh.array.data[dataindex] <0) throw(1);

                        dataindex++;


                    }

                    sourceindex+=poly_vcount[j];

                }


            }
            else
            {
                // there is a symbol in the render state
                // for which no corresponding source in the file exists.
                // default to 0.0 for all vertices

                for (j=0;j<finaldata_total*mesh.array.usestate->getLength(i);j++)
                {
                    mesh.array.data[dataindex]=1.0f;
                    dataindex++;
                }

            }
        }

        // the member variable "data" now has been initialized
        // and the Array resource is ready for linking

        mesh.array.link();

        // free "data", since the model now already resides on gpu.

        delete []mesh.array.data;

        // clean up

        if (skin_v) delete []skin_v;

        delete []poly_p;
        delete []poly_vcount;
        for (i=0;i<3;i++) if (source_array[i]) delete []source_array[i];

        // finally, store the mesh into the mesh map
        // this must be done last because the object is copied into the map,
        // not referenced.

        geometry.meshes[meshcount] = mesh;
        meshcount++;

    }

    // finally, store the geometry into the geometry map
    // this must be done last because the object is copied into the map,
    // not referenced.

    this->geometries[geoxml["name"]]=geometry;

}

void Geometry::draw(ImageMap &materials)
{
    MeshMap::iterator mp;

    for(mp=meshes.begin();mp!=meshes.end();++mp)
    {
        materials[(mp->second).material].use(0);
        (mp->second).array.draw();
    }
}

void Collada::readSkin(XML *skinxml, XML &xml)
{
    int i,j;
    NodeMap::iterator p;

    // initialize the armature object

    armature = new Armature();


    // get the bind shape matrix

    std::stringstream bsmStream ( skinxml->get("bind_shape_matrix")[0].value() );
    for (i=0;i<16;i++) bsmStream >> armature->bindShape.e[i];

    // collada stores matrices in row-major format, transponse
    armature->bindShape=Matrix::flip(armature->bindShape);


    // get bone count

    std::stringstream bonecountStream ( skinxml->get("source")[0].get("Name_array")[0]["count"] );
    bonecountStream >> armature->nBones;
    armature->nBones++; // account for the root bone

    armature->bones = new Bone[armature->nBones];


    // get the inverse bind pose matrices

    std::stringstream ibpStream ( skinxml->get("source")[1].get("float_array")[0].value() );

    for (i=1;i<armature->nBones;i++)
    {
        for (j=0;j<16;j++) ibpStream >> armature->bones[i].inverseBindPose.e[j];

        // collada stores matrices in row-major format, transponse
        armature->bones[i].inverseBindPose=Matrix::flip(armature->bones[i].inverseBindPose);
    }



    // locate the animation library

    NodeMap animations = xml.get("library_animations")[0].get("animation");

    // locate the node hierarchy

    NodeMap visual_scene = xml.get("library_visual_scenes")[0].get("visual_scene");

    NodeMap nodes = visual_scene[0].get("node");

    // read the bone hierarchy

    int boneindex=0;

    for (p=nodes.begin();p!=nodes.end();++p)
    if ((p->second)["id"]=="Armature")
    {
        readBone( (p->second).get("node")[0] , animations, armature, boneindex, NULL );
    }

}


void Collada::readBone( XML &boneXML, NodeMap &animations, Armature *armature, int &boneindex, Bone *parent)
{
    int i,j;

    // save hierarchy

    Bone *thisbone = &armature->bones[boneindex];
    thisbone->parent = parent;


    // read the base matrix

    std::stringstream matrixStream (boneXML.get("matrix")[0].value());

    for(i=0;i<16;i++) matrixStream >> thisbone->baseTransform.e[i];

    // collada stores matrices in row-major format, transponse
    thisbone->baseTransform = Matrix::flip(thisbone->baseTransform);



    // find and read the animation matrices


    NodeMap::iterator p;

    for(p=animations.begin();p!=animations.end();++p)
    if ((p->second)["id"] == "Armature_"+boneXML["id"]+"_pose_matrix")  // HACK
    {
         if (boneindex==0)
         {
             // this is the first bone, get animation count (HACK)

             std::stringstream aniCountStream ( (p->second).get("source")[1].get("technique_common")[0].get("accessor")[0]["count"] );
             aniCountStream >> armature->nAnimations;

         }

         thisbone->aniTransform = new Matrix[armature->nAnimations];

         std::stringstream aniMatrixStream ( (p->second).get("source")[1].get("float_array")[0].value() );

         for(i=0;i<armature->nAnimations;i++)
         {
             for(j=0;j<16;j++) aniMatrixStream >> thisbone->aniTransform[i].e[j];

             // collada stores matrices in row-major format, transponse
             thisbone->aniTransform[i]=Matrix::flip(thisbone->aniTransform[i]);
         }

         break;
    }


    // read children

    boneindex++;

    NodeMap children = boneXML.get("node");
    for(p=children.begin();p!=children.end();++p)
    {
        readBone( (p->second), animations, armature, boneindex, thisbone);
    }

}

