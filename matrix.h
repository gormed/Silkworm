

/*

    matrix.h

    code for matrix math

*/


#ifndef MATRIX_H
#define MATRIX_H

#include <math.h>

#include "vector.h"

class Matrix
{
    public:

    float e[16];

    Matrix() { for(int i=0;i<16;i++) e[i]=0.0f; }

    static Matrix zero()
    {
        Matrix m;
        return m;
    }

    static Matrix identity()
    {
        Matrix m;
        m.e[0]=1.0f;
        m.e[5]=1.0f;
        m.e[10]=1.0f;
        m.e[15]=1.0f;

        return m;
    }

    static Matrix flip(Matrix m)
    {
        Matrix r;

        r.e[0]=m.e[0];
        r.e[1]=m.e[4];
        r.e[2]=m.e[8];
        r.e[3]=m.e[12];

        r.e[4]=m.e[1];
        r.e[5]=m.e[5];
        r.e[6]=m.e[9];
        r.e[7]=m.e[13];

        r.e[8]=m.e[2];
        r.e[9]=m.e[6];
        r.e[10]=m.e[10];
        r.e[11]=m.e[14];

        r.e[12]=m.e[3];
        r.e[13]=m.e[7];
        r.e[14]=m.e[11];
        r.e[15]=m.e[15];

        return r;
    }

    static Matrix interpolate (Matrix m0, Matrix m1, float ip)
    {
        Matrix r;

        for(int i=0;i<16;i++) r.e[i] = m0.e[i]*(1.0f-ip) + m1.e[i]*ip;

        return r;
    }


    static Matrix ortho(float left, float right, float bottom, float top, float _far, float _near)
    {
        Matrix r;

        float width=right-left;
        float height=top-bottom;
        float depth=_far-_near;

        r.e[0]=2.0f/width;
        r.e[5]=2.0f/height;
        r.e[10]=-2.0f/depth;

        r.e[12]=-(right+left)/width;
        r.e[13]=-(top+bottom)/height;
        r.e[14]=-(_far+_near)/depth;

        r.e[15]=1.0f;

        return r;
    }

    static Matrix projection(float right, float top, float _near, float _far)
    {
        Matrix r;

        r.e[0]=_near/right;
        r.e[5]=_near/top;
        r.e[10]=-(_far+_near)/(_far-_near);
        r.e[11]=-1;
        r.e[14]=-(2*_far*_near)/(_far-_near);

        return r;
    }

    static Matrix projection(float left, float right, float bottom, float top, float _near, float _far)
    {
        Matrix r;

        r.e[0]=2.0f*_near/(right-left);
        r.e[5]=2.0f*_near/(top-bottom);
        r.e[8]=(right+left)/(right-left);
        r.e[9]=(top+bottom)/(top-bottom);
        r.e[10]=-(_far+_near)/(_far-_near);
        r.e[11]=-1;
        r.e[14]=-(2*_far*_near)/(_far-_near);

        return r;
    }

    static Matrix rotation(int axis, float radians)
    {
        Matrix r = Matrix::identity();

        r.e[ ((axis+1)%3)*4 + ((axis+1)%3) ] = cosf(radians);
        r.e[ ((axis+2)%3)*4 + ((axis+2)%3) ] = cosf(radians);
        r.e[ ((axis+2)%3)*4 + ((axis+1)%3) ] = -sinf(radians);
        r.e[ ((axis+1)%3)*4 + ((axis+2)%3) ] = sinf(radians);

        return r;
    }

    static Matrix translation(const Vector v)
    {
        Matrix r = Matrix::identity();

        r.e[12] = v.e[0];
        r.e[13] = v.e[1];
        r.e[14] = v.e[2];

        return r;
    }

    static Matrix scale(const Vector v)
    {
        Matrix r = Matrix::identity();

        r.e[0] = v.e[0];
        r.e[5] = v.e[1];
        r.e[10] = v.e[2];

        return r;
    }

    void operator = (const Matrix m)
    {
        for(int i=0;i<16;i++) e[i]=m.e[i];
    }

    bool operator == (const Matrix m) const
    {
        for(int i=0;i<16;i++)
        if(e[i]<m.e[i]-EPSILON || m.e[i]<e[i]-EPSILON)
            return false;

        return true;
    }

    Matrix operator * (const Matrix m) const
    {
        Matrix r;

        int i,j,k=0;
        for (j=0;j<16;j+=4)
        for (i=0;i<4;i++)
        {
            r.e[k]   = e[i+0 ]*m.e[j+0]
                     + e[i+4 ]*m.e[j+1]
                     + e[i+8 ]*m.e[j+2]
                     + e[i+12]*m.e[j+3];
            k++;
        }

        return r;
    }

    void operator *= (const Matrix m)
    {
        float t[16];

        int i,j,k=0;
        for (j=0;j<16;j+=4)
        for (i=0;i<4;i++)
        {
            t[k]     = e[i+0 ]*m.e[j+0]
                     + e[i+4 ]*m.e[j+1]
                     + e[i+8 ]*m.e[j+2]
                     + e[i+12]*m.e[j+3];
            k++;
        }

        for (i=0;i<16;i++) e[i]=t[i];
    }

    Vector operator * (const Vector v) const
    {
        Vector r;

        int i;
        for (i=0;i<4;i++)
        {
            r.e[i]   = e[i+0 ]*v.e[0]
                     + e[i+4 ]*v.e[1]
                     + e[i+8 ]*v.e[2]
                     + e[i+12]*v.e[3];
        }

        return r;
    }

    Vector mul3 (const Vector v) const
    {
        Vector r;

        int i;
        for (i=0;i<3;i++)
        {
            r.e[i]   = e[i+0 ]*v.e[0]
                     + e[i+4 ]*v.e[1]
                     + e[i+8 ]*v.e[2]
                     + e[i+12]*v.e[3];
        }

        return r;
    }

};


#endif

