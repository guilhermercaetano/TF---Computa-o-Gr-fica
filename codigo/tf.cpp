// Guilherme Rodrigues Caetano - 2013100847
// Marcelo Bringuenti Pedro
// TF Computação Gráfica

#include<stdlib.h>

// Para LoadShader()
#include<string>
#include<fstream>
#include<vector>

#include<time.h>
#include<math.h>
#include<glew.h>
#include<GL/glut.h>
#include<GL/glu.h>
#include<GL/gl.h>
#include<tinyxml/tinystr.h>
#include<tinyxml/tinyxml.h>
#include<objloader/imageloader.h>
#include<objloader/objloader.h>

#define STB_IMAGE_IMPLEMENTATION
#include<stb/stb_image.h>

#include"algebra.h"
#include"tf.h"
#include"shape_definitions.cpp"
#include"draw.cpp"
#include"entity_definitions.cpp"

// NOTA: SkyBox
float points[] = {
    -10.0f,  10.0f, -10.0f,
    -10.0f, -10.0f, -10.0f,
    10.0f, -10.0f, -10.0f,
    10.0f, -10.0f, -10.0f,
    10.0f,  10.0f, -10.0f,
    -10.0f,  10.0f, -10.0f,
    
    -10.0f, -10.0f,  10.0f,
    -10.0f, -10.0f, -10.0f,
    -10.0f,  10.0f, -10.0f,
    -10.0f,  10.0f, -10.0f,
    -10.0f,  10.0f,  10.0f,
    -10.0f, -10.0f,  10.0f,
    
    10.0f, -10.0f, -10.0f,
    10.0f, -10.0f,  10.0f,
    10.0f,  10.0f,  10.0f,
    10.0f,  10.0f,  10.0f,
    10.0f,  10.0f, -10.0f,
    10.0f, -10.0f, -10.0f,
    
    -10.0f, -10.0f,  10.0f,
    -10.0f,  10.0f,  10.0f,
    10.0f,  10.0f,  10.0f,
    10.0f,  10.0f,  10.0f,
    10.0f, -10.0f,  10.0f,
    -10.0f, -10.0f,  10.0f,
    
    -10.0f,  10.0f, -10.0f,
    10.0f,  10.0f, -10.0f,
    10.0f,  10.0f,  10.0f,
    10.0f,  10.0f,  10.0f,
    -10.0f,  10.0f,  10.0f,
    -10.0f,  10.0f, -10.0f,
    
    -10.0f, -10.0f, -10.0f,
    -10.0f, -10.0f,  10.0f,
    10.0f, -10.0f, -10.0f,
    10.0f, -10.0f, -10.0f,
    -10.0f, -10.0f,  10.0f,
    10.0f, -10.0f,  10.0f
};

uint LoadTextureRAW(const char * Filename)
{
    uint Texture;
    
    Image* image = LoadBMP(Filename);
    
    glGenTextures(1, &Texture);
    glBindTexture(GL_TEXTURE_2D, Texture);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE,GL_MODULATE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D,                //Always GL_TEXTURE_2D
                 0,                            //0 for now
                 GL_RGB,                       //Format OpenGL uses for image
                 image->width, image->height,  //Width and height
                 0,                            //The border of the image
                 GL_RGB, //GL_RGB, because pixels are stored in RGB format
                 GL_UNSIGNED_BYTE, //GL_UNSIGNED_BYTE, because pixels are stored
                 //as unsigned numbers
                 image->pixels);               //The actual pixel data
    delete image;
    
    return Texture;
}

inline void 
OpenGLTRSTransform(v3f Position, float AngleInDegrees, v3f Normal, v3f Scale)
{
    glTranslatef(Position.x, Position.y, Position.z);
    glRotatef(AngleInDegrees, Normal.x, Normal.y, Normal.z);
    glScalef(Scale.x, Scale.y, Scale.z);
}

inline void MovePlayerArm(entity_player *Player, v3f DesiredDirection)
{
    float Angle = Player->Transform.Rotation.x;
    v3f xAxis = Player->Bases.xAxis;
    v3f yAxis = Player->Bases.yAxis;
    
    float AngleToTurn =
        acosf(DotProduct(yAxis, DesiredDirection) / (VectorSize(DesiredDirection)));
    
    float AngleToTurnInDegrees = RadsToDegrees(AngleToTurn);
    
    if (DotProduct(xAxis, DesiredDirection) > 0)
    {
        if (AngleToTurnInDegrees > 45.0f)
        {
            Player->RightArm->Header.Transform.Rotation.z = DegreesToRads(-45.0f);
            RotateOrthonormalBases(&Player->RightArm->Header.Bases, DegreesToRads(-45.0f));
        }
        
        else
        {
            Player->RightArm->Header.Transform.Rotation.z = -AngleToTurn;
            RotateOrthonormalBases(&Player->RightArm->Header.Bases, -AngleToTurn);
        }
    }
    
    else
    {
        if (AngleToTurnInDegrees > 45.0f)
        {
            Player->RightArm->Header.Transform.Rotation.z = DegreesToRads(45.0f);
            RotateOrthonormalBases(&Player->RightArm->Header.Bases, DegreesToRads(45.0f));
        }
        
        else
        {
            Player->RightArm->Header.Transform.Rotation.z = AngleToTurn;
            RotateOrthonormalBases(&Player->RightArm->Header.Bases, AngleToTurn);
        }
    }
}

inline entity_list * AllocateEntityListNode()
{
    entity_list *Entities = (entity_list *)malloc(sizeof(entity_list));
    Entities->Content = 0;
    Entities->Next = 0;
    
    return Entities;
}

entity_list * Collision(entity *Entity, shape_tree CollisionShape)
{
    entity_list *EntityList = AllocateEntityListNode();
    entity_list *EntityListHeadNode = EntityList;
    
    uint Count = Game.EntityCount;
    for (entity *TargetEntity = Game.Entities; 
         Count--; 
         TargetEntity++)
    {
        if (TargetEntity != Entity && 
            (TargetEntity->Header.State & EntityState_Active))
        {
            
            switch(TargetEntity->Header.Type)
            {
                case Entity_Player:
                {
                    shape_tree Shape = *TargetEntity->Player.Head;
                    
                    v3f d = Entity->Header.Origin - TargetEntity->Header.Origin;
                    d.z = 0;
                    float dCenter = VectorSize(d);
                    float dRadius = Shape.Content.Sphere.Radius + CollisionShape.Content.Circle.Radius;
                    if (dCenter <= dRadius)
                    {
                        EntityList->Content = TargetEntity;
                        EntityList->Next = new entity_list;
                        EntityList = EntityList->Next;
                    }
                } break;
                
                case Entity_Static:
                case Entity_CenterLimit:
                case Entity_ShortObstacle:
                {
                    shape Shape = TargetEntity->Static.Shape;
                    
                    v3f d = Entity->Header.Origin - TargetEntity->Static.Origin;
                    d.z = 0;
                    float dCenter = VectorSize(d);
                    float dRadius = 1.05 * Shape.Circle.Radius + CollisionShape.Content.Circle.Radius;
                    if (dCenter <= dRadius)
                    {
                        EntityList->Content = TargetEntity;
                        EntityList->Next = new entity_list;
                        EntityList = EntityList->Next;
                    }
                    
                } break;
                
                case Entity_Enemy:
                {
                    shape_tree Shape = *TargetEntity->Enemy.Head;
                    
                    v3f d = Entity->Header.Origin - TargetEntity->Header.Origin;
                    d.z = 0;
                    float dCenter = VectorSize(d);
                    float dRadius = Shape.Content.Sphere.Radius + CollisionShape.Content.Circle.Radius;
                    if (dCenter <= dRadius)
                    {
                        EntityList->Content = TargetEntity;
                        EntityList->Next = new entity_list;
                        EntityList = EntityList->Next;
                    }
                } break;
                
                case Entity_Background:
                {
                    shape Shape = TargetEntity->Background.Shape;
                    
                    float CircleRadius = CollisionShape.Content.Circle.Radius;
                    v3f d = Entity->Header.Origin + CircleRadius - Shape.Origin;
                    d.z = 0;
                    float dCenter = VectorSize(d);
                    float dRadius = Shape.Circle.Radius + CircleRadius;
                    
                    if (dCenter <= dRadius)
                    {
                        EntityList->Content = TargetEntity;
                        EntityList->Next = new entity_list;
                        EntityList = EntityList->Next;
                    }
                } break;
            }
        }
    }
    
    EntityList = EntityListHeadNode;
    
    return EntityList;
}

