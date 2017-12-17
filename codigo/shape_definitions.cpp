#include "shape_definitions.h"

void 
CalcSubdividedRectVertices(rectangle *Rectangle, float Height, 
                           float SubdivisionIntervalX, float SubdivisionIntervalY)
{
    uint VertexId = 0;
    
    for (float CurrentWidth = 0;
         CurrentWidth < Rectangle->Width;
         CurrentWidth += SubdivisionIntervalX)
    {
        
        for (uint CurrentHeight = 0; 
             CurrentHeight < Rectangle->Height; 
             CurrentHeight += SubdivisionIntervalY)
        {
            Rectangle->Vertices[VertexId].Coordinate.x = (-Rectangle->Width/2) + CurrentWidth;
            Rectangle->Vertices[VertexId].Coordinate.y = (-Rectangle->Height/2) + CurrentHeight;
            Rectangle->Vertices[VertexId].Coordinate.z = Height;
            
            Rectangle->Vertices[VertexId].UVCoordinate.x = Rectangle->Vertices[VertexId].Coordinate.x / (Rectangle->Width/2);
            Rectangle->Vertices[VertexId].UVCoordinate.y = Rectangle->Vertices[VertexId].Coordinate.y / (Rectangle->Height/2);
            
            Rectangle->Vertices[VertexId].Normal.x = 0;
            Rectangle->Vertices[VertexId].Normal.y = 0;
            Rectangle->Vertices[VertexId].Normal.z = 1;
            
            VertexId++;
            
            Rectangle->Vertices[VertexId].Coordinate.x = (-Rectangle->Width/2) + CurrentWidth + SubdivisionIntervalX;
            Rectangle->Vertices[VertexId].Coordinate.y = (-Rectangle->Height/2) + CurrentHeight;
            Rectangle->Vertices[VertexId].Coordinate.z = Height;
            
            Rectangle->Vertices[VertexId].UVCoordinate.x = Rectangle->Vertices[VertexId].Coordinate.x / (Rectangle->Width/2);
            Rectangle->Vertices[VertexId].UVCoordinate.y = Rectangle->Vertices[VertexId].Coordinate.y / (Rectangle->Height/2);
            
            Rectangle->Vertices[VertexId].Normal.x = 0;
            Rectangle->Vertices[VertexId].Normal.y = 0;
            Rectangle->Vertices[VertexId].Normal.z = 1;
            
            VertexId++;
        }
    }
}


void CalcRectVertices(rectangle *Rectangle, float Height)
{
    Rectangle->Points[0].x = -Rectangle->Width/2;
    Rectangle->Points[0].y = -Rectangle->Height/2;
    Rectangle->Points[0].z = Height;
    
    Rectangle->Points[1].x = Rectangle->Width/2;
    Rectangle->Points[1].y = -Rectangle->Height/2;
    Rectangle->Points[1].z = Height;
    
    Rectangle->Points[2].x = Rectangle->Width/2;
    Rectangle->Points[2].y = Rectangle->Height/2;
    Rectangle->Points[2].z = Height;
    
    Rectangle->Points[3].x = -Rectangle->Width/2;
    Rectangle->Points[3].y = Rectangle->Height/2;
    Rectangle->Points[3].z = Height;
}

void CalcEllipsePoints(ellipse *Ellipse, float Height)
{
    float Phi = 0.0f;
    
    for (int i = 0; i < SHAPE_VERTICES; i++)
    {
        float NewXPosition = Ellipse->xAxis * cosf(Phi);
        float NewYPosition = Ellipse->yAxis * sinf(Phi);
        
        Ellipse->Points[i].x = NewXPosition;
        Ellipse->Points[i].y = NewYPosition;
        Ellipse->Points[i].z = Height;
        
        Phi = ((float)(i+1) / MaxCircleVertices) * 2 * PI;
    }
}

void CalcCircleVertices(circle *Circle, float Height)
{
    float Phi = 0.0f;
    
    for (int i = 0; i < MaxCircleVertices; i++)
    {
        float CosPhi = cosf(Phi);
        float SinPhi = sinf(Phi);
        float NewXPosition = Circle->Radius * CosPhi;
        float NewYPosition = Circle->Radius * SinPhi;
        
        Circle->Vertices[i].Coordinate.x = NewXPosition;
        Circle->Vertices[i].Coordinate.y = NewYPosition;
        Circle->Vertices[i].Coordinate.z = Height;
        
        Circle->Vertices[i].UVCoordinate.x = CosPhi;
        Circle->Vertices[i].UVCoordinate.y = SinPhi;
        
        float InvNormals = 1.0f;
        if (Circle->InvNormals) InvNormals = -1.0f;
        
        Circle->Vertices[i].Normal.x = 0 * InvNormals;
        Circle->Vertices[i].Normal.y = 0 * InvNormals;
        Circle->Vertices[i].Normal.z = 1 * InvNormals;
        
        Phi = ((float)(i+1) / MaxCircleVertices) * 2 * PI;
    }
}

