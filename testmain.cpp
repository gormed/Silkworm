
#include <GL3/gl3w.h>
#include <list>

#include "log.h"
#include "osabs.h"
#include "gamepad.h"
#include "text.h"

#include "vector.h"
#include "matrix.h"

#include "image.h"

#include "collada.h"

#include "entity.h"


enum GameState { EDITOR, GAME };
GameState gamestate = EDITOR;


    char editortext[64*32] =

    "welcome to the druid gameplay prototype                         "
    "you are in level editor mode                                    "
    "                                                                "
    "to move the editing cursor, use     [a] [w] [s] [d] [r] [f]     "
    "to select a tile for painting, use  [arrow up] [arrow down]     "
    "to rotate the selected tile, use    [arrow left] [arrow right]  "
    "to paint, use                       [space bar]                 "
    "to erase, use                       [delete]                    "
    "                                                                "
    "multiple tiles can be painted into one block.                   "
    "use [F2] to save the level and [F3] to load it back.            "
    "use the mouse for navigation. hit [enter] to compile and play.  ";

    char gametext[64*32] =

    "hit [enter] to leave play mode                                  ";


int windowwidth=1024,windowheight=768;

void windowsize(int w, int h)
{
    windowwidth=w;
    windowheight=h;
}

#define LEVELWIDTH 32
#define LEVELHEIGHT 32
#define LEVELDEPTH 8

struct EditorCursor
{
    Vector scrollPosition;
    float lookx,looky;
    Vector ipPosition;
    int position[3];
    float ipRotation;
    int rotation;
    GeometryMap::iterator tileused;
    GeometryMap *tiles;
};

EditorCursor editorcursor = { Vector(0.0f,0.0f,-20.0f), 0.0f, 0.0f, Vector(0.0f,0.0f,0.0f), { 0, 0, 0 }, 0.0f, 0 };

struct LevelTileDef
{
    std::string name;
    int rotation;
};

typedef std::list<LevelTileDef> LevelTile;

LevelTile level[LEVELWIDTH*LEVELDEPTH*LEVELHEIGHT];
unsigned char collisionLevel[LEVELWIDTH*LEVELDEPTH*LEVELHEIGHT];

typedef std::map<std::string,std::string> TilesetMap;

DruidEntity testdruid;
int lastkeystatearray[16] = { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 };
int keystatearray[16] = { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 };


inline int collisionLevelAt(Vector v)
{
    return collisionLevel[(int)(v.e[0]*0.5f)+(int)(v.e[1]*0.5f)*LEVELWIDTH+(int)(v.e[2]*0.5f)*LEVELWIDTH*LEVELHEIGHT];
}

void savelevel();
void loadlevel();


