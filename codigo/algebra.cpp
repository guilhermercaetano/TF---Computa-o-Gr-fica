// Guilherme Rodrigues Caetano - 2013100847
// Marcelo Bringuenti Pedro
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

m3 operator*(m3 M1, m3 M2)
{
    m3 Result;
    Result.a11 = M1.a11*M2.a11+M1.a11*M2.a21+M1.a11*M2.a31;
    Result.a12 = M1.a12*M2.a12+M1.a12*M2.a22+M1.a12*M2.a32;
    Result.a13 = M1.a13*M2.a13+M1.a13*M2.a23+M1.a13*M2.a33;
    
    Result.a21 = M1.a21*M2.a11+M1.a22*M2.a21+M1.a23*M2.a31;
    Result.a22 = M1.a21*M2.a12+M1.a22*M2.a22+M1.a23*M2.a32;
    Result.a23 = M1.a21*M2.a13+M1.a22*M2.a23+M1.a23*M2.a33;
    
    Result.a31 = M1.a31*M2.a11+M1.a32*M2.a21+M1.a33*M2.a31;
    Result.a32 = M1.a31*M2.a12+M1.a32*M2.a22+M1.a33*M2.a32;
    Result.a33 = M1.a31*M2.a13+M1.a32*M2.a23+M1.a33*M2.a33;
    
    return Result;
}

v2f operator*(v2f V, float F)
{
    v2f R;
    R.x = V.x * F;
    R.y = V.y * F;
    
    return R;
}

v2f operator*(float F, v2f V)
{
    return V * F;
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

v3f CrossProduct(v3f U, v3f V)
{
    v3f Result;
    Result.x = U.y*V.z-U.z*V.y;
    Result.y = U.z*V.x-U.x*V.z;
    Result.z = U.x*V.y-U.y*V.x;
    
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

// NOTA: Isso possivelmente vai sumir em breve.
bases * RotateOrthonormalBases(bases *Bases, float AngleToRotate)
{
    Bases->xAxis = V3f(cosf(AngleToRotate), sinf(AngleToRotate), 0.0f);
    Bases->yAxis = V3f(-sinf(AngleToRotate), cosf(AngleToRotate), 0.0f);
    
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

v4f V4f(float x, float y, float z, float w)
{
    v4f R = {};
    R.x = x;
    R.y = y;
    R.z = z;
    R.w = w;
    
    return R;
}

m4 IdentityMat4()
{
    m4 M4 = {};
    M4.a1 = V4f(1.0f, 0.0f, 0.0f, 0.0f);
    M4.a2 = V4f(0.0f, 1.0f, 0.0f, 0.0f);
    M4.a3 = V4f(0.0f, 0.0f, 1.0f, 0.0f);
    M4.a4 = V4f(0.0f, 0.0f, 0.0f, 1.0f);
    
    return M4;
}