sphere* CalcSpherePoints(sphere *Sphere, float space) 
{
    float Radius = Sphere->Radius;
    
    Sphere->VertexCount = (180 / space) * 
        (2 + 360 / (2*space)) * 4;
    Sphere->Vertex = new vertex[Sphere->VertexCount];
    Sphere->Radius = Radius;
    
    int n;
    double vR, lVR;
    double hR, lHR;
    double norm;
    n = 0;
    for( vR = 0; vR <= 180-space; vR+=space){
        for(hR = 0; hR <= 360+2*space; hR+=2*space)
        {
            lVR = vR;
            lHR = hR;
            Sphere->Vertex[n].Coordinate.x = 
                Radius * sin(lHR / 180 * M_PI) * sin(lVR / 180 * M_PI);
            Sphere->Vertex[n].Coordinate.y = 
                Radius * cos(lHR / 180 * M_PI) * sin(lVR / 180 * M_PI);
            Sphere->Vertex[n].Coordinate.z = Radius * cos(lVR / 180 * M_PI);
            Sphere->Vertex[n].UVCoordinate.v = lVR / 180;
            Sphere->Vertex[n].UVCoordinate.u = lHR / 360;
            norm = sqrt(
                Sphere->Vertex[n].Coordinate.x*Sphere->Vertex[n].Coordinate.x+
                Sphere->Vertex[n].Coordinate.y*Sphere->Vertex[n].Coordinate.y+
                Sphere->Vertex[n].Coordinate.z*Sphere->Vertex[n].Coordinate.z);
            Sphere->Vertex[n].Normal.x = Sphere->Vertex[n].Normal.x/norm;
            Sphere->Vertex[n].Normal.y = Sphere->Vertex[n].Normal.y/norm;
            Sphere->Vertex[n].Normal.z = Sphere->Vertex[n].Normal.z/norm;
            n++;
            
            lVR = vR + space;
            lHR = hR;
            Sphere->Vertex[n].Coordinate.x = Radius * sin(lHR / 180 * M_PI) * sin(lVR / 180 * M_PI);
            Sphere->Vertex[n].Coordinate.y = Radius * cos(lHR / 180 * M_PI) * sin(lVR / 180 * M_PI);
            Sphere->Vertex[n].Coordinate.z = Radius * cos(lVR / 180 * M_PI);
            Sphere->Vertex[n].UVCoordinate.v = lVR / 180;
            Sphere->Vertex[n].UVCoordinate.u = lHR / 360;
            
            norm = sqrt(
                Sphere->Vertex[n].Coordinate.x*Sphere->Vertex[n].Coordinate.x+
                Sphere->Vertex[n].Coordinate.y*Sphere->Vertex[n].Coordinate.y+
                Sphere->Vertex[n].Coordinate.z*Sphere->Vertex[n].Coordinate.z);
            
            Sphere->Vertex[n].Normal.x = Sphere->Vertex[n].Normal.x/norm;
            Sphere->Vertex[n].Normal.y = Sphere->Vertex[n].Normal.y/norm;
            Sphere->Vertex[n].Normal.z = Sphere->Vertex[n].Normal.z/norm;
            n++;
            
            lVR = vR;
            lHR = hR + space;
            Sphere->Vertex[n].Coordinate.x = Radius * sin(lHR / 180 * M_PI) * sin(lVR / 180 * M_PI);
            Sphere->Vertex[n].Coordinate.y = Radius * cos(lHR / 180 * M_PI) * sin(lVR / 180 * M_PI);
            Sphere->Vertex[n].Coordinate.z = Radius * cos(lVR / 180 * M_PI);
            Sphere->Vertex[n].UVCoordinate.v = lVR / 180;
            Sphere->Vertex[n].UVCoordinate.u = lHR / 360;
            
            norm = sqrt(
                Sphere->Vertex[n].Coordinate.x*Sphere->Vertex[n].Coordinate.x+
                Sphere->Vertex[n].Coordinate.y*Sphere->Vertex[n].Coordinate.y+
                Sphere->Vertex[n].Coordinate.z*Sphere->Vertex[n].Coordinate.z);
            
            Sphere->Vertex[n].Normal.x = Sphere->Vertex[n].Normal.x/norm;
            Sphere->Vertex[n].Normal.y = Sphere->Vertex[n].Normal.y/norm;
            Sphere->Vertex[n].Normal.z = Sphere->Vertex[n].Normal.z/norm;
            n++;
            
            lVR = vR + space;
            lHR = hR + space;
            Sphere->Vertex[n].Coordinate.x = Radius * sin(lHR / 180 * M_PI) * sin(lVR / 180 * M_PI);
            Sphere->Vertex[n].Coordinate.y = Radius * cos(lHR / 180 * M_PI) * sin(lVR / 180 * M_PI);
            Sphere->Vertex[n].Coordinate.z = Radius * cos(lVR / 180 * M_PI);
            Sphere->Vertex[n].UVCoordinate.v = lVR / 180;
            Sphere->Vertex[n].UVCoordinate.u = lHR / 360;
            
            norm = sqrt(
                Sphere->Vertex[n].Coordinate.x*Sphere->Vertex[n].Coordinate.x+
                Sphere->Vertex[n].Coordinate.y*Sphere->Vertex[n].Coordinate.y+
                Sphere->Vertex[n].Coordinate.z*Sphere->Vertex[n].Coordinate.z);
            
            Sphere->Vertex[n].Normal.x = Sphere->Vertex[n].Normal.x/norm;
            Sphere->Vertex[n].Normal.y = Sphere->Vertex[n].Normal.y/norm;
            Sphere->Vertex[n].Normal.z = Sphere->Vertex[n].Normal.z/norm;
            
            n++;
        }
    }
    return Sphere;
}