// NOTA: Movimento bidimensional
void MoveEntity(entity *Entity, game_dynamics *Dynamics, v3f Velocity, shape_tree Shape)
{
    v3f NewRelP = {};
    NewRelP.x = Velocity.x * Game.Timing.deltaTime;
    NewRelP.y = Velocity.y * Game.Timing.deltaTime;
    NewRelP.z = 0;
    
    v3f NewdeltaP = NewRelP;
    uint MaxLoops = 3;
    
    for (int i = 0; i < MaxLoops; i++)
    {
        Dynamics->Collision = false;
        
        entity_list *EntitiesCollided = Collision(Entity, Shape);
        for (entity_list *Entities = EntitiesCollided; Entities; Entities = Entities->Next)
        {
            if (Entities->Content && (Entities->Content != Entity))
            {
                switch(Entities->Content->Header.Type)
                {
                    case Entity_Enemy:
                    case Entity_CenterLimit:
                    case Entity_Player:
                    {
                        float Offset = 20.0;
                        v3f d = Entity->Header.Origin + Offset + NewdeltaP - Entities->Content->Header.Origin;
                        float dCenter = VectorSize(d);
                        v3f Normal = d / dCenter;
                        Velocity = Velocity - 1 * DotProduct(Velocity, Normal) * Normal;
                        NewdeltaP += 0.05f * Velocity * Game.Timing.deltaTime + Normal;
                        Dynamics->Collision = true;
                        //Entity->Enemy.VelocityMagnitude = -Entity->Enemy.VelocityMagnitude;
                    } break;
                    
                    case Entity_ShortObstacle:
                    {
                        if (Entity->Header.Origin.z <= Entities->Content->Header.Height)
                        {
                            float Offset = 20.0;
                            v3f OffsetFromObstacle = Offset + NewdeltaP;
                            v3f d = Entity->Header.Origin + Offset + NewdeltaP - Entities->Content->Header.Origin;
                            float dCenter = VectorSize(d);
                            v3f Normal = d / dCenter;
                            Velocity = Velocity - 1 * DotProduct(Velocity, Normal) * Normal;
                            NewdeltaP += 0.05f * Velocity * Game.Timing.deltaTime + Normal;
                            Dynamics->Collision = true;
                            
                            if (!Entity->Enemy.Jumping && Entity->Enemy.Velocity.z == 0.0f) 
                            {
                                Entity->Enemy.Dynamics.Ascending = true;
                                Entity->Enemy.Jumping = true;
                                
                                // A velocidade é determinada usando a equação da conservação de energia
                                // para corpos em queda live
                                float GH = 
                                    GRAVITY_ACCELERATION * GravityAccelerationScale * JumpHeight;
                                Entity->Enemy.Velocity.z = sqrt(2 * GH);
                            }
                        }
                    } break;
                    
                    case Entity_Background:
                    {
                        float Offset = 50.0;
                        v3f d = Entities->Content->Background.Origin - NewRelP - Entity->Header.Origin;
                        d.z = 0;
                        float dCenter = VectorSize(d) + Shape.Content.Circle.Radius;
                        if (dCenter + Offset >= Entities->Content->Background.Shape.Circle.Radius)
                        {
                            v3f Normal = Normalize(d);
                            Velocity = Velocity - 1 * DotProduct(Velocity, Normal) * Normal;
                            NewdeltaP += Velocity * Game.Timing.deltaTime + Normal;
                            Dynamics->Collision = true;
                        }
                        
                    } break;
                }
            }
        }
        
        if (Dynamics->Collision)
        {
#if 1
            NewRelP = NewdeltaP;
            Velocity = -1 * Velocity;
            //Entity->Enemy.VelocityMagnitude = -Entity->Enemy.VelocityMagnitude;
            //Entity->Enemy.CountToChangeWalkingDirection = Entity->Enemy.CyclesToChangeWalkingDirection;
#else
            if (i >= MaxLoops-1)
            {
                NewRelP = V3f(0, 0, 0);
                //Dynamics->Collision = false;
            }
            
            else
            {
                NewRelP = NewdeltaP;
                Velocity = -1 * Velocity;
                Entity->Enemy.VelocityMagnitude = -Entity->Enemy.VelocityMagnitude;
                Entity->Enemy.CountToChangeWalkingDirection = Entity->Enemy.CyclesToChangeWalkingDirection;
            }
#endif
        }
        
        else 
        {
            free(EntitiesCollided);
            break;
        }
        
        NewdeltaP = NewRelP + Velocity * Game.Timing.deltaTime;
        
        free(EntitiesCollided);
    }
    
    if (VectorSize(NewRelP) > 0.0f)
    {
        Entity->Header.Origin.x += NewRelP.x;
        Entity->Header.Origin.y += NewRelP.y;
        if (Entity->Header.Type == Entity_Enemy)
        {
            Entity->Enemy.Position.x = Entity->Header.Origin.x;
            Entity->Enemy.Position.y = Entity->Header.Origin.y;
        }
    }
}

inline bool
IsEntityIn(entity_list *Entities, entity_type Type)
{
    for (entity_list *EntityEntry = Entities;
         EntityEntry; EntityEntry = EntityEntry->Next)
    {
        if (EntityEntry->Content && (EntityEntry->Content->Header.Type == Type))
        {
            return true;
        }
    }
    
    return false;
}

inline uint
GetEntityCountWithState(entity_list *Entities, entity_type Type, entity_state State)
{
    uint Count = 0;
    for (entity_list *EntityEntry = Entities;
         EntityEntry; EntityEntry = EntityEntry->Next)
    {
        if (EntityEntry->Content && (EntityEntry->Content->Header.Type == Type) &&
            (EntityEntry->Content->Header.State & State))
        {
            Count++;
        }
    }
    
    return Count;
}


inline uint
GetEntityCount(entity_list *Entities, entity_type Type)
{
    uint Count = 0;
    for (entity_list *EntityEntry = Entities;
         EntityEntry; EntityEntry = EntityEntry->Next)
    {
        if (EntityEntry->Content && (EntityEntry->Content->Header.Type == Type))
        {
            Count++;
        }
    }
    
    return Count;
}

void EnemyJump(entity_enemy *Enemy)
{
    Enemy->Header->Origin.z = Enemy->Header->Origin.z + Enemy->Velocity.z * Game.Timing.deltaTime;
    
    float HeightToJump = fabs(JumpHeight - Enemy->Header->Origin.z);
    
    if (Enemy->Dynamics.Ascending)
    {
        float GH = GravityAccelerationScale * GRAVITY_ACCELERATION * HeightToJump;
        Enemy->Velocity.z = sqrt(2 * GH);
    }
    else
    {
        float GH = GravityAccelerationScale * GRAVITY_ACCELERATION * HeightToJump;
        Enemy->Velocity.z = -1 * sqrt(2 * GH);
    }
    
    if (Enemy->Velocity.z < 5.0) 
    {
        Enemy->Dynamics.Ascending = false;
    }
    
    if (Enemy->Position.z <= Game.Arena.Background->Header->Height) 
    {
        Enemy->Velocity.z = 0.0f;
        Enemy->Header->Origin.z = Game.Arena.Background->Header->Height;
        Enemy->Jumping = false;
        return;
    }
}

void EnemyCollisionZ(arena Arena, entity_enemy *Enemy)
{
    bool PlatformAllowFlag = false;
    
    uint Count = Game.EntityCount;
    for (entity *JumpingPlatform = Game.Entities; 
         Count--; 
         JumpingPlatform++)
    {
        if (JumpingPlatform->Header.Type == Entity_ShortObstacle && 
            (JumpingPlatform->Header.Height > 0.0f))
        {
            v3f d = Enemy->Header->Origin - JumpingPlatform->Static.Origin;
            
            float MinkowskiRadius = JumpingPlatform->Static.Shape.Circle.Radius + Enemy->Head->Content.Sphere.Radius;
            if (VectorSize(d) <= MinkowskiRadius)
            {
                PlatformAllowFlag = true;
                
                if (Enemy->Velocity.z < 0.0f && 
                    Enemy->Header->Origin.z <= JumpingPlatform->Header.Height)
                {
                    Enemy->Velocity.z = 0.0f;
                    Enemy->Header->Origin.z = JumpingPlatform->Header.Height;
                    Enemy->Jumping = false;
                }
            }
        }
    }
    
    if (PlatformAllowFlag) Enemy->Dynamics.PlatformAllow = true;
    else 
    {
        if (Enemy->Dynamics.PlatformAllow && 
            Enemy->Velocity.z == 0.0f)
        {
            Enemy->Dynamics.FreeFalling = true;
        }
        Enemy->Dynamics.PlatformAllow = false;
    }
    
    if (Enemy->Dynamics.FreeFalling)
    {
        bool SaveAscending = Enemy->Dynamics.Ascending;
        Enemy->Dynamics.Ascending = false;
        EnemyJump(Enemy);
        if (Enemy->Header->Origin.z <= 0.0f)
        {
            Enemy->Header->Origin.z = 0.0f;
            Enemy->Velocity.z = 0.0f;
            Enemy->Dynamics.FreeFalling = false;
        }
        Enemy->Dynamics.Ascending = SaveAscending;
    }
}

void ShapeDrawWithTexture(shape_tree *ShapeTree)
{
    //glPushMatrix();
    //{
    v3f Position = ShapeTree->Header.Origin;
    v3f Scale = ShapeTree->Header.Transform.Scale;
    v3f AngleInRadians = ShapeTree->Header.Transform.Rotation;
    
    glTranslatef(Position.x, Position.y, Position.z);
    v3f RotNormal = ShapeTree->Header.RotationNormal;
    glRotatef(RadsToDegrees(AngleInRadians.x), 1, 0, 0);
    glRotatef(RadsToDegrees(AngleInRadians.y), 0, 1, 0);
    glRotatef(RadsToDegrees(AngleInRadians.z), 0, 0, 1);
    glScalef(Scale.x, Scale.y, Scale.z);
    glGetFloatv(GL_MODELVIEW_MATRIX, (float *)ShapeTree->Header.ModelViewMatrix.fv);
    
    glPushMatrix();
    v3f LocalTranslate = ShapeTree->Header.LocalTranslate;
    v3f LocalRotate = ShapeTree->Header.LocalRotate;
    v3f LocalScale = ShapeTree->Header.LocalScale;
    
    glTranslatef(LocalTranslate.x, LocalTranslate.y, LocalTranslate.z);
    glRotatef(LocalRotate.x, 1, 0, 0);
    glRotatef(LocalRotate.y, 0, 1, 0);
    glRotatef(LocalRotate.z, 0, 0, 1);
#if 0
    glScalef(LocalScale.x, LocalScale.y,LocalScale.z);
#endif
    
    DrawShapeTree(ShapeTree, ShapeTree->Header.OffsetFromOrigin);
    glPopMatrix();
    //}
    //glPopMatrix();
}