int collide(Entity *e)
{
   int cflag = 0;

   float colEps = 0.1f;


   // this code only works if,
   // - neither of the bounding box's side lengths are > 1 tile length
   // - movement speed is not > colEps
   // - neither of the bounding box's side legnths are < 2 * colEps

   // this code is fucking ugly and needs to be simplified


   // codes returned from collisonLevelAT:
   // bit 0 : block
   // bit 1 : platform at bottom
   // bit 2 : platform at top

   // add epsilon to account for contact-collision

   int vposr0=(int)(e->pos.e[1]*0.5f);
   int vposr1=(int)( (e->pos.e[1]+e->bba.e[1]-colEps) *0.5f );
   int vposr2=(int)( (e->pos.e[1]+e->bbb.e[1]+colEps) *0.5f );

   if (    (collisionLevelAt(e->pos+Vector(e->bba.e[0]-colEps,e->bba.e[1]+colEps,e->bba.e[2]+colEps))&1)
        || (collisionLevelAt(e->pos+Vector(e->bba.e[0]-colEps,e->bbb.e[1]-colEps,e->bba.e[2]+colEps))&1)
        || (collisionLevelAt(e->pos+Vector(e->bba.e[0]-colEps,e->bba.e[1]+colEps,e->bbb.e[2]-colEps))&1)
        || (collisionLevelAt(e->pos+Vector(e->bba.e[0]-colEps,e->bbb.e[1]-colEps,e->bbb.e[2]-colEps))&1)  ) cflag|=1;

   if (    (collisionLevelAt(e->pos+Vector(e->bbb.e[0]+colEps,e->bba.e[1]+colEps,e->bba.e[2]+colEps))&1)
        || (collisionLevelAt(e->pos+Vector(e->bbb.e[0]+colEps,e->bbb.e[1]-colEps,e->bba.e[2]+colEps))&1)
        || (collisionLevelAt(e->pos+Vector(e->bbb.e[0]+colEps,e->bba.e[1]+colEps,e->bbb.e[2]-colEps))&1)
        || (collisionLevelAt(e->pos+Vector(e->bbb.e[0]+colEps,e->bbb.e[1]-colEps,e->bbb.e[2]-colEps))&1)  ) cflag|=2;

    if ( vposr0 != vposr1 )
    if (   (collisionLevelAt(e->pos+Vector(e->bba.e[0]+colEps,e->bba.e[1]-colEps,e->bba.e[2]+colEps))&4)
        || (collisionLevelAt(e->pos+Vector(e->bbb.e[0]-colEps,e->bba.e[1]-colEps,e->bba.e[2]+colEps))&4)
        || (collisionLevelAt(e->pos+Vector(e->bba.e[0]+colEps,e->bba.e[1]-colEps,e->bbb.e[2]-colEps))&4)
        || (collisionLevelAt(e->pos+Vector(e->bbb.e[0]-colEps,e->bba.e[1]-colEps,e->bbb.e[2]-colEps))&4)  ) cflag|=4;

    if ( vposr0 != vposr2 )
    if (   (collisionLevelAt(e->pos+Vector(e->bba.e[0]+colEps,e->bbb.e[1]+colEps,e->bba.e[2]+colEps))&2)
        || (collisionLevelAt(e->pos+Vector(e->bbb.e[0]-colEps,e->bbb.e[1]+colEps,e->bba.e[2]+colEps))&2)
        || (collisionLevelAt(e->pos+Vector(e->bba.e[0]+colEps,e->bbb.e[1]+colEps,e->bbb.e[2]-colEps))&2)
        || (collisionLevelAt(e->pos+Vector(e->bbb.e[0]-colEps,e->bbb.e[1]+colEps,e->bbb.e[2]-colEps))&2)  ) cflag|=8;

    if (   (collisionLevelAt(e->pos+Vector(e->bba.e[0]+colEps,e->bba.e[1]+colEps,e->bba.e[2]-colEps))&1)
        || (collisionLevelAt(e->pos+Vector(e->bbb.e[0]-colEps,e->bba.e[1]+colEps,e->bba.e[2]-colEps))&1)
        || (collisionLevelAt(e->pos+Vector(e->bba.e[0]+colEps,e->bbb.e[1]-colEps,e->bba.e[2]-colEps))&1)
        || (collisionLevelAt(e->pos+Vector(e->bbb.e[0]-colEps,e->bbb.e[1]-colEps,e->bba.e[2]-colEps))&1)  ) cflag|=16;

    if (   (collisionLevelAt(e->pos+Vector(e->bba.e[0]+colEps,e->bba.e[1]+colEps,e->bbb.e[2]+colEps))&1)
        || (collisionLevelAt(e->pos+Vector(e->bbb.e[0]-colEps,e->bba.e[1]+colEps,e->bbb.e[2]+colEps))&1)
        || (collisionLevelAt(e->pos+Vector(e->bba.e[0]+colEps,e->bbb.e[1]-colEps,e->bbb.e[2]+colEps))&1)
        || (collisionLevelAt(e->pos+Vector(e->bbb.e[0]-colEps,e->bbb.e[1]-colEps,e->bbb.e[2]+colEps))&1)  ) cflag|=32;


    // detect wether climbing an edge is possible


    if ( vposr0 == vposr2 )
    if (  ( (collisionLevelAt(e->pos+Vector(e->bba.e[0]-colEps,e->bba.e[1]-colEps,e->bba.e[2]+colEps))&4)
        ||  (collisionLevelAt(e->pos+Vector(e->bba.e[0]-colEps,e->bba.e[1]-colEps,e->bbb.e[2]-colEps))&4))
        &&  (collisionLevelAt(e->pos+Vector(e->bba.e[0]-colEps,e->bbb.e[1]+colEps,e->bba.e[2]+colEps))&2)
        &&  (collisionLevelAt(e->pos+Vector(e->bba.e[0]-colEps,e->bbb.e[1]+colEps,e->bbb.e[2]-colEps))&2)  ) cflag|=64;

    if ( vposr0 == vposr2 )
    if (  ( (collisionLevelAt(e->pos+Vector(e->bbb.e[0]+colEps,e->bba.e[1]-colEps,e->bba.e[2]+colEps))&4)
        ||  (collisionLevelAt(e->pos+Vector(e->bbb.e[0]+colEps,e->bba.e[1]-colEps,e->bbb.e[2]-colEps))&4))
        &&  (collisionLevelAt(e->pos+Vector(e->bbb.e[0]+colEps,e->bbb.e[1]+colEps,e->bba.e[2]+colEps))&2)
        &&  (collisionLevelAt(e->pos+Vector(e->bbb.e[0]+colEps,e->bbb.e[1]+colEps,e->bbb.e[2]-colEps))&2)  ) cflag|=128;

    if ( vposr0 == vposr2 )
    if (  ( (collisionLevelAt(e->pos+Vector(e->bba.e[0]+colEps,e->bba.e[1]-colEps,e->bba.e[2]-colEps))&4)
        ||  (collisionLevelAt(e->pos+Vector(e->bbb.e[0]-colEps,e->bba.e[1]-colEps,e->bba.e[2]-colEps))&4))
        &&  (collisionLevelAt(e->pos+Vector(e->bba.e[0]+colEps,e->bbb.e[1]+colEps,e->bba.e[2]-colEps))&2)
        &&  (collisionLevelAt(e->pos+Vector(e->bbb.e[0]-colEps,e->bbb.e[1]+colEps,e->bba.e[2]-colEps))&2)  ) cflag|=256;

    if ( vposr0 == vposr2 )
    if (  ( (collisionLevelAt(e->pos+Vector(e->bba.e[0]+colEps,e->bba.e[1]-colEps,e->bbb.e[2]+colEps))&4)
        ||  (collisionLevelAt(e->pos+Vector(e->bbb.e[0]-colEps,e->bba.e[1]-colEps,e->bbb.e[2]+colEps))&4))
        &&  (collisionLevelAt(e->pos+Vector(e->bba.e[0]+colEps,e->bbb.e[1]+colEps,e->bbb.e[2]+colEps))&2)
        &&  (collisionLevelAt(e->pos+Vector(e->bbb.e[0]-colEps,e->bbb.e[1]+colEps,e->bbb.e[2]+colEps))&2)  ) cflag|=512;



   return cflag;
}

