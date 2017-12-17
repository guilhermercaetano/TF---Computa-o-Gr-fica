// Guilherme Rodrigues Caetano - 2013100847
// Marcelo Bringuenti Pedro
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

union v3f
{
    struct
    {
        float x;
        float y;
        float z;
    };
    float fv[3];
};

union v4f
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

union v3
{
    v3f f;
    unsigned int uv[3];
    int iv[3];
    float fv[3];
};

union v4
{
    v4f f;
    unsigned int uv[4];
    int iv[4];
    float fv[4];
};

typedef struct m3x3
{
    union
    {
        struct{float a11, a12, a13;};
        v3f a1;
    };
    
    union
    {
        struct{float a21, a22, a23;};
        v3f a2;
    };
    
    union
    {
        struct{float a31, a32, a33;};
        v3f a3;
    };
    
} m3x3, m3;

typedef struct m4x4
{
    union {
        struct{
            union
            {
                struct{float a11, a12, a13, a14;};
                v4f a1;
            };
            
            union
            {
                struct{float a21, a22, a23, a24;};
                v4f a2;
            };
            
            union
            {
                struct{float a31, a32, a33, a34;};
                v4f a3;
            };
            
            union
            {
                struct{float a41, a42, a43, a44;};
                v4f a4;
            };
        };
        float fv[16];
    };
} m4x4, m4;

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
    
} bases;

v2f operator*(v2f V, float F);
v2f operator*(float F, v2f V);

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

m3 IdentityMat3();
m4 IdentityMat4();

float DegreesToRads(float);
float RadsToDegrees(float);
bases * RotateOrthonormalBases(bases *, float);
v3f Normalize(v3f);

float VectorSize(v3f);
float DotProduct(v3f, v3f);
float Square(float);

v3f V3f(float, float, float);
v4f V4f(float, float, float, float);


#endif
