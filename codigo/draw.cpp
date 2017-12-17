
void DrawSkyBox()
{
    
}

// TODO: Mudar tamanho da textura
inline void DrawImmVertex(vertex Vertex, v3f Origin)
{
    glNormal3f(Vertex.Normal.x, Vertex.Normal.y, Vertex.Normal.z);
    glTexCoord2f(Vertex.UVCoordinate.x, Vertex.UVCoordinate.y);
    Vertex.Coordinate += Origin;
    glVertex3f(Vertex.Coordinate.x, Vertex.Coordinate.y, Vertex.Coordinate.z);
}

void SetMaterialAndTexture(uint TextureId, material Material)
{
    glColor3f(1,1,1);
    
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, Material.Emission.fv);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, Material.ColorA.fv);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, Material.ColorD.fv);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, Material.Specular.fv);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, Material.Shininess.fv);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);//X
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);//Y
    
    glBindTexture(GL_TEXTURE_2D, TextureId);
}

void OpenGLDrawLine(v3f Init, v3f End, v3f Color)
{
    glColor3f(Color.x, Color.y, Color.z);
    glBegin(GL_LINES);
    {
        glVertex3f(Init.x, Init.y, Init.z);
        glVertex3f(End.x, End.y, End.z);
    }
    glEnd();
}

void OpenGLDrawSphere(sphere *Sphere, v3f Center)
{
    glBegin (GL_TRIANGLE_STRIP);
    for (int i = 0; i < Sphere->VertexCount; i++)
    {
        glNormal3f(Sphere->Vertex[i].Normal.x, 
                   Sphere->Vertex[i].Normal.y, 
                   Sphere->Vertex[i].Normal.z);
        
        glTexCoord2f(Sphere->Vertex[i].UVCoordinate.u, Sphere->Vertex[i].UVCoordinate.v);
        
        glVertex3f(Sphere->Vertex[i].Coordinate.x, 
                   Sphere->Vertex[i].Coordinate.y, 
                   Sphere->Vertex[i].Coordinate.z);
    }
    glEnd();
}

void OpenGLDrawCylinder(cylinder *Cylinder, v3f Origin)
{
    if (Cylinder->DrawBase)
    {
        // NOTA: Desenhando base do cilindro
        glBegin(GL_TRIANGLE_FAN);
        {
            glNormal3f(0,0,1);
            glTexCoord2f(0, 0);
            glVertex3f(Origin.x, Origin.y, Origin.z);
            
            float TexScale = 1.0;
            
            for (int i = 0; i < MaxCircleVertices; i++)
            {
                v2f TexCoordScaled = TexScale * Cylinder->BaseVertices[i].UVCoordinate;
                DrawImmVertex(Cylinder->BaseVertices[i], Origin);
            }
            
            v2f TexCoordScaled = TexScale * Cylinder->BaseVertices[0].UVCoordinate;
            DrawImmVertex(Cylinder->BaseVertices[0], Origin);
        }
        glEnd();
        
    }
    
    if (Cylinder->DrawSide)
    {
        // NOTA: Desenhando superficie lateral do cilindro
        glBegin(GL_TRIANGLES);
        {
            DrawImmVertex(Cylinder->Vertices[0], Origin);
            DrawImmVertex(Cylinder->Vertices[1], Origin);
            DrawImmVertex(Cylinder->Vertices[2], Origin);
        }
        glEnd();
        
        glBegin(GL_TRIANGLE_STRIP);
        {
            DrawImmVertex(Cylinder->Vertices[2], Origin);
            DrawImmVertex(Cylinder->Vertices[1], Origin);
            
            for (int i = 3; i < 2 * MaxCircleVertices; i++)
            {
                DrawImmVertex(Cylinder->Vertices[i], Origin);
            }
            
#if 0
            DrawImmVertex(Cylinder->Vertices[0]);
            DrawImmVertex(Cylinder->Vertices[2]);
#else
            v3f Vertex = Cylinder->Vertices[0].Coordinate + Origin;
            glNormal3fv(Cylinder->Vertices[0].Normal.fv);
            glTexCoord2f(2 * PI, 0.0);
            glVertex3fv(Vertex.fv);
            
            Vertex = Cylinder->Vertices[2].Coordinate + Origin;
            glNormal3fv(Cylinder->Vertices[2].Normal.fv);
            glTexCoord2f(2 * PI, 1.0);
            glVertex3fv(Vertex.fv);
#endif
        }
        glEnd();
    }
    
    if (Cylinder->DrawTop)
    {
        // NOTA: Desenhando topo do cilindro
        glBegin(GL_TRIANGLE_FAN);
        {
            glNormal3f(0,0,-1);
            glTexCoord2f(0, 0);
            glVertex3f(Origin.x, Origin.y, Cylinder->Height + Origin.z);
            
            float TexScale = 1.0;
            
            for (int i = 0; i < MaxCircleVertices; i++)
            {
                v2f TexCoordScaled = TexScale * Cylinder->TopVertices[i].UVCoordinate;
                DrawImmVertex(Cylinder->TopVertices[i], Origin);
            }
            
            v2f TexCoordScaled = TexScale * Cylinder->TopVertices[0].UVCoordinate;
            DrawImmVertex(Cylinder->TopVertices[0], Origin);
        }
        glEnd();
    }
}