void UpdateAndDrawEntity(entity *Entity)
{
    switch(Entity->Header.Type)
    {
        case Entity_Player:
        {
            if (Entity->Header.State & EntityState_Active)
            {
                
                entity_player *Player = &Entity->Player;
                
                // NOTA: Movimento do braço
                v2f MouseScreenRel;
                MouseScreenRel.x = Game.Input.Mouse.Position.x - WindowWidth*0.5f;
                MouseScreenRel.y = Game.Input.Mouse.Position.y - WindowHeight*0.5f;
                MouseScreenRel.x /= WindowWidth*0.5f;
                MouseScreenRel.y /= WindowHeight*0.5f;
                
                Player->RightArm->Header.RotationNormal = V3f(0.0f, 0.0f, 1.0f);
                
                float ArmRestAngleInRads = PI*0.50f;
                float MaxRotationAngle = PI*0.25f;
                float XAbsRotation = (ArmRestAngleInRads + MaxRotationAngle * MouseScreenRel.y);
                float YAbsRotation = (-MaxRotationAngle * MouseScreenRel.x);
                
                if (MouseScreenRel.y > 1.0f)
                {
                    XAbsRotation = (ArmRestAngleInRads + MaxRotationAngle);
                }
                
                printf("%.3f\n", XAbsRotation);
                
                GunTurnX = YAbsRotation;
                GunTurnY = XAbsRotation;
                
                static uint Movement = 0;
                //Player->RightArm->Header.Transform.Rotation.x = 
                //XAbsRotation + 0.02f*sinf(0.0001f*VectorSize(Game.Player->Player.Velocity)*Movement++);
                Player->RightArm->Header.Transform.Rotation.x = XAbsRotation;
                Player->RightArm->Header.Transform.Rotation.y = YAbsRotation;
                //Player->RightArm->Header.Transform.Rotation.z = 0.1f;
                
                Player->Velocity.x *= 0.6f;
                Player->Velocity.y *= 0.6f;
            }
            
            if (Entity->Header.State & EntityState_Visible)
            {
                bases PlayerBases = Game.Player->Player.Bases;
                glPushMatrix();
                v3f PlayerP = Entity->Player.Position;
                v3f Scale = Entity->Player.Transform.Scale;
                float FacingAngle = RadsToDegrees(Entity->Player.Transform.Rotation.z);
                OpenGLTRSTransform(PlayerP, FacingAngle, V3f(0, 0, 1), Scale);
                ShapeTreeWalk(Entity->Player.BodyTree, ShapeDrawWithTexture);
                glPopMatrix();
            }
            
        } break;
        
        case Entity_Bullet:
        {
            if (Entity->Header.State & EntityState_Active)
            {
                Entity->Bullet.Position += Entity->Bullet.VelocityMagnitude * Entity->Bullet.Bases.yAxis * Game.Timing.deltaTime;
                Entity->Bullet.Header->Origin = Entity->Bullet.Position;
                
                // TODO: A constante alocação de entidades é ineficiente.
                // substituir por um modelo mais viável.
                shape_tree ShapeTree = {};
                ShapeTree.Header = Entity->Bullet.Shape.Header;
                ShapeTree.Content.Circle = Entity->Bullet.Shape.Circle;
                entity_list *Entities = Collision(Entity, ShapeTree);
                
                if (!IsEntityIn(Entities, Entity_Background))
                {
                    Entity->Header.State &= ~EntityState_Visible;
                    Entity->Header.State &= ~EntityState_Active;
                }
                
                for (entity_list *EntityEntry = Entities;
                     EntityEntry; EntityEntry = EntityEntry->Next)
                {
                    if (EntityEntry->Content && 
                        (EntityEntry->Content->Header.Type == Entity_CenterLimit || 
                         EntityEntry->Content->Header.Type == Entity_TallObstacle))
                    {
                        Entity->Header.State &= ~EntityState_Visible;
                        Entity->Header.State &= ~EntityState_Active;
                    }
                    
                    else if (EntityEntry->Content && 
                             EntityEntry->Content->Header.Type == Entity_ShortObstacle &&
                             Entity->Bullet.Header->Origin.z <= EntityEntry->Content->Header.Height)
                    {
                        Entity->Header.State &= ~EntityState_Visible;
                        Entity->Header.State &= ~EntityState_Active;
                    }
                    
                    if (EntityEntry->Content &&
                        EntityEntry->Content->Header.Type == Entity_Enemy)
                    {
                        Entity->Header.State &= ~EntityState_Visible;
                        Entity->Header.State &= ~EntityState_Active;
                        
                        if (Entity->Bullet.CastingEntityType != Entity_Enemy)
                        {
                            EntityEntry->Content->Header.State &= ~EntityState_Visible;
                            EntityEntry->Content->Header.State &= ~EntityState_Active;
                            
                            // TODO: Dinamismo para não precisar de loops a cada atualização dos minimapas
                            for (uint i = 0; i < MinimapEntitiesCount; i++)
                            {
                                if (MinimapEntities[i].Header.Type == Entity_MiniEnemy &&
                                    MinimapEntities[i].Header.Id == EntityEntry->Content->Header.Id)
                                {
                                    MinimapEntities[i].Header.State &= 
                                        ~(EntityState_Visible|EntityState_Active);
                                }
                            }
                        }
                    }
                    
                    if (EntityEntry->Content &&
                        EntityEntry->Content->Header.Type == Entity_Player)
                    {
                        Entity->Header.State &= ~EntityState_Visible;
                        Entity->Header.State &= ~EntityState_Active;
                        
                        if (Entity->Bullet.CastingEntityType != Entity_Player)
                        {
                            EntityEntry->Content->Header.State &= ~EntityState_Visible;
                            EntityEntry->Content->Header.State &= ~EntityState_Active;
                            
                            // TODO: Dinamismo para não precisar de loops a cada atualização dos minimapas
                            for (uint i = 0; i < MinimapEntitiesCount; i++)
                            {
                                if (MinimapEntities[i].Header.Type == Entity_MiniPlayer &&
                                    MinimapEntities[i].Header.Id == EntityEntry->Content->Header.Id)
                                {
                                    MinimapEntities[i].Header.State &= 
                                        ~(EntityState_Visible|EntityState_Active);
                                }
                            }
                        }
                    }
                    
                }
                
                delete Entities;
            }
            
            if (Entity->Header.State & EntityState_Visible)
            {
                DrawShape(&Entity->Bullet.Shape, Entity->Bullet.Position, &BulletTexture);
            }
        } break;
        
        case Entity_Enemy:
        {
            if (Entity->Header.State & EntityState_Active)
            {
                Entity->Enemy.CyclesToShoot--;
                if (Entity->Enemy.CyclesToShoot <= 0)
                {
                    Entity->Enemy.CyclesToShoot = Game.EnemyCountToShoot;
                    //ASSERT(Entity->Enemy.Body.RightArm.Bases.Angle == 0);
                    CreateBulletEntity(GlobalBullets, Entity_Enemy, 100, Entity->Enemy.Position.z + Entity->Enemy.ArmHeight, 
                                       Entity->Enemy.ShotVelocity, Entity->Enemy.Bases, 
                                       Entity->Enemy.Transform.Rotation, Entity->Enemy.RightArm,
                                       Entity->Enemy.Position);
                    
                    if ((GlobalBullets-SaveGlobalBullets) < 99)
                    {
                        GlobalBullets++;
                    }
                    else
                    {
                        GlobalBullets = SaveGlobalBullets;
                    }
                }
                
                Entity->Enemy.CyclesToChangeFootDirection -= (int)(Game.EnemyVelocity/100);
                
                if (Entity->Enemy.CyclesToChangeFootDirection <= 0 && !Entity->Enemy.Jumping)
                {
                    Entity->Enemy.CyclesToChangeFootDirection = 30;
                    Entity->Enemy.RightLeg->Header.Origin.y *= -1;
                    Entity->Enemy.LeftLeg->Header.Origin.y *= -1;
                    //Entity->Enemy.Body.RightLeg[1].Origin.y *= -1;
                    //Entity->Enemy.Body.LeftLeg[1].Origin.y *= -1;
                }
                
                // Padrão de movimentação dos inimigos
                Entity->Enemy.Velocity.x = Entity->Enemy.VelocityMagnitude/2 * Entity->Enemy.Bases.yAxis.x;
                Entity->Enemy.Velocity.y = Entity->Enemy.VelocityMagnitude/2 * Entity->Enemy.Bases.yAxis.y;
                if (Entity->Enemy.CountToChangeWalkingDirection == 0)
                {
                    Entity->Enemy.CountToChangeWalkingDirection = Entity->Enemy.CyclesToChangeWalkingDirection;
                    Entity->Enemy.VelocityMagnitude = -Entity->Enemy.VelocityMagnitude;
                }
                
                Entity->Enemy.CountToChangeWalkingDirection--;
                
                MoveEntity(Entity, &Entity->Enemy.Dynamics, Entity->Enemy.Velocity, 
                           *Entity->Enemy.Head);
                
                if (Entity->Enemy.Jumping)
                {
                    EnemyJump(&Entity->Enemy);
                }
                EnemyCollisionZ(Game.Arena, &Entity->Enemy);
            }
            
            
            if (Entity->Header.State & EntityState_Visible)
            {
                
                glPushMatrix();
                v3f EnemyP = Entity->Enemy.Position;
                v3f Scale = Entity->Enemy.Transform.Scale;
                float FacingAngle = RadsToDegrees(Entity->Enemy.Transform.Rotation.z);
                OpenGLTRSTransform(EnemyP, FacingAngle, V3f(0, 0, 1), Scale);
                ShapeTreeWalk(Entity->Enemy.BodyTree, ShapeDrawWithTexture);
                glPopMatrix();
            }
        } break;
        
        case Entity_Wall:
        case Entity_CenterLimit:
        case Entity_MiniPlayer:
        case Entity_MiniEnemy:
        case Entity_MiniPlatform:
        {
            if (Entity->Header.State & EntityState_Visible)
            {
                glPushMatrix();
                v3f Scale = Entity->Static.Shape.Transform.Scale;
                glScalef(Scale.x, Scale.y, Scale.z);
                DrawShape(&Entity->Static.Shape, 
                          Entity->Static.Origin, 
                          Entity->Static.Shape.Texture);
                glPopMatrix();
            }
            
        } break;
        
        case Entity_ShortObstacle:
        {
            DrawShape(&Entity->Static.Shape, Entity->Static.Origin, &FloorTexture);
        } break;
        
        case Entity_Static:
        {
            DrawShape(&Entity->Static.Shape, Entity->Static.Origin, 0);
        } break;
        
        case Entity_Background:
        {
            DrawShape(&Entity->Background.Shape, Entity->Background.Origin, 0);
        } break;
        
        default: {ASSERT(false);}
    }
}

