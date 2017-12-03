// Guilherme Rodrigues Caetano - 2013100847
// TF Computação Gráfica

#include<stdlib.h>
#include<time.h>
#include<math.h>
#include<GL/glut.h>
#include<GL/glu.h>
#include<GL/gl.h>
#include<tinyxml/tinystr.h>
#include<tinyxml/tinyxml.h>
#include<imageloader/imageloader.h>
#include"algebra.h"
#include"tf.h"
#include"shape_definitions.cpp"
#include"draw.cpp"
#include"entity_definitions.cpp"

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
            Player->Body.RightArm.Transform.Rotation.z = DegreesToRads(-45.0f);
            RotateOrthonormalBases(&Player->Body.RightArm.Bases, DegreesToRads(-45.0f));
        }
        
        else
        {
            Player->Body.RightArm.Transform.Rotation.z = -AngleToTurn;
            RotateOrthonormalBases(&Player->Body.RightArm.Bases, -AngleToTurn);
        }
    }
    
    else
    {
        if (AngleToTurnInDegrees > 45.0f)
        {
            Player->Body.RightArm.Transform.Rotation.z = DegreesToRads(45.0f);
            RotateOrthonormalBases(&Player->Body.RightArm.Bases, DegreesToRads(45.0f));
        }
        
        else
        {
            Player->Body.RightArm.Transform.Rotation.z = AngleToTurn;
            RotateOrthonormalBases(&Player->Body.RightArm.Bases, AngleToTurn);
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

entity_list * Collision(entity *Entity, shape CollisionShape)
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
                    shape Shape = TargetEntity->Player.Body.Head;
                    
                    v3f d = Entity->Header.Origin - TargetEntity->Header.Origin;
                    d.z = 0;
                    float dCenter = VectorSize(d);
                    float dRadius = Shape.Sphere.Radius + CollisionShape.Circle.Radius;
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
                    float dRadius = 1.05 * Shape.Circle.Radius + CollisionShape.Circle.Radius;
                    if (dCenter <= dRadius)
                    {
                        EntityList->Content = TargetEntity;
                        EntityList->Next = new entity_list;
                        EntityList = EntityList->Next;
                    }
                    
                } break;
                
                case Entity_Enemy:
                {
                    shape Shape = TargetEntity->Enemy.Body.Head;
                    
                    v3f d = Entity->Header.Origin - TargetEntity->Enemy.Position;
                    d.z = 0;
                    float dCenter = VectorSize(d);
                    float dRadius = Shape.Sphere.Radius + CollisionShape.Circle.Radius;
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
                    
                    float CircleRadius = CollisionShape.Circle.Radius;
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
void MoveEntity(entity *Entity, game_dynamics *Dynamics, v3f Velocity, shape Shape)
{
    v3f NewRelP = {};
    NewRelP.x = Velocity.x * Game.Timing.deltaTime;
    NewRelP.y = Velocity.y * Game.Timing.deltaTime;
    NewRelP.z = 0;
    
    v3f NewdeltaP = NewRelP;
    uint MaxLoops = 4;
    
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
                        v3f d = Entity->Header.Origin + NewdeltaP - Entities->Content->Header.Origin;
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
                            v3f d = Entity->Header.Origin + NewdeltaP - Entities->Content->Header.Origin;
                            float dCenter = VectorSize(d);
                            v3f Normal = d / dCenter;
                            Velocity = Velocity - 1 * DotProduct(Velocity, Normal) * Normal;
                            NewdeltaP += 0.05f * Velocity * Game.Timing.deltaTime + Normal;
                            Dynamics->Collision = true;
                            
#if 1
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
#endif
                        }
                    } break;
                    
                    case Entity_Background:
                    {
                        v3f d = Entities->Content->Background.Origin - NewRelP - Entity->Header.Origin;
                        d.z = 0;
                        float dCenter = VectorSize(d) + Shape.Circle.Radius;
                        if (dCenter >= Entities->Content->Background.Shape.Circle.Radius)
                        {
                            v3f Normal = Normalize(d);
                            Velocity = Velocity - 1 * DotProduct(Velocity, Normal) * Normal;
                            NewdeltaP += 0.05f * Velocity * Game.Timing.deltaTime + Normal;
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
    
#if 0
    printf("VX: %.6f\n", Entity->Enemy.Velocity.x);
    printf("VY: %.6f\n\n", Entity->Enemy.Velocity.y);
#endif
    
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
            
            float MinkowskiRadius = JumpingPlatform->Static.Shape.Circle.Radius + Enemy->Body.Head.Sphere.Radius;
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

void UpdateAndDrawEntity(entity *Entity)
{
    switch(Entity->Header.Type)
    {
        case Entity_Player:
        {
            if (Entity->Header.State & EntityState_Active)
            {
                
                entity_player *Player = &Entity->Player;
#if 0
                v3f RightArmOrigin = Player->Body.RightArm.Origin;
                v3f ArmOrigin = CoordinateChange(Player->Bases.BaseMatrix, RightArmOrigin);
                Player->RightArmDir = Game.Input.Mouse.Position - (Player->Position + ArmOrigin);
                MovePlayerArm(Player, Player->RightArmDir);
#endif
                
                for (int i = 0; i < ArrayCount(Player->Body.Components); i++)
                {
                    float EntityRelHeight = 
                        Player->Position.z + Player->Body.Components[i].Origin.z;
                    // TODO: Fazer a interpreação correta da coordenada z par a 
                    // definição da geometria ????
                    //CalcShapePoints(&Player->Body.Components[i], EntityRelHeight);
                    CalcShapePoints(&Player->Body.Components[i], 0.0f);
                }
                
                Player->Velocity.x *= 0.7f;
                Player->Velocity.y *= 0.7f;
            }
            
            if (Entity->Header.State & EntityState_Visible)
            {
                glPushMatrix();
                v3f PlayerP = Entity->Player.Position;
                v3f Scale = Entity->Player.Transform.Scale;
                float FacingAngle = RadsToDegrees(Entity->Player.Transform.Rotation.z);
                OpenGLTRSTransform(PlayerP, FacingAngle, V3f(0, 0, 1), Scale);
                
                for (int i = 0; 
                     i < ArrayCount(Entity->Player.Body.Components);
                     i++)
                {
                    shape Component = Entity->Player.Body.Components[i];
                    v3f Position = Component.Origin;
                    v3f AngleInRadians = Entity->Player.Body.Components[i].Transform.Rotation;
                    glPushMatrix();
                    glTranslatef(Position.x, Position.y, Position.z);
                    v3f RotNormal = Entity->Player.Body.Components[i].RotationNormal;
                    
                    glRotatef(RadsToDegrees(AngleInRadians.x), 1, 0, 0);
                    glRotatef(RadsToDegrees(AngleInRadians.y), 0, 1, 0);
                    glRotatef(RadsToDegrees(AngleInRadians.z), 1, 0, 1);
                    
                    if (Component.Texture)
                    {
                        DrawShape(&Component, Component.OffsetFromOrigin, Component.Texture);
                    }
                    else
                    {
                        DrawShape(&Component, Component.OffsetFromOrigin, 0);
                    }
                    
                    glPopMatrix();
                }
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
                entity_list *Entities = Collision(Entity, Entity->Bullet.Shape);
                
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
                         EntityEntry->Content->Header.Type == Entity_ShortObstacle ||
                         EntityEntry->Content->Header.Type == Entity_TallObstacle))
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
                        }
                    }
                    
                    if (EntityEntry->Content &&
                        EntityEntry->Content->Header.Type == Entity_Player)
                    {
                        Entity->Header.State &= ~EntityState_Visible;
                        Entity->Header.State &= ~EntityState_Active;
                        
                        EntityEntry->Content->Header.State &= ~EntityState_Visible;
                        EntityEntry->Content->Header.State &= ~EntityState_Active;
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
                if (Entity->Enemy.CyclesToShoot == 0)
                {
                    Entity->Enemy.CyclesToShoot = Game.EnemyCountToShoot;
                    //ASSERT(Entity->Enemy.Body.RightArm.Bases.Angle == 0);
                    CreateBulletEntity(GlobalBullets, Entity_Enemy, 100, Entity->Enemy.ArmHeight, 
                                       Entity->Enemy.ShotVelocity, Entity->Enemy.Bases, Entity->Enemy.Transform.Rotation, Entity->Enemy.Body.RightArm,
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
                    Entity->Enemy.Body.RightLeg.Origin.y *= -1;
                    Entity->Enemy.Body.LeftLeg.Origin.y *= -1;
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
                           Entity->Enemy.Body.Head);
                
                if (Entity->Enemy.Jumping)
                {
                    EnemyJump(&Entity->Enemy);
                }
                EnemyCollisionZ(Game.Arena, &Entity->Enemy);
                
                for (int i = 0; i < ArrayCount(Entity->Enemy.Body.Components); i++)
                {
                    float EntityRelHeight = Entity->Enemy.Position.z + Entity->Enemy.Body.Components[i].Origin.z;
                    CalcShapePoints(&Entity->Enemy.Body.Components[i], 0.0f);
                }
            }
            
            
            if (Entity->Header.State & EntityState_Visible)
            {
                glPushMatrix();
                v3f EnemyP = Entity->Enemy.Position;
                v3f Scale = Entity->Enemy.Transform.Scale;
                float FacingAngle = RadsToDegrees(Entity->Enemy.Transform.Rotation.z);
                OpenGLTRSTransform(EnemyP, FacingAngle, V3f(0, 0, 1), Scale);
                
                for (int i = 0; 
                     i < ArrayCount(Entity->Enemy.Body.Components);
                     i++)
                {
                    shape Component = Entity->Enemy.Body.Components[i];
                    v3f RotNormal = Entity->Enemy.Body.Components[i].RotationNormal;
                    v3f Position = Component.Origin;
                    v3f AngleInRadians = Component.Transform.Rotation;
                    
                    glPushMatrix();
                    glTranslatef(Position.x, Position.y, Position.z);
                    glRotatef(RadsToDegrees(AngleInRadians.x), 1, 0, 0);
                    glRotatef(RadsToDegrees(AngleInRadians.y), 0, 1, 0);
                    glRotatef(RadsToDegrees(AngleInRadians.z), 0, 0, 1);
                    DrawShape(&Component, Component.OffsetFromOrigin, 0);
                    glPopMatrix();
                }
                glPopMatrix();
                
            }
        } break;
        
        case Entity_Wall:
        case Entity_CenterLimit:
        {
            DrawShape(&Entity->Static.Shape, Entity->Static.Origin, &WallTexture);
        } break;
        
        case Entity_ShortObstacle:
        {
            DrawShape(&Entity->Static.Shape, Entity->Static.Origin, &FloorTexture);
        } break;
        
        case Entity_Static:
        {
            DrawShape(&Entity->Static.Shape, Entity->Static.Origin, 0);
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
        
        if (strncmp(SVGC->Fill, "blue", strlen("blue")) == 0)
        {
            v3f Center = {SVGC->CenterX, SVGC->CenterY, 0.0f};
            Game.Arena.Background = 
                CreateBackgroundEntity(Entity, SVGC->Id, 0.0, SVGC->Radius, Center, Blue);
            
            CircleCenter.x = Center.x - SVGC->Radius;
            CircleCenter.y = Center.y - SVGC->Radius;
            CenterRadius = SVGC->Radius;
            
            Game.EntityCount++;
            Entity++;
            Center = {SVGC->CenterX, SVGC->CenterY, -20.0f};
            CreateEntityStatic(Entity, Entity_Wall, Blue, 15, 300.0, SVGC->Radius, Center, true);
        }
        
        else if (strncmp(SVGC->Fill, "white", strlen("white")) == 0)
        {
            float Height = 200.0;
            v3f Center = {SVGC->CenterX, SVGC->CenterY, 0.1};
            Game.Arena.CenterLimit = 
                CreateEntityStatic(Entity, Entity_CenterLimit, White, SVGC->Id, Height, 
                                   SVGC->Radius, Center, false);
        }
        
        else if (strncmp(SVGC->Fill, "green", strlen("green")) == 0)
        {
            v3f Center = {SVGC->CenterX, SVGC->CenterY, 0.0f};
            CreatePlayer(Entity, SVGC->Id, 10.0, SVGC->Radius, Center);
            Game.Player = Entity;
        }
        
        else if (strncmp(SVGC->Fill, "red", strlen("red")) == 0)
        {
            v3f Center = {SVGC->CenterX, SVGC->CenterY, 0.0f};
            // TODO: Representação local de inimigos
            entity_enemy *Enemy = &(Game.Enemies[Game.EnemyCount++]);
            Game.Enemies = CreateEnemy(Entity, SVGC->Id, 10.0, SVGC->Radius, Center);
        }
        
        else if (strncmp(SVGC->Fill, "black", strlen("black")) == 0)
        {
            // TODO: Revisar esta fórmula
            float PlatformHeight = JumpHeight * (float)(Game.ObstaclesHeight / 100.0);
            v3f Center = {SVGC->CenterX, SVGC->CenterY, 0.1};
            CreateEntityStatic(Entity, Entity_ShortObstacle, Black, SVGC->Id,
                               PlatformHeight, SVGC->Radius, Center, false);
        }
        
        else
        {
            ASSERT(false);
        }
        
        Game.EntityCount++;
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
    return (V3f(X, (WindowHeight - Y), 0.0));
}

inline void DrawGame(arena Arena)
{
    DrawShape(&Arena.Background->Shape, Arena.Background->Shape.Origin, &FloorTexture);
    UpdateAndDrawEntity(Arena.CenterLimit);
    
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
    glOrtho(0, 1, 0, 1, -1, 1);
    RasterChars(x, y, 0, text, r, g, b);    
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

void Display(void)
{
    GLfloat LighPosition[] = { 0.0, 0.0, 100.0, 1.0 };
    glLightfv(GL_LIGHT0, GL_POSITION, LighPosition);
    
    v3f PlayerP = Game.Player->Player.Position;
    bases PlayerBases = Game.Player->Player.Bases;
    
    v3f SpotlightDirection;
#if 0
    SpotlightDirection.x = 1.0 * PlayerBases.yAxis.x;
    SpotlightDirection.y = 1.0 * PlayerBases.yAxis.y;
    SpotlightDirection.z = 1.0 * PlayerBases.yAxis.z;
#else 
    SpotlightDirection = Game.Player->Player.Bases.yAxis;
#endif
    v3f Pos = V3f(0, 1, 0);
    v4f Position = v4f{PlayerP.x, PlayerP.y, 1, 1};
    printf("%.2f %.2f %.2f\n", SpotlightDirection.x, SpotlightDirection.y, SpotlightDirection.z);
    glLightfv(GL_LIGHT1, GL_POSITION, Position.fv);
    glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, SpotlightDirection.fv);
    
    if (FlashLightOn)
    {
        glEnable(GL_LIGHT0); 
        glEnable(GL_LIGHT1); 
    }
    
    else
    {
        glDisable(GL_LIGHT0); 
        //glDisable(GL_LIGHT1); 
    }
    
    DrawGame(Game.Arena);
    //DrawShape(&Wall1, V3f(0, 0, 0), 0);
    UpdateAndDrawEntity(Game.Player);
    
    for (int i = 0; i < GlobalBulletsCount; i++)
    {
        if (SaveGlobalBullets[i].Header.State & EntityState_Active)
        {
            UpdateAndDrawEntity(&SaveGlobalBullets[i]);
        }
    }
    
#if !DEBUG_PLAYER_LINE
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
        GlobalAllowPerspCameraToMove)
    {
        DistTraveledLastFrame.x = X - GlobalLastXCoordinate;
        DistTraveledLastFrame.y = (WindowWidth - Y) - GlobalLastYCoordinate;
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
    
#if 1
    else if (Button == GLUT_RIGHT_BUTTON)
    {
        
        if (State == GLUT_DOWN)
        {
            GlobalAllowPerspCameraToMove = true;
            GlobalLastXCoordinate = X;
            GlobalLastYCoordinate = WindowWidth - Y;
        }
        
#if 1
        else if (State == GLUT_UP)
        {
            GlobalAllowPerspCameraToMove = false;
        }
#endif
    }
#endif
    
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
    
    if (Key == 'g')
    {
        FlashLightOn = !FlashLightOn;
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
    
#if 1
    printf("Jump velocity: %.4f, Displ. z: %.4f, Radius: %.4f\n", 
           Player->Velocity.z, 
           Player->Position.z, 
           Player->Body.Head.Sphere.Radius);
    
    printf("Torso z: %.4f\n", 
           Player->Header->Origin.z + Player->Body.Torso.Origin.z);
#endif
    
    Player->Position.z = Player->Position.z + Player->Velocity.z * Game.Timing.deltaTime;
    //Player->Position.z *= 1.03;
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
#if 0
    float AccelerationLength = VectorSize(Player->Acceleration);
    if (AccelerationLength > AccelMagnitude)
    {
        Player->Acceleration *= (AccelMagnitude / AccelerationLength);
    }
    
    Player->Velocity = (Player->Acceleration * deltaTime) + Player->Velocity;
#endif
    
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
                        v3f d = Player->Position + NewdeltaPos - Entity->Enemy.Position;
                        d.z = 0;
                        float dCenter = VectorSize(d);
                        sphere Sphere = Player->Body.Head.Sphere; 
                        float dRadius = Sphere.Radius + Entity->Enemy.Body.Head.Sphere.Radius;
                        
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
                        v3f d = Player->Position + NewdeltaPos - Entity->Static.Origin;
                        d.z = 0;
                        float dCenter = VectorSize(d);
                        sphere Sphere = Player->Body.Head.Sphere; 
                        float dRadius = Sphere.Radius + Entity->Static.Shape.Circle.Radius;
                        
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
                        float ToWallDist = 6.0;
                        v3f d = Player->Position + ToWallDist * NewdeltaPos - Entity->Static.Origin;
                        d.z = 0;
                        float dCenter = VectorSize(d);
                        sphere Sphere = Player->Body.Head.Sphere; 
                        float dRadius = Sphere.Radius + Entity->Static.Shape.Circle.Radius;
                        
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
                    float ToWallDist = 6.0;
                    v3f d = Entity->Background.Origin - ToWallDist * NewRelPos - Player->Position;
                    d.z = 0;
                    float dCenter = VectorSize(d) + Player->Body.Head.Sphere.Radius;
                    
                    if (dCenter >= Entity->Background.Shape.Circle.Radius)
                    {
                        v3f Normal = Normalize(d);
                        float Dot = DotProduct(Player->Velocity, Normal);
                        Player->Velocity = Player->Velocity - 1 * Dot * Normal;
                        NewdeltaPos += 0.05f * Player->Velocity * Game.Timing.deltaTime + 0.5 * Normal;
                        
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
    
#if 0
    printf("VX: %.6f\n", Player->Velocity.x);
    printf("VY: %.6f\n\n", Player->Velocity.y);
#endif
    
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
            
            float MinkowskiRadius = JumpingPlatform->Static.Shape.Circle.Radius + Player->Body.Head.Sphere.Radius;
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
        Game.Camera.Type = Camera_FirstPersonEye;
    }
    else if (Input.Keyboard['3'])
    {
        ToThirdPersonCamTransition = true;
        Game.Camera.Type = Camera_ThirdPerson;
    }
    
    if (ShotFired && !Player->Jumping && !Player->Dynamics.PlatformAllow)
    {
        CreateBulletEntity(GlobalBullets, Entity_Player, 100, Player->ArmHeight, Player->ShotVelocity,
                           Player->Bases, Player->Transform.Rotation, Player->Body.RightArm, Player->Position);
        
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
        Player->Body.RightLeg.Origin.y *= -1;
        Player->Body.LeftLeg.Origin.y *= -1;
    }
}

inline bool CameraLerp(v3f *Source, v3f *Dest)
{
    v3f CamTransitionVelocity = {6.0, 6.0, 1.5};
    
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

void CameraUpdate(camera *Camera)
{
    // NOTA: Código relativo ao tratamento da câmera em coordenadas cilindricas
    float RelativeDistXTraveled = DistTraveledLastFrame.x / WindowWidth;
    float RelativeDistYTraveled = DistTraveledLastFrame.y / WindowHeight;
    float NewCameraPhi = CameraPerspectivePhi + PI * Camera->Sensitivity * RelativeDistYTraveled;
    float NewCameraTheta = CameraPerspectiveTheta + PI * Camera->Sensitivity * RelativeDistXTraveled;
    float PlayerAngle = Game.Player->Player.Transform.Rotation.z;
    
    if (CameraPerspMoved)
    {
        if ((NewCameraPhi > -PI/2) && 
            (NewCameraPhi < 0) && 
            (NewCameraTheta - PlayerAngle < -PI) && 
            (NewCameraTheta - PlayerAngle > -2 * PI))
        {
            CameraPerspectiveTheta += PI * Camera->Sensitivity * RelativeDistXTraveled;
            CameraPerspectivePhi += PI * Camera->Sensitivity * RelativeDistYTraveled;
        }
        
        GlobalLastXCoordinate = Game.Input.Mouse.Position.x;
        GlobalLastYCoordinate = Game.Input.Mouse.Position.y;
        
        CameraPerspMoved = false;
    }
    
    float CameraMotionVelocity = 0.8;
    
    if ((NewCameraTheta - PlayerAngle) >= -PI)
    {
        
        CameraPerspectiveTheta -= CameraMotionVelocity * Game.Timing.deltaTime;
    }
    
    else if (NewCameraTheta - PlayerAngle <= -2 * PI)
    {
        CameraPerspectiveTheta += CameraMotionVelocity * Game.Timing.deltaTime;
    }
    
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    
    ASSERT(Game.Player);
    
    uint EnemiesDeadCount = Game.EnemyCount - EnemiesWithStateCount(EntityState_Active);
    char StringStatusEnemies[64];
    snprintf(StringStatusEnemies, 64, "Mortos: %u", EnemiesDeadCount);
    PrintText(0.85, 0.95, StringStatusEnemies, 1, 0, 0);
    
    if ((EnemiesDeadCount == Game.EnemyCount) && !GameEndedWithVictory)
    {
        PrintText(0.45, 0.5, "Ganhou!", 1, 0, 0);
        GameEndedWithFailure = true;
    }
    
    if (!(Game.Player->Header.State & EntityState_Active) && !GameEndedWithFailure)
    {
        PrintText(0.45, 0.5, "Perdeu!", 0, 1, 0);
        GameEndedWithVictory = true;
    }
    
    bases PlayerBases = Game.Player->Player.Bases;
    v3f PlayerP = Game.Player->Header.Origin;
    
    if (Camera->Type == Camera_FirstPersonGun)
    {
        v3f PointingGun;
        PointingGun.x = 0;
        PointingGun.y = PlayerP.y + 0.5 * (Game.Input.Mouse.Position.x-WindowWidth/2);
        PointingGun.z = Game.Input.Mouse.Position.y-WindowHeight/2;
        
        // TODO: Corrigir a altura da arma
        float GunHeight = 70.0;
        
        float CamHorizonDist = 100.0;
        float FirstPersonCamOffset = 20.0;
        
        // TODO: Transicao na mudanca de cameras
        if (ToFirstPersonCamTransition)
        {
            v3f TargetCamPoint;
            TargetCamPoint.x = PlayerP.x+FirstPersonCamOffset*PlayerBases.yAxis.x;
            TargetCamPoint.y = PlayerP.y+FirstPersonCamOffset*PlayerBases.yAxis.y;
            TargetCamPoint.z = GunHeight+PlayerP.z;
            
            ToFirstPersonCamTransition = CameraLerp(&Camera->P, &TargetCamPoint);
        }
        
        else
        {
            Camera->P.x = PlayerP.x+FirstPersonCamOffset*PlayerBases.yAxis.x;
            Camera->P.y = PlayerP.y+FirstPersonCamOffset*PlayerBases.yAxis.y;
            Camera->P.z = GunHeight + PlayerP.z;
        }
        
        v3f PlayerLookingDir = V3f((PlayerP.x+CamHorizonDist*PlayerBases.yAxis.x), 
                                   (PlayerP.y+CamHorizonDist*PlayerBases.yAxis.y), 
                                   ((PlayerP.z+PointingGun.z)+CamHorizonDist*PlayerBases.zAxis.z));
        
        gluLookAt(Camera->P.x, Camera->P.y, Camera->P.z, 
                  PlayerLookingDir.x, PlayerLookingDir.y, PlayerLookingDir.z, 
                  Camera->Up.x, Camera->Up.y, Camera->Up.z);
    }
    
    else if (Camera->Type == Camera_ThirdPerson)
    {
        // NOTA: Câmera em terceira pessoa olhando para o jogador
        float CamBoundingSphereR = 150.0;
        
        if (ToThirdPersonCamTransition)
        {
            v3f TargetCamP;
            TargetCamP.x = PlayerP.x + CamBoundingSphereR * sinf(CameraPerspectivePhi) * cosf(CameraPerspectiveTheta);
            TargetCamP.y = PlayerP.y + CamBoundingSphereR * sinf(CameraPerspectivePhi) * sinf(CameraPerspectiveTheta);
            TargetCamP.z = PlayerP.z + CamBoundingSphereR * cosf(CameraPerspectivePhi);
            
            ToThirdPersonCamTransition = CameraLerp(&Camera->P, &TargetCamP);
        }
        
        else
        {
            Camera->P.x = PlayerP.x + CamBoundingSphereR * sinf(CameraPerspectivePhi) * cosf(CameraPerspectiveTheta);
            Camera->P.y = PlayerP.y + CamBoundingSphereR * sinf(CameraPerspectivePhi) * sinf(CameraPerspectiveTheta);
            Camera->P.z = PlayerP.z + CamBoundingSphereR * cosf(CameraPerspectivePhi);
        }
        
        gluLookAt(Camera->P.x, Camera->P.y, Camera->P.z,
                  PlayerP.x, PlayerP.y, PlayerP.z + 50.0,
                  Camera->Up.x,Camera->Up.y,Camera->Up.z);
    }
}

void UpdateGame(int Value)
{
    ASSERT(Game.Player);
    ProcessInput(Game.Input, &Game.Player->Player);
    MovePlayer(&Game.Player->Player);
    if (Game.Player->Player.Jumping)
    {
        PlayerJump(&Game.Player->Player);
    }
    PlayerCollisionZ(Game.Arena, &Game.Player->Player);
    
    CameraUpdate(&Game.Camera);
    
    glutPostRedisplay();
    glutTimerFunc(Game.Timing.deltaTimeMs, UpdateGame, 0);
}

void Reshape(int Width, int Height)
{
    WindowWidth = Width;
    WindowHeight = Height;
    
    glViewport (0, 0, Width, Height);
    glMatrixMode (GL_PROJECTION);
    glLoadIdentity();
    
    if (Width <= Height)
    {
        gluPerspective(60, (float)Height/(float)Width, 1, 2000);
    }
    
    else
    {
        gluPerspective(60, (float)Width/(float)Height, 1, 2000);
    }
    
    glMatrixMode(GL_MODELVIEW);
}

void Init()
{
    float Size = 1.0f;
    
    glClearColor(1, 1, 1, 0);
    glShadeModel (GL_SMOOTH);
    
    glEnable(GL_LIGHTING);
    //glEnable(GL_LIGHT0);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    
    float AmbientLight[] = {1.0, 1.0, 0.4, 1.0};
    glLightfv(GL_LIGHT1, GL_AMBIENT, AmbientLight);
    glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 30.0);
    glLightf(GL_LIGHT1, GL_SPOT_EXPONENT, 1);
    
    FloorTexture.Id = LoadTextureRAW("texture/Beach_sand_pxr128.bmp");
    BulletTexture.Id = LoadTextureRAW("texture/sun1.bmp");
    FloorNormalTexture.Id = LoadTextureRAW("texture/Beach_sand_pxr128_normal.bmp");
    WallTexture.Id = LoadTextureRAW("texture/Stucco_pxr128.bmp");
    
    StandardMaterial.Emission = {0.4, 0.4, 0.4, 1};
    StandardMaterial.ColorA = {0.4, 0.4, 0.4, 1};
    StandardMaterial.ColorD = {1.0, 1.0, 1.0, 1};
    StandardMaterial.Specular = {0.9, 0.9, 0.9, 1};
    StandardMaterial.Shininess = {80.0};
    
    FloorTexture.Material = StandardMaterial;
    BulletTexture.Material = StandardMaterial;
    WallTexture.Material = StandardMaterial;
    
    // Inicializacao da camera
    Game.Camera.Up = V3f(0.0, 0.0, 1.0);
    Game.Camera.Sensitivity = 0.6;
    Game.Camera.Type = Camera_FirstPersonGun;
    memset(Game.Input.Mouse.ButtonState, -1, 3);
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
            glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);
            glutInitWindowSize(WindowWidth, WindowHeight);
            glutCreateWindow("TF - Guilherme Caetano");
            
            Init();
            glutDisplayFunc(Display);
            glutReshapeFunc(Reshape);
            glutKeyboardUpFunc(ProcessKeyUp);
            glutKeyboardFunc(ProcessKeyboard);
            glutMouseFunc(ProcessMouseInput);
            glutPassiveMotionFunc(ProcessPassiveMotion);
            glutMotionFunc(ProcessMotion);
            glutTimerFunc(Game.Timing.deltaTimeMs, UpdateGame, 0);
            
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