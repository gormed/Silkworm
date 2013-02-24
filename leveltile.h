#ifndef LEVELTILE_H_INCLUDED
#define LEVELTILE_H_INCLUDED


// Struct that defines one level tile. the sub unit of a level block
class LevelTileDef
{

    public:

        LevelTileDef() {}

        std::string name;
        int rotation;


};

class LevelTile : public std::list<LevelTileDef>
{
    public:
        bool isPassable = false;
};

//typedef std::list<LevelTileDef> LevelTile;

LevelTile level[LEVELWIDTH*LEVELDEPTH*LEVELHEIGHT];

#endif // LEVELTILE_H_INCLUDED