void 
SVGDefinitionsToEntities(svg_circle *SVGCircles, int CirclesCount)
{
    entity *Entity = Game.Entities;
    
    v3f CircleCenter = {};
    float CenterRadius = 0.0f;
    
    for (svg_circle *SVGC = SVGCircles; 
         CirclesCount > 0; SVGC++, 
         Entity++, CirclesCount--)
    {
        ASSERT(MinimapEntitiesCount < 100);
        if (strncmp(SVGC->Fill, "blue", strlen("blue")) == 0)
        {
            v3f Center = {SVGC->CenterX, SVGC->CenterY, 0.0f};
            Game.Arena.Background = 
                CreateBackground(Entity, SVGC->Id, 0.0, SVGC->Radius, Center, Blue);
            
            uint PixelsTall = 240;
            
            CircleCenter.x = Center.x - SVGC->Radius;
            CircleCenter.y = Center.y - SVGC->Radius;
            CenterRadius = SVGC->Radius;
            
            uint Incr = 60;
            for (uint i = 0; i < PixelsTall; i += Incr)
            {
                Game.EntityCount++;
                Entity++;
                Center = {SVGC->CenterX, SVGC->CenterY, (float)i};
                cylinder Cylinder = {};
                Cylinder.Radius = SVGC->Radius;
                Cylinder.Height = Incr;
                Cylinder.InvSideNormals = true;
                Cylinder.DrawBase = false;
                Cylinder.DrawTop = false;
                Cylinder.DrawSide = true;
                
                uint BaseId = SVGC->Id + 200;
                uint Id = BaseId + (i / 30);
                
                CreateExternalWall(Entity, Id, Center, Cylinder, &WallTexture);
            }
        }
        
        else if (strncmp(SVGC->Fill, "white", strlen("white")) == 0)
        {
            float PixelsTall = 200.0;
            v3f Center = {SVGC->CenterX, SVGC->CenterY, 0.0};
            
            uint Incr = 40;
            for (int i = 0; i < PixelsTall; i += Incr)
            {
                
                Center = {SVGC->CenterX, SVGC->CenterY, (float)i};
                cylinder Cylinder = {};
                Cylinder.Radius = SVGC->Radius;
                Cylinder.Height = Incr;
                Cylinder.InvSideNormals = false;
                Cylinder.DrawBase = false;
                Cylinder.DrawTop = false;
                Cylinder.DrawSide = true;
                
                uint BaseId = SVGC->Id + 300;
                uint Id = BaseId + (i / 30);
                
                CreateInternalWall(Entity, Id, Center, Cylinder, &WallTexture);
                Game.EntityCount++;
                Entity++;
            }
            
            entity CentralCircle = {};
            entity_header CentralCircleHeader = {};
            CentralCircleHeader.Id = SVGC->Id;
            CentralCircleHeader.Type = Entity_CenterLimit;
            CentralCircleHeader.Height = PixelsTall;
            CentralCircleHeader.Origin = 
            {SVGC->CenterX/MinimapReduxFactor, SVGC->CenterY/MinimapReduxFactor, 0.0f};
            CentralCircleHeader.State = EntityState_Visible;
            CentralCircle.Header = CentralCircleHeader;
            CentralCircle.Static.Header = &CentralCircleHeader;
            CentralCircle.Static.Origin = {
                SVGC->CenterX/MinimapReduxFactor, 
                SVGC->CenterY/MinimapReduxFactor, 
                0.0f};
            CentralCircle.Static.Shape.ColorFill = White;
            CentralCircle.Static.Shape.Type = Shape_Circle;
            CentralCircle.Static.Shape.Origin = Center;
            CentralCircle.Static.Shape.Bases = 
            {
                V3f(1.0f, 0.0f, 0.0f), 
                V3f(0.0f, 1.0f, 0.0f), 
                V3f(0.0f, 0.0f, 1.0f)
            };
            
            CentralCircle.Static.Shape.Transform = {};
            CentralCircle.Static.Shape.Transform.Translation = {0.0f, 0.0f, 0.0f};
            CentralCircle.Static.Shape.Transform.Rotation = {0.0f, 0.0f, 0.0f};
            CentralCircle.Static.Shape.Transform.Scale = {1, 1, 1};
            
            CentralCircle.Static.Shape.Circle.Radius = SVGC->Radius / MinimapReduxFactor;
            CalcShapePoints(&CentralCircle.Static.Shape, 0.0f);
            
            MinimapEntities[MinimapEntitiesCount++] = CentralCircle;
        }
        
        else if (strncmp(SVGC->Fill, "green", strlen("green")) == 0)
        {
            float PixelsTall = 100.0f;
            v3f Center = {SVGC->CenterX, SVGC->CenterY, 0.0f};
            CreatePlayerTree(Entity, SVGC->Id, 10.0, SVGC->Radius, Center);
            Game.Player = Entity;
            
            entity MiniPlayer = {};
            entity_header MiniPlayerHeader = {};
            MiniPlayerHeader.Id = SVGC->Id;
            MiniPlayerHeader.Type = Entity_MiniPlayer;
            MiniPlayerHeader.Height = PixelsTall;
            MiniPlayerHeader.Origin = Center / MinimapReduxFactor;
            MiniPlayerHeader.State = EntityState_Visible | EntityState_Active;
            MiniPlayer.Header = MiniPlayerHeader;
            MiniPlayer.Static.Header = &MiniPlayerHeader;
            MiniPlayer.Static.Origin = MiniPlayerHeader.Origin;
            MiniPlayer.Static.Shape.ColorFill = Green;
            MiniPlayer.Static.Shape.Type = Shape_Circle;
            MiniPlayer.Static.Shape.Origin = MiniPlayerHeader.Origin;
            MiniPlayer.Static.Shape.Bases = 
            {
                V3f(1.0f, 0.0f, 0.0f), 
                V3f(0.0f, 1.0f, 0.0f), 
                V3f(0.0f, 0.0f, 1.0f)
            };
            
            MiniPlayer.Static.Shape.Transform = {};
            MiniPlayer.Static.Shape.Transform.Translation = {0.0f, 0.0f, 0.0f};
            MiniPlayer.Static.Shape.Transform.Rotation = {0.0f, 0.0f, 0.0f};
            MiniPlayer.Static.Shape.Transform.Scale = {1, 1, 1};
            
            MiniPlayer.Static.Shape.Circle.Radius = SVGC->Radius / MinimapReduxFactor;
            MiniPlayer.Static.Shape.Circle.CollisionRadius = SVGC->Radius / 3000.0f;
            
            MinimapEntities[MinimapEntitiesCount++] = MiniPlayer;
        }
        
        else if (strncmp(SVGC->Fill, "red", strlen("red")) == 0)
        {
            float PixelsTall = 100.0f;
            
            v3f Center = {SVGC->CenterX, SVGC->CenterY, 0.0f};
            // TODO: Representação local de inimigos
            entity_enemy *Enemy = &(Game.Enemies[Game.EnemyCount++]);
            Game.Enemies = CreateEnemyTree(Entity, SVGC->Id, 10.0, SVGC->Radius, Center);
            
            entity MiniEnemy = {};
            entity_header MiniEnemyHeader = {};
            MiniEnemyHeader.Id = SVGC->Id;
            MiniEnemyHeader.Type = Entity_MiniEnemy;
            MiniEnemyHeader.Height = PixelsTall;
            MiniEnemyHeader.Origin = Center / MinimapReduxFactor;
            MiniEnemyHeader.State = EntityState_Visible | EntityState_Active;
            MiniEnemy.Header = MiniEnemyHeader;
            MiniEnemy.Static.Header = &MiniEnemyHeader;
            MiniEnemy.Static.Origin = MiniEnemyHeader.Origin;
            MiniEnemy.Static.Shape.ColorFill = Red;
            MiniEnemy.Static.Shape.Type = Shape_Circle;
            MiniEnemy.Static.Shape.Origin = MiniEnemyHeader.Origin;
            MiniEnemy.Static.Shape.Bases = 
            {
                V3f(1.0f, 0.0f, 0.0f), 
                V3f(0.0f, 1.0f, 0.0f), 
                V3f(0.0f, 0.0f, 1.0f)
            };
            
            MiniEnemy.Static.Shape.Transform = {};
            MiniEnemy.Static.Shape.Transform.Translation = {0.0f, 0.0f, 0.0f};
            MiniEnemy.Static.Shape.Transform.Rotation = {0.0f, 0.0f, 0.0f};
            MiniEnemy.Static.Shape.Transform.Scale = {1, 1, 1};
            
            MiniEnemy.Static.Shape.Circle.Radius = SVGC->Radius / MinimapReduxFactor;
            MiniEnemy.Static.Shape.Circle.CollisionRadius = MiniEnemy.Static.Shape.Circle.Radius;
            
            MinimapEntities[MinimapEntitiesCount++] = MiniEnemy;
        }
        
        else if (strncmp(SVGC->Fill, "black", strlen("black")) == 0)
        {
            // TODO: Revisar esta fórmula
            float PlatformHeight = JumpHeight * (float)(Game.ObstaclesHeight / 100.0);
            v3f Center = {SVGC->CenterX, SVGC->CenterY, 0.0};
            CreateEntityStatic(Entity, Entity_ShortObstacle, Black, SVGC->Id,
                               PlatformHeight, SVGC->Radius, Center, false, true, true, true);
            
            entity MiniPlatform = {};
            entity_header MiniPlatformHeader = {};
            MiniPlatformHeader.Id = SVGC->Id;
            MiniPlatformHeader.Origin = Center / MinimapReduxFactor;
            MiniPlatformHeader.Type = Entity_MiniPlatform;
            MiniPlatformHeader.Height = PlatformHeight;
            MiniPlatformHeader.State = EntityState_Visible | EntityState_Active;
            MiniPlatform.Header = MiniPlatformHeader;
            MiniPlatform.Static.Header = &MiniPlatformHeader;
            MiniPlatform.Static.Origin = MiniPlatformHeader.Origin;
            MiniPlatform.Static.Shape.ColorFill = Black;
            MiniPlatform.Static.Shape.Type = Shape_Circle;
            MiniPlatform.Static.Shape.Origin = MiniPlatformHeader.Origin;
            MiniPlatform.Static.Shape.Bases = 
            {
                V3f(1.0f, 0.0f, 0.0f), 
                V3f(0.0f, 1.0f, 0.0f), 
                V3f(0.0f, 0.0f, 1.0f)
            };
            
            MiniPlatform.Static.Shape.Circle.Radius = SVGC->Radius / MinimapReduxFactor;
            
            MiniPlatform.Static.Shape.Transform = {};
            MiniPlatform.Static.Shape.Transform.Translation = {0.0f, 0.0f, 0.0f};
            MiniPlatform.Static.Shape.Transform.Rotation = {0.0f, 0.0f, 0.0f};
            MiniPlatform.Static.Shape.Transform.Scale = {1, 1, 1};
            
            MinimapEntities[MinimapEntitiesCount++] = MiniPlatform;
        }
        
        else
        {
            ASSERT(false);
        }
        
        Game.EntityCount++;
    }
    
    for (uint i = 0; i < MinimapEntitiesCount; i++)
    {
        if (MinimapEntities[i].Header.Type == Entity_MiniPlatform ||
            MinimapEntities[i].Header.Type == Entity_CenterLimit)
        {
            MinimapEntities[i].Static.Origin.x -= -0.75f+CircleCenter.y/MinimapReduxFactor;
            float dCenter = MinimapEntities[i].Static.Origin.y - CircleCenter.y/MinimapReduxFactor;
            MinimapEntities[i].Static.Origin.y = 2 * CenterRadius/MinimapReduxFactor - dCenter;
            
            MinimapEntities[i].Header.Origin = MinimapEntities[i].Static.Origin;
            MinimapEntities[i].Static.Shape.Origin = MinimapEntities[i].Header.Origin;
            
            CalcShapePoints(&MinimapEntities[i].Static.Shape, 0.0f);
        }
    }
    
    uint Count = Game.EntityCount;
    for (entity *Entity = Game.Entities; Count--; Entity++)
    {
        
        if (Entity->Header.Type == Entity_Player)
        {
            Entity->Player.Position.x = Entity->Player.Position.x - CircleCenter.x;
            Entity->Player.Position.y = 2 * CenterRadius - (Entity->Player.Position.y - CircleCenter.y);
            Entity->Header.Origin = Entity->Player.Position;
        }
        
        else if (Entity->Header.Type == Entity_Background)
        {
            Entity->Background.Shape.Origin.x = Entity->Background.Shape.Origin.x - CircleCenter.x;
            Entity->Background.Shape.Origin.y = 2 * CenterRadius - (Entity->Background.Shape.Origin.y - CircleCenter.y);
            Entity->Background.Origin.x = Entity->Background.Shape.Origin.x;
            Entity->Background.Origin.y = Entity->Background.Shape.Origin.y;
        }
        
        else if (Entity->Header.Type == Entity_Enemy)
        {
            Entity->Enemy.Position.x = Entity->Enemy.Position.x - CircleCenter.x;
            Entity->Enemy.Position.y = 2 * CenterRadius - (Entity->Enemy.Position.y - CircleCenter.y);
            Entity->Header.Origin = Entity->Enemy.Position;
        }
        
        else if (Entity->Header.Type == Entity_Static ||
                 Entity->Header.Type == Entity_ShortObstacle ||
                 Entity->Header.Type == Entity_CenterLimit ||
                 Entity->Header.Type == Entity_Wall)
        {
            Entity->Static.Origin.x = Entity->Static.Origin.x - CircleCenter.x;
            Entity->Static.Origin.y = 2 * CenterRadius - (Entity->Static.Origin.y - CircleCenter.y);
            Entity->Header.Origin = Entity->Static.Origin;
        }
    }
}

