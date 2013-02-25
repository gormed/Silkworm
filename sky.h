
/*

    sky.h

    sky renderer

*/

#include "array.h"
#include "collada.h"
#include "image.h"

class Sky
{
private:
    State state;
    Image image;
    Collada collada;

public:
    Sky();
    ~Sky();

    void draw(Matrix &projection, Matrix &modelview);

};