box * CalcBoxPoints(box * Box)
{
    Box->Vertex[0].Coordinate.x = -Box->Width/2;
    Box->Vertex[0].Coordinate.y = -Box->Height/2;
    Box->Vertex[0].Coordinate.z = -Box->Depth/2;
    
    Box->Vertex[12].Coordinate.x = -Box->Width/2;
    Box->Vertex[12].Coordinate.y = -Box->Height/2;
    Box->Vertex[12].Coordinate.z = -Box->Depth/2;
    
    Box->Vertex[1].Coordinate.x = Box->Width/2;
    Box->Vertex[1].Coordinate.y = -Box->Height/2;
    Box->Vertex[1].Coordinate.z = -Box->Depth/2;
    Box->Vertex[5].Coordinate.x = Box->Width/2;
    Box->Vertex[5].Coordinate.y = -Box->Height/2;
    Box->Vertex[5].Coordinate.z = -Box->Depth/2;
    
    Box->Vertex[2].Coordinate.x = -Box->Width/2;
    Box->Vertex[2].Coordinate.y = Box->Height/2;
    Box->Vertex[2].Coordinate.z = -Box->Depth/2;
    
    Box->Vertex[10].Coordinate.x = -Box->Width/2;
    Box->Vertex[10].Coordinate.y = Box->Height/2;
    Box->Vertex[10].Coordinate.z = -Box->Depth/2;
    
    Box->Vertex[3].Coordinate.x = Box->Width/2;
    Box->Vertex[3].Coordinate.y = Box->Height/2;
    Box->Vertex[3].Coordinate.z = -Box->Depth/2;
    
    Box->Vertex[4].Coordinate.x = Box->Width/2;
    Box->Vertex[4].Coordinate.y = Box->Height/2;
    Box->Vertex[4].Coordinate.z = Box->Depth/2;
    
    Box->Vertex[8].Coordinate.x = Box->Width/2;
    Box->Vertex[8].Coordinate.y = Box->Height/2;
    Box->Vertex[8].Coordinate.z = Box->Depth/2;
    
    Box->Vertex[6].Coordinate.x = Box->Width/2;
    Box->Vertex[6].Coordinate.y = -Box->Height/2;
    Box->Vertex[6].Coordinate.z = Box->Depth/2;
    
    Box->Vertex[7].Coordinate.x = -Box->Width/2;
    Box->Vertex[7].Coordinate.y = -Box->Height/2;
    Box->Vertex[7].Coordinate.z = Box->Depth/2;
    
    Box->Vertex[11].Coordinate.x = -Box->Width/2;
    Box->Vertex[11].Coordinate.y = -Box->Height/2;
    Box->Vertex[11].Coordinate.z = Box->Depth/2;
    
    Box->Vertex[9].Coordinate.x = -Box->Width/2;
    Box->Vertex[9].Coordinate.y = Box->Height/2;
    Box->Vertex[9].Coordinate.z = Box->Depth/2;
}

