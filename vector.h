
/*

    vector.h

    code for vector math

*/

#ifndef VECTOR_H
#define VECTOR_H

#include <math.h>

// a very small value
// floating points value which differ less than EPSILON are considered
// identical in comparisons

#define EPSILON 0.000001f

// the number pi

#define PI 3.14159269f

class Vector
{
    public:

    float e[4];

    Vector() { e[0]=0.0f; e[1]=0.0f; e[2]=0.0f; e[3]=1.0f; }
    Vector(float _x, float _y, float _z) { e[0]=_x, e[1]=_y; e[2]=_z; e[3]=1.0f; }
    Vector(float _x, float _y, float _z, float _w) { e[0]=_x, e[1]=_y, e[2]=_z, e[3]=_w; }

    void operator = (const Vector v)
    {
        for (int i=0;i<4;i++) e[i]=v.e[i];
    }

    bool operator == (const Vector v) const
    {
        for(int i=0;i<4;i++)
        if(e[i]<v.e[i]-EPSILON || v.e[i]<e[i]-EPSILON)
            return false;

        return true;
    }

    Vector operator + (const Vector v) const
    {
        Vector r;
        for(int i=0;i<4;i++) r.e[i]=e[i]+v.e[i];
        return r;
    }

    Vector operator - (const Vector v) const
    {
        Vector r;
        for(int i=0;i<4;i++) r.e[i]=e[i]-v.e[i];
        return r;
    }

    void operator += (const Vector v)
    {
        for (int i=0;i<4;i++) e[i]+=v.e[i];
    }

    void operator -= (const Vector v)
    {
        for (int i=0;i<4;i++) e[i]-=v.e[i];
    }

    Vector operator * (const Vector v) const
    {
        Vector r;
        r.e[0]=e[1]*v.e[2]+v.e[1]*e[2];
        r.e[1]=e[2]*v.e[0]+v.e[2]*e[0];
        r.e[2]=e[0]*v.e[1]+v.e[0]*e[1];

        return r;
    }

    void operator *= (const Vector v)
    {
        float t[3];
        t[0]=e[1]*v.e[2]+v.e[1]*e[2];
        t[1]=e[2]*v.e[0]+v.e[2]*e[0];
        t[2]=e[0]*v.e[1]+v.e[0]*e[1];

        e[0]=t[0];
        e[1]=t[1];
        e[2]=t[2];
    }

    Vector operator * (const float s) const
    {
        Vector r;
        for(int i=0;i<4;i++) r.e[i]=e[i]*s;

        return r;
    }

    void operator *= (const float s)
    {
        for (int i=0;i<4;i++) e[i]*=s;
    }

    float dot(const Vector v) const
    {
        return e[0]*v.e[0]+e[1]*v.e[1]+e[2]*v.e[2];
    }

    float dot4(const Vector v) const
    {
        return e[0]*v.e[0]+e[1]*v.e[1]+e[2]*v.e[2]+e[3]*v.e[3];
    }

    float length2() const
    {
        return e[0]*e[0]+e[1]*e[1]+e[2]*e[2];
    }

    void normalize()
    {
        float l=sqrtf(length2());
        e[0]/=l;
        e[1]/=l;
        e[2]/=l;
    }



};

#endif
