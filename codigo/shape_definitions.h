#ifndef SHAPE_DEFINITIONS_H
#define SHAPE_DEFINITIONS_H

#define MaxCircleVertices 600
#define SHAPE_VERTICES 600
#define MaxRectangleVertices 2000

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
    Shape_Undefined,
    Shape_Ellipse,
    Shape_Circle,
    Shape_Rectangle,
    Shape_Point,
    Shape_Composed,
    Shape_Sphere,
    Shape_Box,
    Shape_Cylinder,
    Shape_SubdividedRectangle,
    Shape_MeshObject,
} shape_type;

struct vertex
{
    v3f Coordinate;
    v3f Normal;
    v2f UVCoordinate;
};

struct sphere
{
    float Radius;
    int VertexCount;
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
    
    bool InvSideNormals;
    bool InvTopNormals;
    bool InvBaseNormals;
    
    bool DrawBase;
    bool DrawTop;
    bool DrawSide;
    
    vertex BaseVertices[MaxCircleVertices];
    vertex TopVertices[MaxCircleVertices];
    vertex Vertices[2 * MaxCircleVertices];
};

typedef struct circle
{
    float Radius;
    bool InvNormals;
    // TODO: Usar vértices para fazer o cálculo dos texels
    vertex Vertices[MaxCircleVertices];
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
    float ChunkSizeX;
    float ChunkSizeY;
    vertex Vertices[MaxRectangleVertices];
    
    v3f Points[4];
} rectangle;

struct obj_mesh
{
    char *FilePath;
    char *TextureFilePath;
    
    vertex *Vertex;
};

struct shape_header
{
    svg_color_names ColorFill;
    shape_type Type;
    v3f Origin;
    v3f OffsetFromOrigin;
    v3f RotationNormal;
    bases Bases;
    m4 ModelViewMatrix;
    transform Transform;
    texture *Texture;
    bool BuiltinTexture;
};

union shape_content
{
    circle Circle;
    ellipse Ellipse;
    rectangle Rectangle;
    sphere Sphere;
    box Box;
    cylinder Cylinder;
    obj_mesh ObjMesh;
};

struct shape_tree
{
    shape_header Header;
    shape_content Content;
    
    shape_tree *ChildrenLeft;
    shape_tree *ChildrenRight;
};

struct shape
{
    union 
    {
        struct
        {
            svg_color_names ColorFill;
            shape_type Type;
            v3f Origin;
            v3f OffsetFromOrigin;
            v3f RotationNormal;
            bases Bases;
            transform Transform;
            texture *Texture;
            bool BuiltinTexture;
        };
        shape_header Header;
    };
    
    union
    {
        circle Circle;
        ellipse Ellipse;
        rectangle Rectangle;
        sphere Sphere;
        box Box;
        cylinder Cylinder;
        obj_mesh ObjMesh;
    };
    
};

bases OrthonormalBases;
//OrthonormalBases. = ;

#endif 