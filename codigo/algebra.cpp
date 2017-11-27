// Guilherme Rodrigues Caetano - 2013100847
// TF Computação Gráfica

#include<math.h>
#include"algebra.h"

v3f operator+=(v3f &A, v3f B)
{
    A.x += B.x;
    A.y += B.y;
    A.z += B.z;
    
    return A;
}

v3f operator-=(v3f &A, v3f B)
{
    A.x -= B.x;
    A.y -= B.y;
    A.z -= B.z;
    
    return A;
}

v3f operator+(v3f A, v3f B)
{
    v3f R = {};
    R.x = A.x + B.x;
    R.y = A.y + B.y;
    R.z = A.z + B.z;
    
    return R;
}

v3f operator-(v3f A, v3f B)
{
    v3f R = {};
    R.x = A.x - B.x;
    R.y = A.y - B.y;
    R.z = A.z - B.z;
    
    return R;
}

v3f operator-(v3f A)
{
    v3f R = {};
    R.x = -A.x;
    R.y = -A.y;
    R.z = -A.z;
    
    return R;
}

v3f operator+(v3f A, float B)
{
    return V3f(A.x+B, A.y+B, A.z+B);
}

v3f operator+(float B, v3f A)
{
    return A + B;
}

v3f operator*=(v3f &A, float B)
{
    A.x *= B;
    A.y *= B;
    A.z *= B;
    
    return A;
}

v3f operator/=(v3f &A, float B)
{
    return (A *= (1/B));
}

v3f operator*(v3f V, float F)
{
    v3f R;
    R.x = V.x * F;
    R.y = V.y * F;
    R.z = V.z * F;
    
    return R;
}

v3f operator*(float F, v3f V)
{
    return (V*F);
}

v3f operator/(float F, v3f V)
{
    v3f R;
    R.x = F / V.x;
    R.y = F / V.y;
    R.z = F / V.z;
    
    return R;
}

v3f operator/(v3f V, float F)
{
    v3f R;
    R.x = V.x / F;
    R.y = V.y / F;
    R.z = V.z / F;
    
    return R;
}

v3f operator*(m3x3 M, v3f V)
{
    v3f Result;
    Result.x = DotProduct(M.a1, V);
    Result.y = DotProduct(M.a2, V);
    Result.z = DotProduct(M.a3, V);
    
    return Result;
}

m3x3 Transpose(m3x3 M)
{
    return (m3x3{
            M.a11, M.a21, M.a31,
            M.a12, M.a22, M.a32,
            M.a13, M.a23, M.a33});
    
}

v3f CoordinateChange(m3x3 M, v3f V)
{
    M = Transpose(M);
    v3f Result = M * V;
    
    return Result;
}

float Square(float Value)
{
    return (Value * Value);
}

float VectorSize(v3f V)
{
    return sqrtf(Square(V.x) + Square(V.y));
}

float DotProduct(v3f V1, v3f V2)
{
    float Result = (V1.x * V2.x) + (V1.y * V2.y) + (V1.z * V2.z);
    
    return Result;
}

float DegreesToRads(float AngleInDegrees)
{
    float Result = AngleInDegrees * 2 * Pi / 360;
    
    return Result;
}

float RadsToDegrees(float AngleInRads)
{
    float Result = AngleInRads * 360 / (2 * Pi);
    
    return Result;
}

bases * RotateOrthonormalBases(bases *Bases, float AngleToRotate)
{
    Bases->xAxis = V3f(cosf(AngleToRotate), sinf(AngleToRotate), 0.0f);
    Bases->yAxis = V3f(-sinf(AngleToRotate), cosf(AngleToRotate), 0.0f);
    Bases->Angle = AngleToRotate;
    
    //TODO: Rotação incremental das bases Bases->Angle += AngleToRotate;
    
    return Bases;
}

v3f Normalize(v3f V)
{
    return (V /= VectorSize(V));
}

v3f V3f(float x, float y, float z)
{
    v3f R = {};
    R.x = x;
    R.y = y;
    R.z = z;
    
    return R;
}