inline int
TinyXMLParseSVG(TiXmlDocument SVGFile, svg_header *Header, svg_circle *SVGCircles)
{
    TiXmlHandle DocHandle(&SVGFile);
    int CirclesCount = 0;
    
    TiXmlElement *SVGInfo = DocHandle.FirstChildElement().Element();
    
    if (strncmp(SVGInfo->Value(), "svg", strlen("svg")) == 0)
    {
        strncpy(Header->Xmlns, SVGInfo->Attribute("xmlns"), 128);
        strncpy(Header->Version, SVGInfo->Attribute("version"), 16);
        
        TiXmlElement *SVGContents = DocHandle.FirstChildElement().FirstChildElement().Element();
        svg_circle *Circle = SVGCircles;
        
        for (SVGContents; 
             SVGContents; 
             SVGContents = SVGContents->NextSiblingElement())
        {
            if (strncmp(SVGContents->Value(), "circle", sizeof("circle")) == 0)
            {
                SVGContents->QueryFloatAttribute("cx", &Circle->CenterX);
                SVGContents->QueryFloatAttribute("cy", &Circle->CenterY);
                SVGContents->QueryFloatAttribute("r", &Circle->Radius);
                SVGContents->QueryIntAttribute("id", &Circle->Id);
                
                strncpy(Circle->Fill, SVGContents->Attribute("fill"), 16);
                
                Circle++;
                CirclesCount++;
            }
        }
        Circle = 0;
    }
    
    return CirclesCount;
}

inline void
TinyXMLParseConfigFile(TiXmlDocument Config)
{
    TiXmlHandle DocHandle(&Config);
    TiXmlElement *DocElement = DocHandle.FirstChildElement().Element();
    
    if(strncmp(DocElement->Value(), "aplicacao", strlen("aplicacao")) == 0)
    {
        TiXmlElement *Content = DocHandle.FirstChildElement().FirstChildElement().Element();
        
        for (Content; 
             Content; 
             Content = Content->NextSiblingElement())
        {
            
            if (strncmp(Content->Value(), "arquivoDaArena", strlen("arquivoDaArena")) == 0)
            {
                strncpy(Game.Arena.AppName, Content->Attribute("nome"), 128);
                strncpy(Game.Arena.FilePath, Content->Attribute("caminho"), 128);
                strcat(Game.Arena.FilePath, Game.Arena.AppName);
                
                char FileType[32];
                strncpy(FileType, Content->Attribute("tipo"), 32);
                
                if (strncmp(FileType, "svg", strlen("svg")) == 0)
                {
                    Game.Arena.Type = FileType_Svg;
                    strcat(Game.Arena.FilePath, ".svg");
                }
            }
            
            else if (strncmp(Content->Value(), "jogador", strlen("jogador")) == 0)
            {
                Content->QueryFloatAttribute("velTiro", &Game.ShotVelocity);
                Content->QueryFloatAttribute("vel", &Game.PlayerVelocity);
                
                Game.ShotVelocity *= 1000.0f;
                Game.PlayerVelocity *= 1000.0f;
            }
            
            else if (strncmp(Content->Value(), "inimigo", strlen("inimigo")) == 0)
            {
                Content->QueryFloatAttribute("freqTiro", &Game.EnemyShotFrequency);
                Content->QueryFloatAttribute("velTiro", &Game.EnemyShotVelocity);
                Content->QueryFloatAttribute("vel", &Game.EnemyVelocity);
                
                Game.EnemyShotVelocity *= 1000.0f;
                Game.EnemyVelocity *= 1000.0f;
                Game.EnemyCountToShoot = (int)(Game.EnemyShotFrequency * Game.FramesPerSecond);
            }
            
            else if (strncmp(Content->Value(), "obstaculo", strlen("obstaculo")) == 0)
            {
                Content->QueryFloatAttribute("altura", &Game.ObstaclesHeight);
            }
            
            else
            {
                ASSERT(0);
            }
        }
        
    }
    
    if (Game.Arena.FilePath)
    {
        TiXmlDocument ArenaSVG(Game.Arena.FilePath);
        
        if (ArenaSVG.LoadFile())
        {
            if (Game.Arena.Type == FileType_Svg)
            {
                svg_circle *SVGCircle = (svg_circle *)malloc(sizeof(svg_circle) * MAX_ENTITIES);
                int CirclesCount = 
                    TinyXMLParseSVG(ArenaSVG, &Game.Arena.SVGFile, SVGCircle);
                
                // Depois, colocar isso numa função com uma semântica melhor
                SVGDefinitionsToEntities(SVGCircle, CirclesCount);
                
                free(SVGCircle);
            }
        }
    }
}

// A origem da janela de contexto OpenGL é no canto esquerdo superior
// Como as coordenadas do mouse são em relação ao ao canto esquerdo inferior, é conveniente
// adotar apenas uma referência para a origem
inline v3f GetCanonicalCoordinates(float X, float Y, float Z)
{
    return (V3f(X, (WindowHeight+EyeVisionHeight - Y), 0.0));
}

inline void DrawGame(arena Arena)
{
    DrawShape(&Arena.Background->Shape, Arena.Background->Shape.Origin, &FloorTexture);
    //UpdateAndDrawEntity(Arena.CenterLimit);
    
    uint Count = Game.EntityCount;
    for (entity *Entity = Game.Entities; Count--; Entity++)
    {
        if (Entity->Header.Type == Entity_ShortObstacle)
        {
            UpdateAndDrawEntity(Entity);
        }
        
        if (Entity->Header.Type == Entity_Enemy)
        {
            UpdateAndDrawEntity(Entity);
        }
        
        if (Entity->Header.Type == Entity_Wall)
        {
            UpdateAndDrawEntity(Entity);
        }
        
        if (Entity->Header.Type == Entity_CenterLimit)
        {
            UpdateAndDrawEntity(Entity);
        }
    }
}

void PrintScore(GLfloat x, GLfloat y, const char *Message)
{
    char *StringToPrint;
    char ScoreString[16];
    
    sprintf(ScoreString, "%s", Message);
    glRasterPos2f(x, y);
    //glutBitmapLength(font,(unsigned char*)str);
    StringToPrint = ScoreString;
    //Print each of the other Char at time
    for(;*StringToPrint; StringToPrint++)
    {
        glutBitmapCharacter(GLUT_BITMAP_9_BY_15, *StringToPrint);
    }
    
}

uint EnemiesWithStateCount(entity_state State)
{
    uint Count = 0;
    
    for (int i = 0; i < Game.EntityCount; i++)
    {
        if (Game.Entities[i].Header.Type == Entity_Enemy &&
            Game.Entities[i].Header.State & State)
        {
            Count++;
        }
    }
    
    return Count;
}

void RasterChars(GLfloat x, GLfloat y, GLfloat z, const char * text, double r, double g, double b)
{
    //Push to recover original attributes
    glPushAttrib(GL_ENABLE_BIT);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    //Draw text in the x, y, z position
    glColor3f(r,g,b);
    glRasterPos3f(x, y, z);
    const char* tmpStr;
    tmpStr = text;
    while( *tmpStr ){
        glutBitmapCharacter(GLUT_BITMAP_9_BY_15, *tmpStr);
        tmpStr++;
    }
    glPopAttrib();
}

void PrintText(GLfloat x, GLfloat y, const char * text, double r, double g, double b)
{
    //Draw text considering a 2D space (disable all 3d features)
    glMatrixMode(GL_PROJECTION);
    //Push to recover original PROJECTION MATRIX
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, 1, 0, 1, -2, 2);
    RasterChars(x, y, 0, text, r, g, b);
    
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

inline bool CameraLerp(v3f *Source, v3f *Dest)
{
    v3f CamTransitionVelocity = {3.0, 3.0, 1.5};
    
    if (Source->x < 0.9*Dest->x)
    {
        Source->x += CamTransitionVelocity.x;
    }
    else if (Source->x > 1.1*Dest->x)
    {
        Source->x -= CamTransitionVelocity.x;
    }
    
    if (Source->y < 0.9*Dest->y)
    {
        Source->y += CamTransitionVelocity.y;
    }
    else if (Source->y > 1.1*Dest->y)
    {
        Source->y -= CamTransitionVelocity.y;
    }
    
    if (Source->z < 0.9*Dest->z)
    {
        Source->z += CamTransitionVelocity.z;
    }
    else if (Source->z > 1.1*Dest->z)
    {
        Source->z -= CamTransitionVelocity.z;
    }
    
    if ((Source->x >= 0.9*Dest->x && Source->x <= 1.1*Dest->x) &&
        (Source->y >= 0.9*Dest->y && Source->y <= 1.1*Dest->y) &&
        (Source->z >= 0.9*Dest->z && Source->z <= 1.1*Dest->z))
    {
        return false;
    }
    
    return true;
}