void OpenGLDrawBox(box * Box, v3f Center)
{
    glBegin(GL_TRIANGLE_STRIP);
    {
        for (int i = 0; i < 13; i++)
        {
            glVertex3f(Box->Vertex[i].Coordinate.x + Center.x, 
                       Box->Vertex[i].Coordinate.y + Center.y, 
                       Box->Vertex[i].Coordinate.z + Center.z);
        }
    }
    glEnd();
}

void OpenGLDrawSubdividedRect(rectangle *Rectangle, v3f Center)
{
    uint MaxYRegions = Rectangle->Height / 10.0;
    uint MaxXRegions = Rectangle->Width / 10.0;
    for (int j = 0; j < MaxYRegions; j++)
    {
        glBegin(GL_TRIANGLE_STRIP);
        {
            for (int i = 0; i < MaxXRegions; i++)
            {
                glNormal3f(Rectangle->Vertices[MaxXRegions*j+i].Normal.x, 
                           Rectangle->Vertices[MaxXRegions*j+i].Normal.y, 
                           Rectangle->Vertices[MaxXRegions*j+i].Normal.z);
                
                glTexCoord2f(Rectangle->Vertices[MaxXRegions*j+i].UVCoordinate.x, 
                             Rectangle->Vertices[MaxXRegions*j+i].UVCoordinate.y);
                
                glVertex3f(Rectangle->Vertices[MaxXRegions*j+i].Coordinate.x + Center.x, 
                           Rectangle->Vertices[MaxXRegions*j+i].Coordinate.y + Center.y, 
                           Rectangle->Vertices[MaxXRegions*j+i].Coordinate.z + Center.z);
            }
        }
        glEnd();
    }
}

void OpenGLDrawRect(rectangle *Rectangle, v3f Center)
{
    glBegin(GL_TRIANGLE_FAN);
    {
        for (int i = 0; i < 4; i++)
        {
            glVertex3f(Rectangle->Points[i].x + Center.x, 
                       Rectangle->Points[i].y + Center.y, 
                       Rectangle->Points[i].z + Center.z);
        }
    }
    glEnd();
}

inline void OpenGLDrawEllipse(ellipse *Ellipse, v3f Center)
{
    glBegin(GL_TRIANGLE_FAN);
    {
        glVertex3f(Center.x, Center.y, Center.z);
        
        for (int i = 0; i < MaxCircleVertices; i++)
        {
            glVertex3f(Ellipse->Points[i].x + Center.x, 
                       Ellipse->Points[i].y + Center.y, 
                       Ellipse->Points[i].z + Center.z);
        }
        
        glVertex3f(Ellipse->Points[0].x + Center.x, 
                   Ellipse->Points[0].y + Center.y, 
                   Ellipse->Points[0].z + Center.z);
    }
    glEnd();
}

inline void OpenGLDrawCircle(circle *Circle, v3f Center)
{
    glBegin(GL_TRIANGLE_FAN);
    {
        float TextureScale = 1.0f;
        
        glNormal3f(0,0,1);
        glTexCoord2f(0, 0);
        glVertex3f(Center.x, Center.y, Center.z);
        
        for (int i = 0; i < MaxCircleVertices; i++)
        {
            glNormal3f(Circle->Vertices[i].Normal.x,
                       Circle->Vertices[i].Normal.y,
                       Circle->Vertices[i].Normal.z);
            
            // TODO: Passar TextureScale como um parâmetro
            
            glTexCoord2f(TextureScale * Circle->Vertices[i].UVCoordinate.x, 
                         TextureScale * Circle->Vertices[i].UVCoordinate.y);
            
            glVertex3f(Circle->Vertices[i].Coordinate.x + Center.x, 
                       Circle->Vertices[i].Coordinate.y + Center.y, 
                       Circle->Vertices[i].Coordinate.z + Center.z);
        }
        
        glNormal3f(Circle->Vertices[0].Normal.x,
                   Circle->Vertices[0].Normal.y,
                   Circle->Vertices[0].Normal.z);
        glTexCoord2f(TextureScale * Circle->Vertices[0].UVCoordinate.x, 
                     TextureScale * Circle->Vertices[0].UVCoordinate.y);
        glVertex3f(Circle->Vertices[0].Coordinate.x + Center.x, 
                   Circle->Vertices[0].Coordinate.y + Center.y, 
                   Circle->Vertices[0].Coordinate.z + Center.z);
    }
    glEnd();
}