void initgame(TilesetMap &tileset)
{
    // build the levels collision data

    memset(collisionLevel,0,LEVELWIDTH*LEVELHEIGHT*LEVELDEPTH);

    LevelTile::iterator t;

    for(int i=0;i<LEVELWIDTH*LEVELHEIGHT*LEVELDEPTH;i++)
    {
        for (t=level[i].begin();t!=level[i].end();++t)
        {
            std::string a = t->name;
            if ( tileset[a]=="block") collisionLevel[i] |= 1;
            else if ( tileset[a]=="platform")
            {
                collisionLevel[i] |= 2;                             // platform at bottom of tile
                if(i>=LEVELWIDTH) collisionLevel[i-LEVELWIDTH] |=4; // platform at top of tile
            }
            else if ( tileset[a]=="complete")
            {
                collisionLevel[i] |= 5;
                if(i<LEVELWIDTH*(LEVELHEIGHT*LEVELDEPTH-1)) collisionLevel[i+LEVELWIDTH] |= 2;
            }
        }
    }

    for(int x=0;x<LEVELWIDTH;x++)
    for(int z=0;z<LEVELDEPTH;z++)
    {
        collisionLevel[x+z*LEVELWIDTH*LEVELHEIGHT]=5; // dont fall through the floor into sig sev realm!
    }


    // reset the druid

    testdruid.pos = Vector(3.0f,5.0f,5.0f);
    testdruid.vel = Vector(0.0f,0.0f,0.0f);
    testdruid.acc = Vector(0.0f,0.0f,0.0f);

    testdruid.movementState = FLOOR;

    gamestate = GAME;
}