// TODO: Esta é a forma mais simples e direta ao ponto de se calcular
// os pontos que definem um cilindro no espaco tridimensional, considerando
// a altura desse cilindro.
void CalcCylinderPoints(cylinder *Cylinder, float Height)
{
    float Phi = 0.0f;
    
    // NOTA: Calculando pontos para a base inferior do cilindro
    if (1)
    {
        int Inv = 1;
        if (Cylinder->InvBaseNormals) Inv = -1;
        
        for (int i = 0; i < MaxCircleVertices; i++)
        {
            float CosPhi = cosf(Phi);
            float SinPhi = sinf(Phi);
            float NewXPosition = Cylinder->Radius * CosPhi;
            float NewYPosition = Cylinder->Radius * SinPhi;
            
            Cylinder->BaseVertices[i].Coordinate.x = NewXPosition;
            Cylinder->BaseVertices[i].Coordinate.y = NewYPosition;
            Cylinder->BaseVertices[i].Coordinate.z = 0.0;
            
            Cylinder->BaseVertices[i].Normal = Inv * V3f(0, 0, 1);
            
            Cylinder->BaseVertices[i].UVCoordinate.x = CosPhi;
            Cylinder->BaseVertices[i].UVCoordinate.y = SinPhi;
            
            Phi = ((float)(i+1) / MaxCircleVertices) * 2 * PI;
        }
    }
    
    // NOTA: Calculando pontos para a base superior do cilindro
    if (1)
    {
        int Inv = 1;
        if (Cylinder->InvTopNormals) Inv = -1;
        
        Phi = 0.0f;
        for (int i = 0; i < MaxCircleVertices; i++)
        {
            float CosPhi = cosf(Phi);
            float SinPhi = sinf(Phi);
            float NewXPosition = Cylinder->Radius * CosPhi;
            float NewYPosition = Cylinder->Radius * SinPhi;
            
            Cylinder->TopVertices[i].Coordinate.x = NewXPosition;
            Cylinder->TopVertices[i].Coordinate.y = NewYPosition;
            Cylinder->TopVertices[i].Coordinate.z = Cylinder->Height;
            
            Cylinder->TopVertices[i].Normal = Inv * V3f(0, 0, -1);
            
            Cylinder->TopVertices[i].UVCoordinate.x = CosPhi;
            Cylinder->TopVertices[i].UVCoordinate.y = SinPhi;
            
            Phi = ((float)(i+1) / MaxCircleVertices) * 2 * PI;
        }
        
    }
    
    if (1)
    {
        int Inv = 1;
        if (Cylinder->InvSideNormals) Inv = -1;
        // NOTA: Calculando pontos para a superfície lateral do cilindro
        Cylinder->Vertices[0].Coordinate.x = Cylinder->BaseVertices[0].Coordinate.x;
        Cylinder->Vertices[0].Coordinate.y = Cylinder->BaseVertices[0].Coordinate.y;
        Cylinder->Vertices[0].Coordinate.z = Cylinder->BaseVertices[0].Coordinate.z;
        Cylinder->Vertices[0].Normal = Inv * Normalize(Cylinder->Vertices[0].Coordinate);
        Cylinder->Vertices[0].UVCoordinate.x = 0.0;
        Cylinder->Vertices[0].UVCoordinate.y = 0.0;
        
        Cylinder->Vertices[1].Coordinate.x = Cylinder->BaseVertices[1].Coordinate.x;
        Cylinder->Vertices[1].Coordinate.y = Cylinder->BaseVertices[1].Coordinate.y;
        Cylinder->Vertices[1].Coordinate.z = Cylinder->BaseVertices[1].Coordinate.z;
        Cylinder->Vertices[1].Normal = Inv * Normalize(Cylinder->Vertices[1].Coordinate);
        Cylinder->Vertices[1].UVCoordinate.x = 2 * PI / MaxCircleVertices;
        Cylinder->Vertices[1].UVCoordinate.y = 0.0;
        
        Cylinder->Vertices[2].Coordinate.x = Cylinder->TopVertices[0].Coordinate.x;
        Cylinder->Vertices[2].Coordinate.y = Cylinder->TopVertices[0].Coordinate.y; 
        Cylinder->Vertices[2].Coordinate.z = Cylinder->TopVertices[0].Coordinate.z;
        Cylinder->Vertices[2].Normal = Inv * Normalize(Cylinder->Vertices[2].Coordinate);
        Cylinder->Vertices[2].UVCoordinate.x = 0.0;
        Cylinder->Vertices[2].UVCoordinate.y = 1.0;
        
        Cylinder->Vertices[3].Coordinate.x = Cylinder->TopVertices[1].Coordinate.x;
        Cylinder->Vertices[3].Coordinate.y = Cylinder->TopVertices[1].Coordinate.y;
        Cylinder->Vertices[3].Coordinate.z = Cylinder->TopVertices[1].Coordinate.z;
        Cylinder->Vertices[3].Normal = Inv * Normalize(Cylinder->Vertices[3].Coordinate);
        Cylinder->Vertices[3].UVCoordinate.x = 2 * PI / MaxCircleVertices;
        Cylinder->Vertices[3].UVCoordinate.y = 1.0;
        
        for (int i = 2; i < MaxCircleVertices; i++)
        {
            Cylinder->Vertices[2*i].Coordinate.x = Cylinder->BaseVertices[i].Coordinate.x;
            Cylinder->Vertices[2*i].Coordinate.y = Cylinder->BaseVertices[i].Coordinate.y;
            Cylinder->Vertices[2*i].Coordinate.z = Cylinder->BaseVertices[i].Coordinate.z;
            Cylinder->Vertices[2*i].Normal = Inv * Normalize(Cylinder->Vertices[2*i].Coordinate);
            Cylinder->Vertices[2*i].UVCoordinate.x = i * 2 * PI / MaxCircleVertices;
            Cylinder->Vertices[2*i].UVCoordinate.y = 0.0;
            
            Cylinder->Vertices[2*i+1].Coordinate.x = Cylinder->TopVertices[i].Coordinate.x;
            Cylinder->Vertices[2*i+1].Coordinate.y = Cylinder->TopVertices[i].Coordinate.y;
            Cylinder->Vertices[2*i+1].Coordinate.z = Cylinder->TopVertices[i].Coordinate.z;
            Cylinder->Vertices[2*i+1].Normal = Inv * Normalize(Cylinder->Vertices[2*i+1].Coordinate);
            Cylinder->Vertices[2*i+1].UVCoordinate.x = i * 2 * PI / MaxCircleVertices;
            Cylinder->Vertices[2*i+1].UVCoordinate.y = 1.0;
        }
    }
    
}