void DrawShape(shape *Shape, v3f Position, texture *Texture)
{
    glPushAttrib(GL_ENABLE_BIT);
    {
        if (Texture)
        {
            SetMaterialAndTexture(Texture->Id, Texture->Material);
        }
        
        else
        {
            glDisable(GL_LIGHTING);
            glDisable(GL_TEXTURE_2D);
            
            v4f Color = {
                Colors[Shape->ColorFill].x, 
                Colors[Shape->ColorFill].y,
                Colors[Shape->ColorFill].z,
                1.0};
            glColor3fv(Color.fv);
        }
        
        switch(Shape->Type)
        {
            case Shape_Circle:
            {
                OpenGLDrawCircle(&Shape->Circle, Position);
            } break;
            
            case Shape_Rectangle:
            {
                OpenGLDrawRect(&Shape->Rectangle, Position);
            } break;
            
            case Shape_Ellipse:
            {
                OpenGLDrawEllipse(&Shape->Ellipse, Position);
            } break;
            
            case Shape_Point:
            {
                glBegin(GL_QUADS);
                {
                    glVertex3f(Position.x-3, Position.y-3, Position.z);
                    glVertex3f(Position.x+3, Position.y-3, Position.z);
                    glVertex3f(Position.x+3, Position.y+3, Position.z);
                    glVertex3f(Position.x-3, Position.y+3, Position.z);
                }
                glEnd();
            } break;
            
            case Shape_Sphere:
            {
                OpenGLDrawSphere(&Shape->Sphere, Position);
            } break;
            
            case Shape_Cylinder:
            {
                OpenGLDrawCylinder(&Shape->Cylinder, Position);
            } break;
            
            case Shape_Box:
            {
                OpenGLDrawBox(&Shape->Box, Position);
            } break;
            
            case Shape_SubdividedRectangle:
            {
                OpenGLDrawSubdividedRect(&Shape->Rectangle, Position);
            } break;
            
            case Shape_Undefined:break;
            
            default:{ASSERT(0);};
        }
    }
    glPopAttrib();
    
}

void DrawShapeTree(shape_tree *ShapeTree, v3f Position)
{
    glPushAttrib(GL_ENABLE_BIT);
    {
        texture *Texture = ShapeTree->Header.Texture;
        if (Texture)
        {
            SetMaterialAndTexture(Texture->Id, Texture->Material);
        }
        
        else if (!ShapeTree->Header.BuiltinTexture)
        {
            glDisable(GL_LIGHTING);
            glDisable(GL_TEXTURE_2D);
            
            v4f Color = {
                Colors[ShapeTree->Header.ColorFill].x, 
                Colors[ShapeTree->Header.ColorFill].y,
                Colors[ShapeTree->Header.ColorFill].z,
                1.0};
            glColor3fv(Color.fv);
        }
        
        switch(ShapeTree->Header.Type)
        {
            case Shape_Circle:
            {
                OpenGLDrawCircle(&ShapeTree->Content.Circle, Position);
            } break;
            
            case Shape_Rectangle:
            {
                OpenGLDrawRect(&ShapeTree->Content.Rectangle, Position);
            } break;
            
            case Shape_Ellipse:
            {
                OpenGLDrawEllipse(&ShapeTree->Content.Ellipse, Position);
            } break;
            
            case Shape_Point:
            {
                glBegin(GL_QUADS);
                {
                    glVertex3f(Position.x-3, Position.y-3, Position.z);
                    glVertex3f(Position.x+3, Position.y-3, Position.z);
                    glVertex3f(Position.x+3, Position.y+3, Position.z);
                    glVertex3f(Position.x-3, Position.y+3, Position.z);
                }
                glEnd();
            } break;
            
            case Shape_Sphere:
            {
                OpenGLDrawSphere(&ShapeTree->Content.Sphere, Position);
            } break;
            
            case Shape_Cylinder:
            {
                OpenGLDrawCylinder(&ShapeTree->Content.Cylinder, Position);
            } break;
            
            case Shape_Box:
            {
                OpenGLDrawBox(&ShapeTree->Content.Box, Position);
            } break;
            
            case Shape_SubdividedRectangle:
            {
                OpenGLDrawSubdividedRect(&ShapeTree->Content.Rectangle, Position);
            } break;
            
            case Shape_MeshObject:
            {
                Mesh.draw();
            } break;
            
            case Shape_Undefined:break;
            
            default:{ASSERT(0);};
        }
    }
    glPopAttrib();
    
}