void DisplayEyeCamera()
{
    v3f PlayerP = Game.Player->Player.Position;
    bases PlayerBases = Game.Player->Player.Bases;
    float EyeHeight = 90.0;
    
    v3f CameraP = Game.Camera.P;
    v3f CameraUp = Game.Camera.Up;
    
    float CamHorizonDist = 200.0;
    
    v3f PointingGun;
    PointingGun.x = Game.Input.Mouse.Position.x-WindowWidth*0.5f;
    PointingGun.y = 0;
    PointingGun.z = Game.Input.Mouse.Position.y-WindowHeight*0.5f;
    
    CameraUp.x = 0.0;
    CameraUp.y = 0.0;
    CameraUp.z = 1.0;
    
    glViewport (0, WindowHeight, WindowWidth, EyeVisionHeight);
    gluPerspective(60, (float)WindowWidth/(float)(EyeVisionHeight), 1, 2000);
    glLoadIdentity();
    
    float FacingAngle = RadsToDegrees(Game.Player->Player.Transform.Rotation.z);
    float FirstPersonCamOffset = 20.0f;
    
    CameraP.x = PlayerP.x;
    CameraP.y = PlayerP.y;
    CameraP.z = EyeHeight + PlayerP.z; 
    
    glRotatef(-FacingAngle , 0, 1, 0);
    glRotatef(-RadsToDegrees(GunTurnY), 1, 0, 0);
    glTranslatef(-FirstPersonCamOffset*PlayerBases.yAxis.x, 
                 -FirstPersonCamOffset*PlayerBases.yAxis.y, 
                 -FirstPersonCamOffset*PlayerBases.yAxis.z);
    glTranslatef(-CameraP.x, -CameraP.y, -CameraP.z);
    
    DrawSkyBox(PlayerP);
    DrawGame(Game.Arena);
    
    UpdateAndDrawEntity(Game.Player);
    
    for (int i = 0; i < GlobalBulletsCount; i++)
    {
        if (SaveGlobalBullets[i].Header.State & EntityState_Active)
        {
            UpdateAndDrawEntity(&SaveGlobalBullets[i]);
        }
    }
}

inline void DisplayMinimap()
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    int NearPlane = -1;
    int FarPlane = 1;
    glOrtho(0, 1, 0, 1, NearPlane, FarPlane);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    glPushAttrib(GL_ENABLE_BIT);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    
    glColor3f(1,0,0);
    
    for (uint i = 0; i < MinimapEntitiesCount; i++)
    {
        UpdateAndDrawEntity(&MinimapEntities[i]);
    }
    glPopAttrib();
}

#include"game_update_and_draw.cpp"

void Display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    v3f PlayerP = Game.Player->Player.Position;
    bases PlayerBases = Game.Player->Player.Bases;
    
#if 1
    glMatrixMode (GL_PROJECTION);
    glLoadIdentity();
    
    if (WindowWidth <= WindowHeight)
    {
        gluPerspective(45, (float)WindowHeight/(float)(WindowWidth), 3, 2000);
    }
    
    else
    {
        gluPerspective(45, (float)WindowWidth/(float)(WindowHeight), 3, 2000);
    }
    
    glMatrixMode(GL_MODELVIEW);
    
    glPushMatrix();
    glTranslatef(403.38983f, 701.69495f, 0.0f);
    glRotatef(90, 1, 0, 0);
    glScalef(5.0f, 5.0f, 5.0f);
    CrateMesh.draw();
    glPopMatrix();
    
    // NOTA: Céu
    DrawSkyBox(PlayerP);
    
    DrawGame(Game.Arena);
    UpdateAndDrawEntity(Game.Player);
    
    for (int i = 0; i < GlobalBulletsCount; i++)
    {
        if (SaveGlobalBullets[i].Header.State & EntityState_Active)
        {
            UpdateAndDrawEntity(&SaveGlobalBullets[i]);
        }
    }
#endif
    
    DisplayEyeCamera();
    UpdateLight(PlayerP, PlayerBases);
    
    glViewport(0, 0, WindowWidth, WindowHeight);
    DisplayMinimap();
    
    uint EnemiesDeadCount = Game.EnemyCount - EnemiesWithStateCount(EntityState_Active);
    char StringStatusEnemies[64];
    snprintf(StringStatusEnemies, 64, "Mortos: %u", EnemiesDeadCount);
    PrintText(0.80, 0.95, StringStatusEnemies, 1, 0, 0);
    
    if ((EnemiesDeadCount == Game.EnemyCount) && !GameEndedWithVictory)
    {
        PrintText(0.45, 0.5, "Ganhou!", 0, 1, 0);
        GameEndedWithFailure = true;
    }
    
    if (!(Game.Player->Header.State & EntityState_Active) && !GameEndedWithFailure)
    {
        PrintText(0.45, 0.5, "Perdeu!", 1, 0, 0);
        GameEndedWithVictory = true;
    }
    
#if DEBUG_PLAYER_LINE
    OpenGLDrawLine(Game.Player->Player.Position, 
                   Game.Player->Player.Position + 100 * Game.Player->Player.Bases.yAxis, 
                   Colors[Red]);
    
    OpenGLDrawLine(Game.Player->Player.Position, 
                   Game.Player->Player.Position + 
                   100 *Game.Player->Player.Bases.xAxis, Colors[Brown]);
    
    v3f Origin = Game.Player->Player.Body.RightArm.Origin;
    m3x3 BaseMatrix = Game.Player->Player.Bases.BaseMatrix;
    v3f ArmOrigin = CoordinateChange(BaseMatrix, Origin);
    v3f ArmOffset01 = CoordinateChange(Game.Player->Player.Bases.BaseMatrix, 
                                       V3f(0, Game.Player->Player.Body.RightArm.Rectangle.Height, 0));
    v3f ArmOffset = CoordinateChange(Game.Player->Player.Body.RightArm.Bases.BaseMatrix, 
                                     ArmOffset01);
    OpenGLDrawLine(Game.Player->Player.Position + ArmOrigin + ArmOffset, 
                   Game.Input.Mouse.Position, Colors[Lime]);
#endif
    
    glutSwapBuffers();
}

void ProcessMotion(int X, int Y)
{
    Game.Input.Mouse.IsHovering = true;
    Game.Input.Mouse.Position = GetCanonicalCoordinates(X, Y, 0.0);
    
    if (Game.Input.Mouse.ButtonState[GLUT_RIGHT_BUTTON] == GLUT_DOWN &&
        Game.Camera.Type == Camera_ThirdPerson && GlobalAllowPerspCameraToMove)
    {
        DistTraveledLastFrame.x = X - GlobalLastXCoordinate;
        DistTraveledLastFrame.y = (WindowHeight+EyeVisionHeight - Y) - GlobalLastYCoordinate;
        CameraPerspMoved = true;
    }
}

void ProcessPassiveMotion(int X, int Y)
{
    Game.Input.Mouse.IsHovering = true;
    Game.Input.Mouse.Position = GetCanonicalCoordinates(X, Y, 0.0);
}

void ProcessMouseInput(int Button, int State, int X, int Y)
{
    Game.Input.Mouse.ButtonState[Button] = State;
    
    if (Button == GLUT_LEFT_BUTTON)
    {
        if (State == GLUT_DOWN)
        {
            ShotFired = true;
        }
        
        else if (State == GLUT_UP)
        {
            ShotFired = false;
        }
    }
    
    else if (Button == GLUT_RIGHT_BUTTON)
    {
        if (State == GLUT_DOWN)
        {
            GlobalAllowPerspCameraToMove = true;
            GlobalLastXCoordinate = X;
            GlobalLastYCoordinate = WindowWidth+EyeVisionHeight - Y;
        }
        
        else if (State == GLUT_UP)
        {
            GlobalAllowPerspCameraToMove = false;
        }
    }
    
    else
    {
        Game.Input.Mouse.ButtonState[Button] = -1;
    }
    
    Game.Input.Mouse.IsHovering = false;
    Game.Input.Mouse.Position = GetCanonicalCoordinates(X, Y, 0);
}

void ProcessKeyUp(unsigned char Key, int X, int Y)
{
    Game.Input.Keyboard[Key] = false;
}

void ProcessKeyboard(unsigned char Key, int X, int Y)
{
    Game.Input.Keyboard[Key] = true;
    
    if (Key == 'n')
    {
        if (glIsEnabled(GL_LIGHT0))
        {
            glDisable(GL_LIGHT0);
            SkyTexture.Material.Emission = {0.1f, 0.1f, 0.1f, 1};
        }
        else
        {
            SkyTexture.Material.Emission = {0.9f, 0.9f, 0.9f, 1};
            glEnable(GL_LIGHT0);
        }
    }
    
    if (Key == 'f')
    {
        if (glIsEnabled(GL_LIGHT1))
        {
            glDisable(GL_LIGHT1);
        }
        else
        {
            glEnable(GL_LIGHT1);
        }
    }
}

inline 
v3f UpdatePosition(v3f ddPos, v3f dPos, float deltaTime)
{
    v3f NewPos = (ddPos * Square(deltaTime) * 0.5 + dPos * deltaTime);
    
    return NewPos;
}

void PlayerJump(entity_player *Player)
{
    Player->Position.z = Player->Position.z + Player->Velocity.z * Game.Timing.deltaTime;
    Player->Header->Origin.z = Player->Position.z;
    
    float HeightToJump = fabs(JumpHeight + HeightWhenPressedJumpButton - Player->Position.z);
    
    if (Player->Dynamics.Ascending)
    {
        float GH = GravityAccelerationScale * GRAVITY_ACCELERATION * HeightToJump;
        Player->Velocity.z = sqrt(2 * GH);
    }
    else
    {
        float GH = GravityAccelerationScale * GRAVITY_ACCELERATION * HeightToJump;
        Player->Velocity.z = -1 * sqrt(2 * GH);
    }
    
    float OffsetJumpHeight = 5.0;
    if (Player->Velocity.z < OffsetJumpHeight) 
    {
        Player->Dynamics.Ascending = false;
    }
    
    if (Player->Position.z <= Game.Arena.Background->Header->Origin.z) 
    {
        Player->Velocity.z = 0.0f;
        
        Player->Position.z = Game.Arena.Background->Header->Origin.z;
        Player->Header->Origin.z = Player->Position.z;
        Player->Jumping = false;
        return;
    }
}

