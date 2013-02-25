
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

#include "environment.h"
#include "entity.h"


int windowwidth=1024,windowheight=768;

void windowsize(int w, int h)
{
    windowwidth=w;
    windowheight=h;
}

#define LEVELWIDTH 32
#define LEVELHEIGHT 32
#define LEVELDEPTH 8


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

void initenvironment(Environment *env)
{
    Matrix transform;
    LevelTile::iterator li;

    for(int z=0;z<LEVELDEPTH;z++)
    for(int y=0;y<LEVELHEIGHT;y++)
    for(int x=0;x<LEVELWIDTH;x++)
    {

        for(li=level[x+y*LEVELWIDTH+z*LEVELWIDTH*LEVELHEIGHT].begin();
            li!=level[x+y*LEVELWIDTH+z*LEVELWIDTH*LEVELHEIGHT].end();++li)
        {
            EnvironmentModelInstance model;
            model.transform = Matrix::translation(Vector((float)x*2.0f,(float)y*2.0f,(float)z*2.0f))
                            * Matrix::rotation(1, (float)li->rotation*(PI*0.5f))
                            * Matrix::rotation(0, PI*1.5f); // coordinate system transform hack (z<->y);

            model.rotation = Matrix::rotation(1, (float)li->rotation*(PI*0.5f))
                           * Matrix::rotation(0, PI*1.5f);

            model.geo = &(env->getGeometries()[li->name]);

            env->addModel(model);
        }
    }
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


    // initialize the environment (level) graphics

    Environment *env = new Environment();


    // substitute kathy model to test animation.

    File kathystatefile = File::read("states/anistate.xml");

    State* kathystate = new State();

    kathystate->read(kathystatefile);
    kathystate->link();


    File kathyfile = File::read("models/anikathy.dae");

    Collada *kathy = new Collada();

    kathy->state = kathystate;
    kathy->read(kathyfile);


	// load and parse the tileset definition file

	File tilesetfile = File::read("data/tileset.xml");

	XML tilesetxml = XML::readfile(tilesetfile);
	NodeMap tilesettiles = tilesetxml.get("tile");
	NodeMap::iterator np;

	TilesetMap tileset;

	for(np=tilesettiles.begin();np!=tilesettiles.end();++np)
	{
        std::string a = (np->second)["name"];
	    std::string b = (np->second)["type"];

   	    tileset[a] = b;

	}


    // initialize modelview and projection matrices

	Matrix projectionMatrix = Matrix::projection(-1.0f,1.0f,-0.75f,0.75f,1.0f,50.0f);
	Matrix modelviewMatrix = Matrix::identity();
	Matrix cameraMatrix = Matrix::identity();

    // load level from level.txt

    loadlevel();

    // create environment graphics

    initenvironment(env);

    env->bakeLightmaps(0);

    // create the level collision data and reset the player character

    initgame(tileset);

    while (!quit)
    if (!system_hook(&quit))    // quit will become true if the os wants the application to be closed
    {
        static Vector smoothvel=Vector(0,0,0);
        smoothvel=smoothvel*0.8f+testdruid.vel*0.2f;
        static Vector smoothpos=Vector(0,0,0);
        smoothpos=smoothpos*0.8f+(testdruid.pos+smoothvel*10.0f)*0.2f;


        // recalculate camera matrix

        cameraMatrix = Matrix::translation(Vector(0.0f,0.0f,-3.0f-sqrtf(smoothvel.length2())*10.0f))
                     * Matrix::rotation(0,0.2f)
                     * Matrix::translation(Vector(1.0f,-1.0f,0.0f)-smoothpos);


        // a standard opengl setup


        //glCullFace(GL_BACK);
        //glEnable(GL_CULL_FACE);

        glHint(GL_POLYGON_SMOOTH_HINT,GL_NICEST);           // nicest shading
        glEnable(GL_POLYGON_SMOOTH);                        // turn of flat-shading

        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);   // standard blending

        glDepthMask(1);                                     // depth write on

        glClearColor(0.0f,0.0f,0.0f,1.0f);                // r,g,b,a for screen clear
        glClearDepth(1.0f);                                 // z for screen clear

        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);   // clear screen

        glViewport(0,0,windowwidth,windowheight);


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


        // display the test animation

        Matrix testMatrices[20];

        kathy->armature->animate(testdruid.aniControl.lastframe,testdruid.aniControl.frame,testdruid.aniControl.ip,testMatrices);

        kathystate->set();

        kathystate->uniformMatrix("projection",projectionMatrix);
        kathystate->uniformMatrix("modelview",modelviewMatrix* Matrix::scale(Vector(0.07f,0.07f,0.07f)) ); // the model is much to large
        kathystate->uniformMatrix("bonematrix",testMatrices,20);

        kathy->geometries["Cube.004"].meshes[0].array.draw();


        // display the level

        env->render(projectionMatrix,cameraMatrix,1);


        Text::update();

        frame_hook();
    }


    delete kathy;
    delete kathystate;

    delete env;



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

    int keycodes[16] = { 'A', 'D', 'W', 'S',
                         0, 0,
                         ' ', VK_CONTROL, 'X', VK_SHIFT, VK_LMENU /* alt key */, 'C',
                         0, 0, 0, 0 };

    for (int i=0;i<16;i++)
    {
        if (keycode==keycodes[i]+('d'<<8)) keystatearray[i]=true;
        if (keycode==keycodes[i]+('u'<<8)) keystatearray[i]=false;
    }


    if (keycode=='m'+('l'<<8)+('d'<<16)) mld=true;
    if (keycode=='m'+('l'<<8)+('u'<<16)) mld=false;
    if (keycode=='m'+('r'<<8)+('d'<<16)) mrd=true;
    if (keycode=='m'+('r'<<8)+('u'<<16)) mrd=false;


    return 1;

}

int mousemove(int x, int y)
{

    return 1;
}

// call-back routine for the text renderer

const char *console()
{
    static char nulltext[64*32];
    return nulltext;
}


// load the level

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