int renderloop()
{
    static int quit = false;

    int glmaj, glmin;



    if (!create_context()) return false;        // create the opengl rendering window

    if(0!=gl3wInit()) return false;             // initialize the gl3w library

    glGetIntegerv(GL_MAJOR_VERSION, &glmaj);    // get opengl version number
	glGetIntegerv(GL_MINOR_VERSION, &glmin);    // 4.1 would be 4 in glmaj and 1 in glmin


    gamepad_create();                           // gamepad support!! :)

    Text::init();                               // initialize the simple text renderer



    // load the image resources defined in imageresources.xml

    ImageMap imagemap;

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

        imagemap[(np->second)["name"]]=img;
    }



    // substitute kathy model to test animation.

    File kathystatefile = File::read("states/anistate.xml");

    State* kathystate = new State();

    kathystate->read(kathystatefile);
    kathystate->link();


    File kathyfile = File::read("models/anikathy.dae");

    Collada *kathy = new Collada();

    kathy->state = kathystate;
    kathy->read(kathyfile);





    // initialise an opengl render state defined in simplestate.xml

	State *teststate = new State();

	File statefile = File::read("states/simplestate.xml");

	teststate->read( statefile );
	teststate->link();

    // load the models from the tileset.dae collada file

	File colladafile = File::read("models/tileset.dae");

	Collada *collada = new Collada();

	collada->state = teststate;
	collada->read( colladafile );

	// load and parse the tileset definition file

	File tilesetfile = File::read("data/tileset.xml");

	XML tilesetxml = XML::readfile(tilesetfile);
	NodeMap tilesettiles = tilesetxml.get("tile");

	TilesetMap tileset;

	for(np=tilesettiles.begin();np!=tilesettiles.end();++np)
	{
        std::string a = (np->second)["name"];
	    std::string b = (np->second)["type"];

   	    tileset[a] = b;

	}

    // initialize the cursor

    editorcursor.tileused=collada->geometries.begin();
    editorcursor.tiles=&collada->geometries;

    // initialize modelview and projection matrices

	Matrix projectionMatrix = Matrix::projection(-1.0f,1.0f,-0.75f,0.75f,1.0f,50.0f);
	Matrix modelviewMatrix = Matrix::identity();
	Matrix cameraMatrix = Matrix::identity();

    // load level from level.txt

    loadlevel();

    // create the level collision data and reset the player character

    initgame(tileset);









    while (!quit)
    if (!system_hook(&quit))    // quit will become true if the os wants the application to be closed
    {
        int i;

        // animate the editor cursor

        for (i=0;i<3;i++) editorcursor.ipPosition.e[i] = editorcursor.ipPosition.e[i]*0.8f + (float)editorcursor.position[i]*(2.0f*0.2f);
        editorcursor.ipRotation = editorcursor.ipRotation*0.8f + (float)editorcursor.rotation*(PI*2.0f*0.25f*0.2f);

        // recalculate camera matrix

        if (gamestate==EDITOR)
        {

            cameraMatrix = Matrix::translation(Vector(0,0,editorcursor.scrollPosition.e[2]))
                         * Matrix::rotation(0,editorcursor.looky)
                         * Matrix::rotation(1,editorcursor.lookx)
                         * Matrix::translation(Vector(editorcursor.scrollPosition.e[0],editorcursor.scrollPosition.e[1],0));

        }
        else
        {
            cameraMatrix = Matrix::translation(Vector(0.0f,0.0f,-5.0f))
                         * Matrix::rotation(0,0.2f)
                         * Matrix::translation(Vector(1.0f,-2.0f,0.0f)-testdruid.pos);

        }



        // a standard opengl setup

        glHint(GL_POLYGON_SMOOTH_HINT,GL_NICEST);           // nicest shading
        glEnable(GL_POLYGON_SMOOTH);                        // turn of flat-shading

        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);   // standard blending

        glDepthMask(1);                                     // depth write on

        glClearColor(0.5f,0.5f,0.5f,0.0f);                  // r,g,b,a for screen clear
        glClearDepth(1.0f);                                 // z for screen clear

        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);   // clear screen

        glViewport(0,0,windowwidth,windowheight);


        // set the state for rendering the level

        teststate->set();


        // load the projection matrix

        teststate->uniformMatrix("projection",projectionMatrix);



        if (gamestate==EDITOR)
        {
            // editor mode

            // display the cursor

            modelviewMatrix = cameraMatrix * Matrix::translation(editorcursor.ipPosition)
                            * Matrix::rotation(1, editorcursor.ipRotation)
                            * Matrix::rotation(0, PI*1.5f);

            teststate->uniformMatrix("modelview",modelviewMatrix);

            imagemap["Material_001"].use(0);
            collada->geometries["selectbox"].meshes[0].array.draw();



            // display the tile that can be painted

            (editorcursor.tileused->second).draw(imagemap);

        }
        else
        {
            // game mode

            // query the gamepad and mix data with keyboard input

            gamepad gg;
            gamepad_query(gg);

            int ggkeystatearray[16];

            memcpy(ggkeystatearray,keystatearray,16*4);

            ggkeystatearray[0]|=gg.x<-0.1f;
            ggkeystatearray[1]|=gg.x>0.1f;
            ggkeystatearray[2]|=gg.y<-0.1f;
            ggkeystatearray[3]|=gg.y>0.1f;
            ggkeystatearray[6]|=gg.b[0];
            ggkeystatearray[11]|=gg.b[1];

            // move the druid around

            testdruid.step();                                        // physics
            testdruid.control(ggkeystatearray,lastkeystatearray);    // player control

            memcpy(lastkeystatearray,ggkeystatearray,16*4);



            modelviewMatrix = cameraMatrix * Matrix::translation(testdruid.pos+Vector(-1.0f,-0.9f-0.2f+testdruid.aniControl.positionoffset,-1.0f))
                             /* * Matrix::scale((testdruid.bbb-testdruid.bba)*0.5f) */
                            * Matrix::rotation(1, testdruid.aniControl.rotation)
                            * Matrix::rotation(0, PI*1.5f);

            // display the druid's bounding box

            //teststate->uniformMatrix("modelview",modelviewMatrix);

            //imagemap["Material_001"].use(0);
            //collada->geometries["selectbox"].meshes[0].array.draw();


            // display the test animation

            Matrix testMatrices[20];

            kathy->armature->animate(testdruid.aniControl.lastframe,testdruid.aniControl.frame,testdruid.aniControl.ip,testMatrices);

            kathystate->set();

            kathystate->uniformMatrix("projection",projectionMatrix);
            kathystate->uniformMatrix("modelview",modelviewMatrix* Matrix::scale(Vector(0.07f,0.07f,0.07f)) ); // the model is much to large
            kathystate->uniformMatrix("bonematrix",testMatrices,20);

            kathy->geometries["Cube.004"].meshes[0].array.draw();

            teststate->set();


        }

        // render the level

        for(int z=0;z<LEVELDEPTH;z++)
        for(int y=0;y<LEVELHEIGHT;y++)
        for(int x=0;x<LEVELWIDTH;x++)
        {
            LevelTile::iterator li;

            for(li=level[x+y*LEVELWIDTH+z*LEVELWIDTH*LEVELHEIGHT].begin();
                li!=level[x+y*LEVELWIDTH+z*LEVELWIDTH*LEVELHEIGHT].end();++li)
            {
                modelviewMatrix = cameraMatrix * Matrix::translation(Vector((float)x*2.0f,(float)y*2.0f,(float)z*2.0f))
                                * Matrix::rotation(1, (float)li->rotation*(PI*0.5f))
                                * Matrix::rotation(0, PI*1.5f);

                teststate->uniformMatrix("modelview",modelviewMatrix);

                Geometry &g = collada->geometries[li->name];

                g.draw(imagemap);

            }
        }


        Text::update();

        frame_hook();
    }


    delete kathy;
    delete kathystate;
    delete collada;
    delete teststate;



    Text::deinit();

    kill_context();

    return true;
}


