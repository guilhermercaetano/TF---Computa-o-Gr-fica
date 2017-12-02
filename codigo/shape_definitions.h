#ifndef SHAPE_DEFINITIONS_H
#define SHAPE_DEFINITIONS_H

#define MaxCircleVertices 60
#define SHAPE_VERTICES 60

struct material
{
    v4f Emission;
    v4f ColorA;
    v4f ColorD;
    v4f Specular;
    v4f Shininess;
};

struct texture
{
    uint Id;
    material Material;
    float Scale;
};

typedef enum shape_type
{
    Shape_Ellipse,
    Shape_Circle,
    Shape_Rectangle,
    Shape_Point,
    Shape_Composed,
    Shape_Sphere,
    Shape_Box,
    Shape_Cylinder,
} shape_type;

struct vertex
{
    v3f Coordinate;
    v3f Normal;
    v2f UVCoordinate;
};

struct sphere
{
    int VertexCount;
    float Radius;
    vertex *Vertex;
};

struct box
{
    float Width;
    float Height;
    float Depth;
    vertex Vertex[13];
};

struct cylinder
{
    float Radius;
    float Height;
    
    vertex BaseVertices[MaxCircleVertices];
    vertex TopVertices[MaxCircleVertices];
    vertex Vertices[2 * MaxCircleVertices];
};

typedef struct circle
{
    float Radius;
    // TODO: Usar vértices para fazer o cálculo dos texels
    v2f TexelPoints[MaxCircleVertices];
    v3f Points[MaxCircleVertices];
} circle;

typedef struct ellipse
{
    float xAxis;
    float yAxis;
    v3f Points[SHAPE_VERTICES];
} ellipse;

typedef struct rectangle
{
    float Width;
    float Height;
    v3f Points[4];
} rectangle;

struct shape
{
    svg_color_names ColorFill;
    shape_type Type;
    v3f Origin;
    v3f OffsetFromOrigin;
    v3f RotationNormal;
    bases Bases;
    transform Transform;
    texture *Texture;
    
    union
    {
        circle Circle;
        ellipse Ellipse;
        rectangle Rectangle;
        sphere Sphere;
        box Box;
        cylinder Cylinder;
    };
    
};

#endif 