void MovePlayer(entity_player *Player)
{
    
    v3f NewRelPos = {}; 
    NewRelPos.x = Player->Velocity.x * Game.Timing.deltaTime;
    NewRelPos.y = Player->Velocity.y * Game.Timing.deltaTime;
    NewRelPos.z = 0.0f;
    
    v3f NewdeltaPos = NewRelPos;
    // NOTA: Loop de colisão é útil para resolver casos mais complexos, onde existe
    // mistura de geometrias de colisão
    for (int i = 0; i < 4; i++)
    {
        Player->Dynamics.Collision = false;
        
        int Count = Game.EntityCount;
        for (entity *Entity = Game.Entities; Count--; Entity++)
        {
            switch(Entity->Header.Type)
            {
                case Entity_Enemy:
                {
                    if (Entity->Header.State & EntityState_Active)
                    {
                        float ToWallDist = 20.0;
                        v3f d = Player->Position + NewdeltaPos - Entity->Enemy.Position;
                        d.z = 0;
                        float dCenter = VectorSize(d);
                        sphere Sphere = Player->Head->Content.Sphere; 
                        float dRadius = Sphere.Radius + ToWallDist + Entity->Enemy.Head->Content.Sphere.Radius;
                        
                        if (dCenter <= dRadius)
                        {
                            v3f Normal = d / dCenter;
                            Player->Velocity = Player->Velocity - 1 * DotProduct(Player->Velocity, Normal) * Normal;
                            // HACK: Multiplicação por 0.05f é apenas um hack para evitar deslizamento exagerado
                            NewdeltaPos += 0.05f * Player->Velocity * Game.Timing.deltaTime + 1 * Normal;
                            Player->Dynamics.Collision = true;
                        }
                    }
                } break;
                
                case Entity_ShortObstacle:
                {
                    if ((Entity->Header.State & EntityState_Active) &&
                        !Player->Dynamics.PlatformAllow &&
                        Player->Position.z <= Entity->Header.Height)
                    {
                        float ToWallDist = 1.0;
                        v3f d = Player->Position + NewdeltaPos - Entity->Static.Origin;
                        d.z = 0;
                        float dCenter = VectorSize(d);
                        sphere Sphere = Player->Head->Content.Sphere; 
                        float dRadius = Sphere.Radius + ToWallDist + Entity->Static.Shape.Circle.Radius;
                        
                        if (dCenter <= dRadius)
                        {
                            v3f Normal = d / dCenter;
                            Player->Velocity = Player->Velocity - 1 * DotProduct(Player->Velocity, Normal) * Normal;
                            // TODO: Multiplicação por 0.05f é apenas um hack para evitar deslizamento exagerado
                            NewdeltaPos += 0.05f * Player->Velocity * Game.Timing.deltaTime + 1 * Normal;
                            Player->Dynamics.Collision = true;
                        }
                    }
                } break;
                
                case Entity_CenterLimit:
                {
                    if (Entity->Header.State & EntityState_Active)
                    {
                        float ToWallDist = 10.0;
                        v3f d = Player->Position + NewdeltaPos - Entity->Static.Origin;
                        d.z = 0;
                        float dCenter = VectorSize(d);
                        sphere Sphere = Player->Head->Content.Sphere; 
                        float dRadius = Sphere.Radius + ToWallDist + Entity->Static.Shape.Circle.Radius;
                        
                        if (dCenter <= dRadius)
                        {
                            v3f Normal = d / dCenter;
                            Player->Velocity = Player->Velocity - 1 * DotProduct(Player->Velocity, Normal) * Normal;
                            // TODO: Multiplicação por 0.05f é apenas um hack para evitar deslizamento exagerado
                            NewdeltaPos += 0.05f * Player->Velocity * Game.Timing.deltaTime + 1.0 * Normal;
                            Player->Dynamics.Collision = true;
                        }
                    }
                } break;
                
                case Entity_Background:
                {
                    float ToWallDist = 10.0;
                    v3f d = Entity->Background.Origin - NewRelPos - Player->Position;
                    d.z = 0;
                    float dCenter = VectorSize(d) + Player->Head->Content.Sphere.Radius;
                    
                    if (dCenter + ToWallDist >= (Entity->Background.Shape.Rectangle.Width/2))
                    {
                        v3f Normal = Normalize(d);
                        float Dot = DotProduct(Player->Velocity, Normal);
                        Player->Velocity = Player->Velocity - 1 * Dot * Normal;
                        NewdeltaPos += 0.05f * Player->Velocity * Game.Timing.deltaTime + 1 * Normal;
                        
                        Player->Dynamics.Collision = true;
                    }
                } break;
                
            } 
        }
        
        if (Player->Dynamics.Collision)
        {
            if (i >= 3)
            {
                NewRelPos.x = 0.0f;
                NewRelPos.y = 0.0f;
                NewRelPos.z = 0.0f;
            }
            
            else
            {
                NewRelPos = NewdeltaPos;
            }
        }
        
        else
        {
            // Saia do loop do colisão
            break;
        }
        
        NewdeltaPos = NewRelPos + Player->Velocity * Game.Timing.deltaTime;
    }
    
    if (VectorSize(NewRelPos) > 0.0f)
    {
        Player->Position.x += NewRelPos.x;
        Player->Position.y += NewRelPos.y;
        Player->Header->Origin = Player->Position;
    }
}

void PlayerCollisionZ(arena Arena, entity_player *Player)
{
    bool PlatformAllowFlag = false;
    
    uint Count = Game.EntityCount;
    for (entity *JumpingPlatform = Game.Entities; 
         Count--; 
         JumpingPlatform++)
    {
        if (JumpingPlatform->Header.Type == Entity_ShortObstacle && 
            (JumpingPlatform->Header.Height > 0.0f))
        {
            v3f d = Player->Position - JumpingPlatform->Static.Origin;
            
            float MinkowskiRadius = JumpingPlatform->Static.Shape.Circle.Radius + Player->Head->Content.Sphere.Radius;
            if (VectorSize(d) <= MinkowskiRadius)
            {
                PlatformAllowFlag = true;
                
                if (Player->Velocity.z < 0.0f && 
                    Player->Position.z <= JumpingPlatform->Header.Height)
                {
                    Player->Velocity.z = 0.0f;
                    Player->Position.z = JumpingPlatform->Header.Height;
                    Player->Jumping = false;
                }
            }
        }
    }
    
    if (PlatformAllowFlag) Player->Dynamics.PlatformAllow = true;
    else 
    {
        if (Player->Dynamics.PlatformAllow && 
            Player->Velocity.z == 0.0f)
        {
            Player->Dynamics.FreeFalling = true;
        }
        Player->Dynamics.PlatformAllow = false;
    }
    
    if (Player->Dynamics.FreeFalling)
    {
        bool SaveAscending = Player->Dynamics.Ascending;
        Player->Dynamics.Ascending = false;
        PlayerJump(Player);
        if (Player->Position.z <= 0.0f)
        {
            Player->Position.z = 0.0f;
            Player->Velocity.z = 0.0f;
            Player->Dynamics.FreeFalling = false;
        }
        Player->Dynamics.Ascending = SaveAscending;
    }
}

inline void ProcessInput(input Input, entity_player *Player)
{
    if (Input.Keyboard['a'] || Input.Keyboard['A'])
    {
        Player->Transform.Rotation.z += DegreesToRads(Game.Player->Player.SpinMagnitude);
        RotateOrthonormalBases(&Player->Bases, Player->Transform.Rotation.z);
    }
    
    if (Input.Keyboard['s'] || Input.Keyboard['S'])
    {
        Player->Velocity.x = -Game.PlayerVelocity * Player->Bases.yAxis.x;
        Player->Velocity.y = -Game.PlayerVelocity * Player->Bases.yAxis.y;
        Player->CyclesToChangeFootDirection -= (int)(Game.PlayerVelocity/100);
    }
    
    if (Input.Keyboard['w'] || Input.Keyboard['W'])
    {
        Player->Velocity.x = Game.PlayerVelocity * Player->Bases.yAxis.x;
        Player->Velocity.y = Game.PlayerVelocity * Player->Bases.yAxis.y;
        Player->CyclesToChangeFootDirection -= (int)(Game.PlayerVelocity/100);
    }
    
    if (Input.Keyboard['d'] || Input.Keyboard['D'])
    {
        Player->Transform.Rotation.z -= DegreesToRads(Game.Player->Player.SpinMagnitude);
        RotateOrthonormalBases(&Player->Bases, Player->Transform.Rotation.z);
    }
    
    if (Input.Keyboard['p'] || Input.Keyboard['P'])
    {
        if (!Player->Jumping && Player->Velocity.z == 0.0f) 
        {
            Player->Dynamics.Ascending = true;
            Player->Jumping = true;
            
            // A velocidade é determinada usando a equação da conservação de energia
            // para corpos em queda live
            float GH = 
                GravityAccelerationScale * GRAVITY_ACCELERATION * JumpHeight;
            Player->Velocity.z = sqrt(2 * GH);
            
            HeightWhenPressedJumpButton = Player->Position.z;
        }
    }
    
    if (Input.Keyboard['1'])
    {
        ToFirstPersonCamTransition = true;
        Game.Camera.Type = Camera_FirstPersonGun;
    }
    else if (Input.Keyboard['2'])
    {
        ToThirdPersonCamTransition = true;
        Game.Camera.Type = Camera_ThirdPerson;
    }
    
    if (ShotFired)
    {
        shape_tree RightArm = *Player->RightArm;
        v3f BulletRot = {Player->Transform.Rotation.x, Player->Transform.Rotation.y, Player->Transform.Rotation.z+GunTurnX};
        CreateBulletEntity(GlobalBullets, Entity_Player, 100, Player->Position.z + Player->ArmHeight, Player->ShotVelocity,
                           Player->Bases, BulletRot, &RightArm,
                           Player->Position);
        
        if ((GlobalBullets-SaveGlobalBullets) < 99)
        {
            GlobalBullets++;
        }
        else
        {
            GlobalBullets = SaveGlobalBullets;
        }
        
        ShotFired = false;
    }
    
    if (Player->CyclesToChangeFootDirection <= 0 && !Player->Jumping)
    {
        Player->CyclesToChangeFootDirection = 30;
        Player->RightLeg->Header.Origin.y *= -1;
        Player->LeftLeg->Header.Origin.y *= -1;
        
        //Player->Body.RightLeg[1].Origin.y *= -1;
        //Player->Body.LeftLeg[1].Origin.y *= -1;
    }
}

void UpdateGame(int Value)
{
    ASSERT(Game.Player);
    ProcessInput(Game.Input, &Game.Player->Player);
    
    if (Game.Player->Header.State & EntityState_Active)
    {
        MovePlayer(&Game.Player->Player);
        UpdateMinimap();
    }
    
    if (Game.Player->Player.Jumping)
    {
        PlayerJump(&Game.Player->Player);
    }
    PlayerCollisionZ(Game.Arena, &Game.Player->Player);
    
    CameraUpdate(&Game.Camera);
    
    glutPostRedisplay();
    glutTimerFunc(Game.Timing.deltaTimeMs, UpdateGame, 0);
}