int mld=0,mrd=0;

// 32 bit keycodes get passed to keydown()

// the first byte represents the key
// the second byte is 'd' for key press or 'u' for key release
// the third and fourth bytes are zero

// special keycodes are
// "mwb" mousewheel back
// "mwf" mousewheel front
// "mld" left mouse button down
// "mlu" left mouse button up
// "mrd" right mouse button down
// "mru" right mouse button up
// where the fourth byte is zero

int keydown(int keycode)
{
    if (gamestate==EDITOR)
    {

        if (keycode=='A'+('d'<<8)) if (editorcursor.position[0]>0)   editorcursor.position[0]--;
        if (keycode=='D'+('d'<<8)) if (editorcursor.position[0]<LEVELWIDTH-1) editorcursor.position[0]++;

        if (keycode=='S'+('d'<<8)) if (editorcursor.position[1]>0)   editorcursor.position[1]--;
        if (keycode=='W'+('d'<<8)) if (editorcursor.position[1]<LEVELHEIGHT-1) editorcursor.position[1]++;

        if (keycode=='R'+('d'<<8)) if (editorcursor.position[2]>0)   editorcursor.position[2]--;
        if (keycode=='F'+('d'<<8)) if (editorcursor.position[2]<LEVELDEPTH-1)  editorcursor.position[2]++;

        if (keycode==VK_LEFT+('d'<<8)) editorcursor.rotation--;
        if (keycode==VK_RIGHT+('d'<<8)) editorcursor.rotation++;

        if (keycode==VK_UP+('d'<<8)) if (editorcursor.tileused!=editorcursor.tiles->begin())
        {
            --editorcursor.tileused;
            memset(editortext+64*13,0,64);
            strcpy(editortext+64*13,(editorcursor.tileused->first).c_str());
        }
        if (keycode==VK_DOWN+('d'<<8))
        {
            ++editorcursor.tileused;
            if (editorcursor.tileused==editorcursor.tiles->end()) --editorcursor.tileused;
            else
            {
                memset(editortext+64*13,0,64);
                strcpy(editortext+64*13,(editorcursor.tileused->first).c_str());
            }
        }

        if (keycode==' '+('d'<<8))
        {
            LevelTileDef ltd;
            ltd.name = editorcursor.tileused->first;
            ltd.rotation = editorcursor.rotation&3;

            level[editorcursor.position[0]+editorcursor.position[1]*LEVELWIDTH+editorcursor.position[2]*LEVELWIDTH*LEVELHEIGHT].push_front(ltd);
        }

        if (keycode==VK_DELETE+('d'<<8))
        {
            level[editorcursor.position[0]+editorcursor.position[1]*LEVELWIDTH+editorcursor.position[2]*LEVELWIDTH*LEVELHEIGHT].clear();
        }

        if (keycode==VK_F2+('d'<<8)) savelevel();
        if (keycode==VK_F3+('d'<<8)) loadlevel();

        //if (keycode==VK_RETURN+('d'<<8)) initgame();

    }

    else
    {


        int keycodes[16] = { 'A', 'D', 'W', 'S',
                             0, 0,
                             ' ', VK_CONTROL, 'X', VK_SHIFT, VK_LMENU /* alt key */, 'C',
                             0, 0, 0, 0 };

        for (int i=0;i<16;i++)
        {
            if (keycode==keycodes[i]+('d'<<8)) keystatearray[i]=true;
            if (keycode==keycodes[i]+('u'<<8)) keystatearray[i]=false;
        }


        if (keycode==VK_RETURN+('d'<<8))
        {
            gamestate = EDITOR;
        }
    }


    if (keycode=='m'+('l'<<8)+('d'<<16)) mld=true;
    if (keycode=='m'+('l'<<8)+('u'<<16)) mld=false;
    if (keycode=='m'+('r'<<8)+('d'<<16)) mrd=true;
    if (keycode=='m'+('r'<<8)+('u'<<16)) mrd=false;


    return 1;

}

