
void BindTexture(uint TextureId, material Material)
{
    glColor3f(1,1,1);
    
    glMaterialfv(GL_FRONT, GL_EMISSION, Material.Emission.fv);
    glMaterialfv(GL_FRONT, GL_AMBIENT, Material.ColorA.fv);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, Material.ColorD.fv);
    glMaterialfv(GL_FRONT, GL_SPECULAR, Material.Specular.fv);
    glMaterialfv(GL_FRONT, GL_SHININESS, Material.Shininess.fv);
    
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

void OpenGLDrawRect(rectangle *Rectangle, v3f Center)
{
    glBegin(GL_QUADS);
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
        
        for (int i = 0; i < CIRCLE_VERTICES; i++)
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
        glNormal3f(0,0,1);
        glTexCoord2f(0, 0);
        glVertex3f(Center.x, Center.y, Center.z);
        
        for (int i = 0; i < CIRCLE_VERTICES; i++)
        {
            glNormal3f(0,0,1);
            glTexCoord2f(1.0 * Circle->TexelPoints[i].x, 1.0 * Circle->TexelPoints[i].y);
            glVertex3f(Circle->Points[i].x + Center.x, 
                       Circle->Points[i].y + Center.y, 
                       Circle->Points[i].z + Center.z);
        }
        
        glNormal3f(0,0,1);
        glTexCoord2f(1.0 * Circle->TexelPoints[0].x, 1.0 * Circle->TexelPoints[0].y);
        glVertex3f(Circle->Points[0].x + Center.x, 
                   Circle->Points[0].y + Center.y, 
                   Circle->Points[0].z + Center.z);
    }
    glEnd();
}

void DrawShape(shape *Shape, v3f Position, texture *Texture)
{
    glPushAttrib(GL_ENABLE_BIT);
    {
        if (Texture)
        {
            BindTexture(Texture->Id, Texture->Material);
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
            
            case Shape_Box:
            {
                OpenGLDrawBox(&Shape->Box, Position);
            } break;
            
            case Shape_Composed:
            {
            } break;
            
            default:{ASSERT(0);};
        }
    }
    glPopAttrib();
    
}