void Init()
{
    float Size = 1.0f;
    
    glClearColor(1, 1, 1, 0);
    glShadeModel(GL_SMOOTH);
    glFrontFace(GL_CCW);
    
    // Precisa mesmo disso??
    //glEnable(GLUT_MULTISAMPLE);
    
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0); 
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_TEXTURE0);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    
    Mesh.loadMesh("model/shotgun.obj");
    Mesh.loadMeshTex(LoadTextureRAW("texture/sg_diffuse.bmp"));
    
    CrateMesh.loadMesh("model/plataforma_blender.obj");
    CrateMesh.loadMeshTex(LoadTextureRAW("texture/Beach_sand_pxr128.bmp"));
    
    float SpecularLight[] = {0.5, 0.5, 0.5, 1.0};
    glLightfv(GL_LIGHT1, GL_SPECULAR, SpecularLight);
    float DiffuseLight[] = {0.1, 0.1, 0.1, 1.0};
    glLightfv(GL_LIGHT1, GL_DIFFUSE, DiffuseLight);
    float AmbientLight[] = {1.0, 1.0, 0.6, 1.0};
    glLightfv(GL_LIGHT1, GL_AMBIENT, AmbientLight);
    glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 20.0);
    glLightf(GL_LIGHT1, GL_SPOT_EXPONENT, 10);
    glLightf(GL_LIGHT1, GL_QUADRATIC_ATTENUATION, 0.0f);
    
    float AmbientLight0[] = {0.5, .5, 0.5, 1.0};
    glLightfv(GL_LIGHT0, GL_AMBIENT, AmbientLight0);
    
    FloorTexture.Id = LoadTextureRAW("texture/Beach_sand_pxr128.bmp");
    BulletTexture.Id = LoadTextureRAW("texture/sun1.bmp");
    FloorNormalTexture.Id = LoadTextureRAW("texture/Beach_sand_pxr128_normal.bmp");
    WallTexture.Id = LoadTextureRAW("texture/Stucco_pxr128.bmp");
    HeadTexture.Id = LoadTextureRAW("texture/head.bmp");
    SkyTexture.Id = LoadTextureRAW("texture/posy.bmp");
    
    StandardMaterial.Emission = {0.05, 0.05, 0.05, 1};
    StandardMaterial.ColorA = {0.4, 0.4, 0.4, 1};
    StandardMaterial.ColorD = {1.0, 1.0, 1.0, 1};
    StandardMaterial.Specular = {0.9, 0.9, 0.9, 1};
    StandardMaterial.Shininess = {80.0};
    
    FloorTexture.Material = StandardMaterial;
    BulletTexture.Material = StandardMaterial;
    WallTexture.Material = StandardMaterial;
    HeadTexture.Material = StandardMaterial;
    
    SkyTexture.Material = StandardMaterial;
    SkyTexture.Material.Emission = {1.0f, 1.0f, 1.0f, 1};
    SkyTexture.Material.ColorA = {0.0f, .0f, .0f, 1};
    SkyTexture.Material.ColorD = {0.1f, 0.1f, 0.1f, 1};
    SkyTexture.Material.Specular = {0.1f, 0.1f, 0.1f, 1};
    
    // Inicializacao da camera
    Game.Camera.Up = V3f(0.0, 0.0, 1.0);
    Game.Camera.Sensitivity = 0.6;
    Game.Camera.Type = Camera_FirstPersonGun;
    memset(Game.Input.Mouse.ButtonState, -1, 3);
    
    glViewport (0, 0, WindowWidth, WindowHeight); // a área que você quer desenhar
    
    glMatrixMode (GL_PROJECTION);
    glLoadIdentity();
    
    if (WindowWidth <= WindowHeight)
    {
        gluPerspective(60, (float)WindowHeight/(float)(WindowWidth), 2, 2000);
    }
    
    else
    {
        gluPerspective(60, (float)WindowWidth/(float)(WindowHeight), 2, 2000);
    }
    
    glMatrixMode(GL_MODELVIEW);
}

GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path){
    
    // Create the shaders
    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
    
    // Read the Vertex Shader code from the file
    std::string VertexShaderCode;
    std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
    if(VertexShaderStream.is_open()){
        std::string Line = "";
        while(getline(VertexShaderStream, Line))
            VertexShaderCode += "\n" + Line;
        VertexShaderStream.close();
    }else{
        printf("Impossible to open %s. Are you in the right directory ? Don't forget to read the FAQ !\n", vertex_file_path);
        getchar();
        return 0;
    }
    
    // Read the Fragment Shader code from the file
    std::string FragmentShaderCode;
    std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
    if(FragmentShaderStream.is_open()){
        std::string Line = "";
        while(getline(FragmentShaderStream, Line))
            FragmentShaderCode += "\n" + Line;
        FragmentShaderStream.close();
    }
    
    GLint Result = GL_FALSE;
    int InfoLogLength;
    
    
    // Compile Vertex Shader
    printf("Compiling shader : %s\n", vertex_file_path);
    char const * VertexSourcePointer = VertexShaderCode.c_str();
    glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
    glCompileShader(VertexShaderID);
    
    // Check Vertex Shader
    glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 0 ){
        std::vector<char> VertexShaderErrorMessage(InfoLogLength+1);
        glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
        printf("%s\n", &VertexShaderErrorMessage[0]);
    }
    
    // Compile Fragment Shader
    printf("Compiling shader : %s\n", fragment_file_path);
    char const * FragmentSourcePointer = FragmentShaderCode.c_str();
    glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
    glCompileShader(FragmentShaderID);
    
    // Check Fragment Shader
    glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 0 ){
        std::vector<char> FragmentShaderErrorMessage(InfoLogLength+1);
        glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
        printf("%s\n", &FragmentShaderErrorMessage[0]);
    }
    
    // Link the program
    printf("Linking program\n");
    uint ProgramID = glCreateProgram();
    glAttachShader(ProgramID, VertexShaderID);
    glAttachShader(ProgramID, FragmentShaderID);
    glLinkProgram(ProgramID);
    
    // Check the program
    glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
    glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 0 ){
        std::vector<char> ProgramErrorMessage(InfoLogLength+1);
        glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
        printf("%s\n", &ProgramErrorMessage[0]);
    }
    
    glDetachShader(ProgramID, VertexShaderID);
    glDetachShader(ProgramID, FragmentShaderID);
    
    glDeleteShader(VertexShaderID);
    glDeleteShader(FragmentShaderID);
    
    return ProgramID;
}

bool LoadCubeMapSide(
uint Texture, GLenum side_target, const char* FileName) {
    glBindTexture(GL_TEXTURE_CUBE_MAP, Texture);
    
    int x, y, n;
    int force_channels = 4;
    unsigned char*  ImageData = stbi_load(
        FileName, &x, &y, &n, force_channels);
    if (!ImageData) {
        fprintf(stderr, "ERROR: could not load %s\n", FileName);
        return false;
    }
    // non-power-of-2 dimensions check
    if ((x & (x - 1)) != 0 || (y & (y - 1)) != 0) {
        fprintf(stderr,
                "WARNING: image %s is not power-of-2 dimensions\n",
                FileName);
    }
    
    // copy image data into 'target' side of cube map
    glTexImage2D(
        side_target,
        0,
        GL_RGBA,
        x,
        y,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        ImageData);
    free(ImageData);
    return true;
}

void CreateCubeMap(
const char* front,
const char* back,
const char* top,
const char* bottom,
const char* left,
const char* right,
GLuint* CubeTexture) {
    // generate a cube-map texture to hold all the sides
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, CubeTexture);
    
    // load each image and copy into a side of the cube-map texture
    LoadCubeMapSide(*CubeTexture, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, front);
    LoadCubeMapSide(*CubeTexture, GL_TEXTURE_CUBE_MAP_POSITIVE_Z, back);
    LoadCubeMapSide(*CubeTexture, GL_TEXTURE_CUBE_MAP_POSITIVE_Y, top);
    LoadCubeMapSide(*CubeTexture, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, bottom);
    LoadCubeMapSide(*CubeTexture, GL_TEXTURE_CUBE_MAP_NEGATIVE_X, left);
    LoadCubeMapSide(*CubeTexture, GL_TEXTURE_CUBE_MAP_POSITIVE_X, right);
    
    // format cube map texture
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

int main(int argc, char **argv)
{
    if ((argc > 1))
    {
        char File[64] = {};
        strncpy(File, argv[1], 64);
        strcat(File, "config.xml");
        TiXmlDocument Config(File);
        
        Game.FramesPerSecond = 60;
        Game.Timing.deltaTimeMs = 1000.0/Game.FramesPerSecond;
        Game.Timing.deltaTime = Game.Timing.deltaTimeMs / 1000;
        
        if (Config.LoadFile()) 
        {
            // TODO: Criar um modelo de armazenamento de entidades que tem pouco tempo de vida.
            GlobalBulletsCount = 100;
            GlobalBullets = (entity *)malloc(sizeof(entity) * GlobalBulletsCount);
            SaveGlobalBullets = GlobalBullets;
            Game.Entities = (entity *)malloc(sizeof(entity) * MAX_ENTITIES);
            
            TinyXMLParseConfigFile(Config);
            
            glutInit(&argc, argv);
            glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH | GLUT_MULTISAMPLE);
            
            glutInitWindowSize(WindowWidth, WindowHeight + EyeVisionHeight);
            
            glutCreateWindow("TF - Guilherme Caetano e Marcelo Bringuenti Pedro");
            
            Init();
            glutDisplayFunc(Display);
            glutKeyboardUpFunc(ProcessKeyUp);
            glutKeyboardFunc(ProcessKeyboard);
            glutMouseFunc(ProcessMouseInput);
            glutPassiveMotionFunc(ProcessPassiveMotion);
            glutMotionFunc(ProcessMotion);
            glutTimerFunc(Game.Timing.deltaTimeMs, UpdateGame, 0);
            
#if 0
            glewInit();
            GLuint vbo;
            glGenBuffers(1, &vbo);
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW);
            
            glGenVertexArrays(1, &VAO);
            glBindVertexArray(VAO);
            glEnableVertexAttribArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
            
            glEnableClientState(GL_VERTEX_ARRAY);
            
            CreateCubeMap("texture/negz.jpg", "texture/posz.jpg", "texture/posy.jpg",
                          "texture/negy.jpg", "texture/negx.jpg", "texture/posx.jpg", 
                          &CubeTextureMap);
            
            ProgramId = LoadShaders("codigo/cube_map_shader.glsl", 
                                    "codigo/cube_map_fragment_shader.glsl");
            
            ProjectionMatrixLoc = glGetUniformLocation(ProgramId, "P");
            ModelViewMatrixLoc = glGetUniformLocation(ProgramId, "V");
            CubeTextureMapLoc = glGetUniformLocation(ProgramId, "CubeTexture");
#endif
            glutMainLoop();
            
            free(SaveGlobalBullets);
            free(Game.Entities);
        }
        
        else
        {
            fprintf(stderr, "Arquivo não pode ser aberto. Terminando execução do programa.\n");
        }
    }
    
    else if (argc <= 1)
    {
        fprintf(stderr, "Arquivo de entrada não especificado. Terminando execução do programa.\n");
    }
    
    else
    {
        fprintf(stderr, "Arquivo não pode ser aberto. Terminando execução do programa.\n");
    }
    
    return 0;
}