// scroll with left button
// look around with right button

int mousemove(int x, int y)
{
    static int lx=0,ly=0;

    if (mld) editorcursor.scrollPosition+=Vector((float)(x-lx)*0.01f,(float)(ly-y)*0.01f,0.0f);
    if (mrd) { editorcursor.lookx+=(float)(x-lx)*0.01f; editorcursor.looky+=(float)(y-ly)*0.01f; }

    lx=x;ly=y;

    return 1;
}

// call-back routine for the text renderer

const char *console()
{
    if (gamestate==EDITOR) return editortext;
                    else   return gametext;
}


// save the level

void savelevel()
{
    LevelTile::iterator p;

    int i;

    std::ofstream f;
    f.open("level.txt");

    for (i=0;i<LEVELWIDTH*LEVELHEIGHT*LEVELDEPTH;i++)
    {
        f << level[i].size() << " ";

        for (p=level[i].begin();p!=level[i].end();++p)
        {
            f << p->name << " " << p->rotation << " ";
        }
    }

    f.close();
}

// reload the level

void loadlevel()
{
    LevelTile::iterator p;

    int i,j;
    int n;

    std::ifstream f;
    f.open("level.txt");

    for (i=0;i<LEVELWIDTH*LEVELHEIGHT*LEVELDEPTH;i++)
    {
        f >> n;

        level[i].clear();

        for (j=0;j<n;j++)
        {
            LevelTileDef t;
            f >> t.name >> t.rotation;
            level[i].push_front(t);
        }
    }

    f.close();

}




