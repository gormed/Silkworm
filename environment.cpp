
/*

    environment.cpp

    class for static environments

*/


#include <GL3/gl3w.h>
#include <stdio.h>

#include "file.h"
#include "xml.h"

#include "environment.h"


Environment::Environment()
{
    // load the image resources defined in imageresources.xml

    File imageresourcesfile = File::read("data/imageresources.xml");
    XML imageresources = XML::readfile( imageresourcesfile);

    NodeMap nm = imageresources.get("image");
    NodeMap::iterator np;
    for(np=nm.begin();np!=nm.end();++np)
    {
        File imagefile = File::read((np->second)["source"].c_str());
        Image img;

        img.read(imagefile);
        img.link();
        img.kill();

        images[(np->second)["name"]]=img;
    }


    // initialise an opengl render state defined in simplestate.xml

	state = new State();

	File statefile = File::read("states/lightmap.xml");

	state->read( statefile );
	state->link();


    // load the models from the tileset.dae collada file

	File colladafile = File::read("models/tileset.dae");

	collada = new Collada();

	collada->state = state;
	collada->read( colladafile );


}

Environment::~Environment()
{
    state->unlink();

    delete state;
    delete collada;
}

void Environment::render( const Matrix &projection, const Matrix &camera, int lightmap )
{
    Matrix modelview;

    // set rendering state

    state->set();

    // load the projection matrix

    state->uniformMatrix("projection",projection);

    // use texture unit 1 for the lightmaps

    glUniform1i(glGetUniformLocation(state->program,"lightmap"),1);
    glUniform1i(glGetUniformLocation(state->program,"emissive"),2);

    images["black"].use(1);

    // iterate through models and draw each

    EnvironmentModelInstanceList::iterator li;
    int i=0;

    for(li=models.begin();li!=models.end();++li)
    {
        modelview = camera * (li->transform);

        state->uniformMatrix("modelview",modelview);

        if (lightmap>=0)
        {
            lightmaps[i].use(1);
        }

        //images["checkers"].use(1);


        (li->geo)->draw(images);

        i++;
    }

}

void grow(unsigned char *pixel)
{
    // increase filled areas in a lightmap by 1 pixel
    // to account for wrong colors at face edges

    int x,y,i,j;
    int ofs[2]={1,LIGHTMAPSIZE};

    for(y=0;y<LIGHTMAPSIZE;y++)
    for(x=0;x<LIGHTMAPSIZE;x++)
    {
        if( pixel[ (y*LIGHTMAPSIZE+x)*4+3 ] == 0 )
        for(i=0;i<2;i++)
        {
            if(pixel[ ((y*LIGHTMAPSIZE+x+ofs[i])&(LIGHTMAPSIZE*LIGHTMAPSIZE-1))*4+3 ] != 0)
            for(j=0;j<4;j++)
            {
                pixel[ (y*LIGHTMAPSIZE+x)*4+j ] = pixel[ ((y*LIGHTMAPSIZE+x+ofs[i])&(LIGHTMAPSIZE*LIGHTMAPSIZE-1))*4+j ];
            }
        }
    }

    for(y=LIGHTMAPSIZE-1;y>=0;y--)
    for(x=LIGHTMAPSIZE-1;x>=0;x--)
    {
        if( pixel[ (y*LIGHTMAPSIZE+x)*4+3 ] == 0 )
        for(i=0;i<2;i++)
        {
            if(pixel[ ((y*LIGHTMAPSIZE+x-ofs[i])&(LIGHTMAPSIZE*LIGHTMAPSIZE-1))*4+3 ] != 0)
            for(j=0;j<4;j++)
            {
                pixel[ (y*LIGHTMAPSIZE+x)*4+j ] = pixel[ ((y*LIGHTMAPSIZE+x-ofs[i])&(LIGHTMAPSIZE*LIGHTMAPSIZE-1))*4+j ];
            }
        }
    }
}

int Environment::readCachedLightmaps(int bounce, unsigned char *pixel)
{

    FILE *f=fopen("lmcache.raw","rb");
    if(!f) return false;

    // iterate through all the models

    EnvironmentModelInstanceList::iterator li;
    MeshMap::iterator mp;

    int count=0;

    for(li=models.begin();li!=models.end();++li)
    {
        fread(pixel,LIGHTMAPSIZE*LIGHTMAPSIZE*4,1,f);

        lightmaps[count].create(LIGHTMAPSIZE,LIGHTMAPSIZE,Image::RGBA,pixel);
        lightmaps[count].link();

        count++;
    }

    fclose(f);

    return true;
}

