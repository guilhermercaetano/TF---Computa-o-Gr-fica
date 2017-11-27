// Guilherme Rodrigues Caetano - 2013100847
// TF Computação Gráfica

#ifndef _ALGEBRA_
#define _ALGEBRA_

#define Pi 3.14159265359

struct v2f
{
    union
    {
        float x;
        float u;
    };
    
    union
    {
        float y;
        float v;
    };
};

typedef struct v3f
{
    float x;
    float y;
    float z;
} v3f;

struct v4f
{
    union
    {
        struct
        {
            union{ float x; float r;};
            union{ float y; float g;};
            union{ float z; float b;};
            union{ float w; float alpha;};
        };
        float fv[4];
    };
    
};

struct v4
{
    union
    {
        v4f V4f;
        float Vfl[4];
    };
};

typedef struct m3x3
{
    union
    {
        struct
        {
            float a11, a12, a13;
        };
        v3f a1;
    };
    
    union
    {
        struct
        {
            float a21, a22, a23;
        };
        
        v3f a2;
    };
    
    union
    {
        struct
        {
            float a31, a32, a33;
        };
        
        v3f a3;
    };
    
} m3x3;

typedef struct bases
{
    union
    {
        struct
        {
            v3f xAxis;
            v3f yAxis;
            v3f zAxis;
        };
        m3x3 BaseMatrix;
    };
    
    float Angle;
    
    v3f Scale;
    v3f Rotation;
} bases;

v3f operator+=(v3f &, v3f);
v3f operator-=(v3f &, v3f);
v3f operator-(v3f, v3f);
v3f operator+(v3f, v3f);

v3f operator+(v3f, float);
v3f operator+(float, v3f);
v3f operator*=(v3f &, float);
v3f operator*(v3f, float);
v3f operator*(float, v3f);
v3f operator/(float F, v3f V);
v3f operator/(v3f V, float F);

v3f operator*(m3x3, v3f);
m3x3 Transpose(m3x3);
v3f CoordinateChange(m3x3, v3f);

float DegreesToRads(float);
float RadsToDegrees(float);
bases * RotateOrthonormalBases(bases *, float);
v3f Normalize(v3f);

float VectorSize(v3f);
float DotProduct(v3f, v3f);
float Square(float);
v3f V3f(float, float, float);


#endif