bool invMatrix(const float m[16], float invOut[16])
{
    // This was lifted from MESA implementation of the GLU library.

    float inv[16], det;
    int i;

    inv[0] = m[5]  * m[10] * m[15] -
             m[5]  * m[11] * m[14] -
             m[9]  * m[6]  * m[15] +
             m[9]  * m[7]  * m[14] +
             m[13] * m[6]  * m[11] -
             m[13] * m[7]  * m[10];

    inv[4] = -m[4]  * m[10] * m[15] +
              m[4]  * m[11] * m[14] +
              m[8]  * m[6]  * m[15] -
              m[8]  * m[7]  * m[14] -
              m[12] * m[6]  * m[11] +
              m[12] * m[7]  * m[10];

    inv[8] = m[4]  * m[9] * m[15] -
             m[4]  * m[11] * m[13] -
             m[8]  * m[5] * m[15] +
             m[8]  * m[7] * m[13] +
             m[12] * m[5] * m[11] -
             m[12] * m[7] * m[9];

    inv[12] = -m[4]  * m[9] * m[14] +
               m[4]  * m[10] * m[13] +
               m[8]  * m[5] * m[14] -
               m[8]  * m[6] * m[13] -
               m[12] * m[5] * m[10] +
               m[12] * m[6] * m[9];

    inv[1] = -m[1]  * m[10] * m[15] +
              m[1]  * m[11] * m[14] +
              m[9]  * m[2] * m[15] -
              m[9]  * m[3] * m[14] -
              m[13] * m[2] * m[11] +
              m[13] * m[3] * m[10];

    inv[5] = m[0]  * m[10] * m[15] -
             m[0]  * m[11] * m[14] -
             m[8]  * m[2] * m[15] +
             m[8]  * m[3] * m[14] +
             m[12] * m[2] * m[11] -
             m[12] * m[3] * m[10];

    inv[9] = -m[0]  * m[9] * m[15] +
              m[0]  * m[11] * m[13] +
              m[8]  * m[1] * m[15] -
              m[8]  * m[3] * m[13] -
              m[12] * m[1] * m[11] +
              m[12] * m[3] * m[9];

    inv[13] = m[0]  * m[9] * m[14] -
              m[0]  * m[10] * m[13] -
              m[8]  * m[1] * m[14] +
              m[8]  * m[2] * m[13] +
              m[12] * m[1] * m[10] -
              m[12] * m[2] * m[9];

    inv[2] = m[1]  * m[6] * m[15] -
             m[1]  * m[7] * m[14] -
             m[5]  * m[2] * m[15] +
             m[5]  * m[3] * m[14] +
             m[13] * m[2] * m[7] -
             m[13] * m[3] * m[6];

    inv[6] = -m[0]  * m[6] * m[15] +
              m[0]  * m[7] * m[14] +
              m[4]  * m[2] * m[15] -
              m[4]  * m[3] * m[14] -
              m[12] * m[2] * m[7] +
              m[12] * m[3] * m[6];

    inv[10] = m[0]  * m[5] * m[15] -
              m[0]  * m[7] * m[13] -
              m[4]  * m[1] * m[15] +
              m[4]  * m[3] * m[13] +
              m[12] * m[1] * m[7] -
              m[12] * m[3] * m[5];

    inv[14] = -m[0]  * m[5] * m[14] +
               m[0]  * m[6] * m[13] +
               m[4]  * m[1] * m[14] -
               m[4]  * m[2] * m[13] -
               m[12] * m[1] * m[6] +
               m[12] * m[2] * m[5];

    inv[3] = -m[1] * m[6] * m[11] +
              m[1] * m[7] * m[10] +
              m[5] * m[2] * m[11] -
              m[5] * m[3] * m[10] -
              m[9] * m[2] * m[7] +
              m[9] * m[3] * m[6];

    inv[7] = m[0] * m[6] * m[11] -
             m[0] * m[7] * m[10] -
             m[4] * m[2] * m[11] +
             m[4] * m[3] * m[10] +
             m[8] * m[2] * m[7] -
             m[8] * m[3] * m[6];

    inv[11] = -m[0] * m[5] * m[11] +
               m[0] * m[7] * m[9] +
               m[4] * m[1] * m[11] -
               m[4] * m[3] * m[9] -
               m[8] * m[1] * m[7] +
               m[8] * m[3] * m[5];

    inv[15] = m[0] * m[5] * m[10] -
              m[0] * m[6] * m[9] -
              m[4] * m[1] * m[10] +
              m[4] * m[2] * m[9] +
              m[8] * m[1] * m[6] -
              m[8] * m[2] * m[5];

    det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];

    if (det == 0)
        return false;

    det = 1.0 / det;

    for (i = 0; i < 16; i++)
        invOut[i] = inv[i] * det;

    return true;
}