void Environment::bakeLightmaps(int bounce)
{
    static unsigned char RTpixel[LIGHTMAPRTSIZE*LIGHTMAPRTSIZE*4];
    static unsigned char pixel[LIGHTMAPSIZE*LIGHTMAPSIZE*4];

    int i,j;

    // count the models

    int count = models.size();

    // allocate memory for the lightmap images

    lightmaps=new Image[count*ENVLIGHTBOUNCES];

    if (readCachedLightmaps(bounce,pixel)) return;


    FILE *fcache = fopen("lmcache.raw","wb");

    // create the render target for lightmap baking

    Image RTcolor; RTcolor.create ( LIGHTMAPRTSIZE, LIGHTMAPRTSIZE, Image::RGBA, NULL );
    Image RTdepth; RTdepth.create ( LIGHTMAPRTSIZE, LIGHTMAPRTSIZE, Image::DEPTH, NULL );

    int RT;

    RTcolor.link();

    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

    //glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    //glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);

    RTdepth.link();

    glGenFramebuffers(1, (GLuint*)&RT);
    glBindFramebuffer(GL_FRAMEBUFFER, RT);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,  GL_TEXTURE_2D, RTdepth.getTexture(), 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, RTcolor.getTexture(), 0);

    glViewport(0,0,LIGHTMAPRTSIZE,LIGHTMAPRTSIZE);

    glClearColor(0.1f,0.1f,0.1f,1.0f);                  // r,g,b,a for screen clear (sky color)
    glClearDepth(1.0f);                                 // z for screen clear

    Matrix projection = Matrix::projection(-0.1f,0.1f,-0.1f,0.1f,0.1f,50.0f);
    Matrix camera,tcam;

    // iterate through all the models

    EnvironmentModelInstanceList::iterator li;
    MeshMap::iterator mp;

    count=0;

    for(li=models.begin();li!=models.end();++li)
    {
        for(i=0;i<LIGHTMAPSIZE*LIGHTMAPSIZE*4;i++) pixel[i]=0;

        // iterate through all the meshes in the models geometry

        //if (count<50)

        for(mp=li->geo->meshes.begin();mp!=li->geo->meshes.end();++mp)
        {

            // iterate through all the triangles in each mesh

            Mesh &m = (mp->second);

            for(i=0;i<m.nElements;i++)
            {
                // software rasterization of the triangle
                // in uv space

                // no clipping is performed, ignore all triangles
                // that are seriously too large

                int ignore;

                do
                {
                    ignore=false;

                    for(j=0;j<3;j++)
                    {
                        if(  m.uvs[i*3+j].e[0]<-1.0f||m.uvs[i*3+j].e[0]>2.0f
                           ||m.uvs[i*3+j].e[1]<-1.0f||m.uvs[i*3+j].e[1]>2.0f) ignore=true;
                    }

                    if (ignore)
                    {
                        printf("/");
                        i++;
                        if(i>=m.nElements) break;
                    }
                } while(ignore);

                if(i>=m.nElements) break;


                printf(".");

                // find vertical dimensions of the triangle

                int bottom=LIGHTMAPSIZE;
                int top=0;

                for(j=0;j<3;j++)
                {
                    int t = (int) (m.uvs[i*3+j].e[1]*LIGHTMAPSIZE);
                    if (t<bottom) bottom=t;
                    if (t>top) top=t;
                }

                // divide the triangle into horizontal spans

                int t,s;

                for(t=bottom;t<=top;t++)
                {

                    // calculate the extents of the span

                    int left = LIGHTMAPSIZE;
                    int right = 0;

                    Vector posLeft,posRight;
                    Vector nmlLeft,nmlRight;

                    for (j=0;j<3;j++)
                    {
                        float t0 = m.uvs[i*3+j].e[1]*(float)LIGHTMAPSIZE;
                        float t1 = m.uvs[i*3+((j+1)%3)].e[1]*(float)LIGHTMAPSIZE;

                        if( (t0>t) != (t1>t) )
                        {
                            // this triangle edge intersects the span
                            // calculate s coordinate of intersection

                            float s0 = m.uvs[i*3+j].e[0]*(float)LIGHTMAPSIZE;
                            float s1 = m.uvs[i*3+((j+1)%3)].e[0]*(float)LIGHTMAPSIZE;

                            float ip=(t-t0)/(t1-t0);
                            s = (int)(ip*(s1-s0)+s0);


                            if(s<left)
                            {
                                left=s;
                                posLeft = m.vertices[i*3+j]*(1.0f-ip)
                                        + m.vertices[i*3+((j+1)%3)]*ip;
                                nmlLeft = m.normals[i*3+j]*(1.0f-ip)
                                        + m.normals[i*3+((j+1)%3)]*ip;
                            }

                            if(s>right)
                            {
                                right=s;
                                posRight = m.vertices[i*3+j]*(1.0f-ip)
                                        + m.vertices[i*3+((j+1)%3)]*ip;
                                nmlRight = m.normals[i*3+j]*(1.0f-ip)
                                        + m.normals[i*3+((j+1)%3)]*ip;
                            }
                        }
                    }

                    // iterate through each pixel in the span

                    posLeft = li->transform*(posLeft);
                    posRight = li->transform*(posRight);
                    nmlLeft = li->rotation*(nmlLeft);
                    nmlRight = li->rotation*(nmlRight);

                    Vector pos=posLeft;
                    Vector nml=nmlLeft;
                    Vector posIP=(posRight-posLeft)*(1.0f/(float)(right-left));
                    Vector nmlIP=(nmlRight-nmlLeft)*(1.0f/(float)(right-left));

                    for (s=left;s<=right;s++)
                    {
                        // inner loop of the software rasterization
                        // s and t hold integer position of the pixel in the light map

                        // pos and nml hold world space position
                        // of the lightmap pixel


                        // for each lightmap pixel in every triangle in every lightmap
                        // in every mesh in every model, :)

                        // render the scene using hardware rendering
                        // to the render target held in RT

                        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);   // clear render target

                        // the projection matrix has already been set up
                        // but the camera matrix needs to be calculated
                        // the camera should be positioned on the triangles surface
                        // and look away in direction of the normal

                        Vector a,b,c,up;

                        // lock countermeasure

                        if (  nml.e[0]*nml.e[0]>nml.e[1]*nml.e[1]
                            &&nml.e[0]*nml.e[0]>nml.e[2]*nml.e[2]) up=Vector(0.0f,1.0f,0.0f); else up=Vector(1.0f,0.0f,0.0f);

                        a = nml*up;
                        b = nml*a;
                        c = nml;

                        a.normalize();
                        b.normalize();
                        c.normalize();

                        tcam.e[0] = a.e[0]; tcam.e[1]=b.e[0]; tcam.e[2]=-c.e[0];tcam.e[3]=0.0f;
                        tcam.e[4] = a.e[1]; tcam.e[5]=b.e[1]; tcam.e[6]=-c.e[1];tcam.e[7]=0.0f;
                        tcam.e[8] = a.e[2]; tcam.e[9]=b.e[2]; tcam.e[10]=-c.e[2];tcam.e[11]=0.0f;

                        tcam.e[12] = 0.0f; tcam.e[13]=0.0f;tcam.e[14]=0.0f;tcam.e[15]=1.0f;

                        tcam = tcam* Matrix::translation(Vector(0,0,0)-pos) ;

                        //tcam = tcam * li->inverseTransform;

                        // render the scene
                        // the id of the lightmap used to render the scene
                        // is equal to the id of the light bounce to be computed - 1

                        render(projection,tcam,bounce-1);

                        // now, read back the data from the render texture

                        glReadBuffer(GL_COLOR_ATTACHMENT0);
                        glReadPixels(0, 0, LIGHTMAPRTSIZE, LIGHTMAPRTSIZE, GL_BGRA, GL_UNSIGNED_BYTE, RTpixel);

                        // accumulate all the color info in the lightmap

                        int colAcc[4]={0,0,0,0};

                        for(int px=0;px<LIGHTMAPRTSIZE*LIGHTMAPRTSIZE*4;px++)
                            colAcc[px&3]+=RTpixel[px];

                        for(int cmp=0;cmp<4;cmp++)
                        {
                            // normalize intensity range
                            colAcc[cmp]/=(LIGHTMAPRTSIZE*LIGHTMAPRTSIZE);
                        }


                        // set alpha to 255

                        colAcc[3]=0xff;


                        // as the final step of lightmap rasterization,
                        // store the accumulated light into the lightmap
                        // change format from BGRA to RGBA (glReadPixels is faster with BGRA)

                        int swizzle[4]={2,1,0,3};

                        for(int cmp=0;cmp<4;cmp++)
                        {
                            pixel[ ( ( (t) &(LIGHTMAPSIZE-1))*LIGHTMAPSIZE+(s&(LIGHTMAPSIZE-1)))*4 + cmp ]=colAcc[swizzle[cmp]];
                        }


                        // interpolate worldspace position and normal
                        pos+=posIP;
                        nml+=nmlIP;
                    }

                }
            }
        }

        // the lightmap for this model has been generated and stored in "pixel",
        // and the only step left to be done is to create the image object

        //if (count==50)for(i=0;i<LIGHTMAPSIZE*LIGHTMAPSIZE*4;i++) pixel[i]=0;

        grow(pixel);
        grow(pixel);

        fwrite(pixel,LIGHTMAPSIZE*LIGHTMAPSIZE*4,1,fcache);

        lightmaps[count].create(LIGHTMAPSIZE,LIGHTMAPSIZE,Image::RGBA,pixel);
        lightmaps[count].link();

        count++;
        printf("%i\n",count);
    }

    fclose(fcache);





    // destroy the rendertarget and revert to rendering to standard frame buffer

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    RTcolor.unlink();
    RTdepth.unlink();

    glDeleteFramebuffers(1,(GLuint*)&RT);


}