void CalcShapePoints(shape *Shape, float Height)
{
    switch(Shape->Type)
    {
        case Shape_Ellipse:
        {
            CalcEllipsePoints(&Shape->Ellipse, Height);
        } break;
        
        case Shape_Circle:
        {
            CalcCircleVertices(&Shape->Circle, Height);
        } break;
        
        case Shape_Rectangle:
        {
            CalcRectVertices(&Shape->Rectangle, Height);
        } break;
        
        case Shape_Sphere:
        {
            CalcSpherePoints(&Shape->Sphere, 10);
        } break;
        
        case Shape_Box:
        {
            CalcBoxPoints(&Shape->Box);
        } break;
        
        case Shape_Cylinder:
        {
            CalcCylinderPoints(&Shape->Cylinder, Height);
        } break;
        
        case Shape_Undefined:
        case Shape_Composed:break;
        
        default:{ASSERT(false);};
    }
}

void CalcShapeTreePoints(shape_tree *Shape)
{
    switch(Shape->Header.Type)
    {
        case Shape_Ellipse:
        {
            CalcEllipsePoints(&Shape->Content.Ellipse, 0.0f);
        } break;
        
        case Shape_Circle:
        {
            CalcCircleVertices(&Shape->Content.Circle, 0.0f);
        } break;
        
        case Shape_Rectangle:
        {
            CalcRectVertices(&Shape->Content.Rectangle, 0.0f);
        } break;
        
        case Shape_Sphere:
        {
            CalcSpherePoints(&Shape->Content.Sphere, 10);
        } break;
        
        case Shape_Box:
        {
            CalcBoxPoints(&Shape->Content.Box);
        } break;
        
        case Shape_Cylinder:
        {
            CalcCylinderPoints(&Shape->Content.Cylinder, 0);
        } break;
        
        case Shape_MeshObject:
        {
            Mesh.draw();
        } break;
        
        case Shape_Undefined:
        case Shape_Composed:
        {
            
        } break;
        
        default:{ASSERT(false);};
    }
}
