
#ifndef ARMATURE_H
#define ARMATURE_H

#include "log.h"

struct Bone;

class Armature;

struct Bone
{
    Bone *parent;
    Matrix baseTransform;
    Matrix *aniTransform;
    Matrix inverseBindPose;
    Matrix world;

};

class Armature
{
public:

    Armature() { bones = NULL; nAnimations=0; nBones=0; }


    void animate(int index, Matrix *mtable)
    {
        bones[0].world = bones[0].aniTransform[index];

        for (int i=1;i<nBones;i++)
        {
            bones[i].world = bones[i].parent->world * bones[i].aniTransform[index];
            mtable[i-1] =  bones[i].world * bones[i].inverseBindPose * bindShape ;
        }

    }

    void animate(int index0, int index1, float ip, Matrix *mtable)
    {
        bones[0].world = Matrix::interpolate ( bones[0].aniTransform[index0], bones[0].aniTransform[index1], ip );

        for (int i=1;i<nBones;i++)
        {
            bones[i].world = bones[i].parent->world * Matrix::interpolate ( bones[i].aniTransform[index0], bones[i].aniTransform[index1], ip );
            mtable[i-1] =  bones[i].world * bones[i].inverseBindPose * bindShape ;
        }

    }


    int nAnimations;
    int nBones;

    Bone *bones;

    Matrix bindShape;

};

#endif
