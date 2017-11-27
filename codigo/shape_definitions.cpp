#include "shape_definitions.h"

void CalcWallVertices(rectangle *Rectangle, float WallYPos)
{
    Rectangle->Points[0].x = -Rectangle->Width/2;
    Rectangle->Points[0].y = WallYPos;
    Rectangle->Points[0].z = -Rectangle->Height/2;
    
    Rectangle->Points[1].x = Rectangle->Width/2;
    Rectangle->Points[1].y = WallYPos;
    Rectangle->Points[1].z = -Rectangle->Height/2;
    
    Rectangle->Points[2].x = Rectangle->Width/2;
    Rectangle->Points[2].y = WallYPos;
    Rectangle->Points[2].z = Rectangle->Height/2;
    
    Rectangle->Points[3].x = -Rectangle->Width/2;
    Rectangle->Points[3].y = WallYPos;
    Rectangle->Points[3].z = Rectangle->Height/2;
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
        
        Phi = ((float)(i+1) / CIRCLE_VERTICES) * 2 * PI;
    }
}

void CalcCircleVertices(circle *Circle, float Height)
{
    float Phi = 0.0f;
    
    for (int i = 0; i < CIRCLE_VERTICES; i++)
    {
        float CosPhi = cosf(Phi);
        float SinPhi = sinf(Phi);
        float NewXPosition = Circle->Radius * CosPhi;
        float NewYPosition = Circle->Radius * SinPhi;
        
        Circle->Points[i].x = NewXPosition;
        Circle->Points[i].y = NewYPosition;
        Circle->Points[i].z = Height;
        
        Circle->TexelPoints[i].x = CosPhi;
        Circle->TexelPoints[i].y = SinPhi;
        
        Phi = ((float)(i+1) / CIRCLE_VERTICES) * 2 * PI;
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

void CalcCylinderPoints(cylinder *Cylinder, float Height)
{
    
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
        
        case Shape_Composed:
        {
            
        } break;
        
        default:{ASSERT(false);};
    }
}