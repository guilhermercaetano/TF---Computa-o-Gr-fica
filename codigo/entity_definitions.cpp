
inline void 
FillEntityHeader(entity_header *Header, int Id, uint State, entity_type Type, 
                 float Height, v3f Origin)
{
    Header->Id = Id;
    Header->Type = Type;
    Header->Height = Height;
    Header->State = State;
    Header->Origin = Origin;
}

inline entity_bullet *
CreateBulletEntity(entity *Entity, entity_type CastingEntityType, int Id, float Height, 
                   float ShotVelocity, bases EntityBases, v3f Rotation, shape BodyPart, v3f Origin)
{
    Entity->Bullet.VelocityMagnitude = ShotVelocity;
    
    //RotateOrthonormalBases(&Entity->Bullet.Bases, EntityBases.Angle + BodyPart.Bases.Angle);
    v3f ArmOrigin = CoordinateChange(EntityBases.BaseMatrix, BodyPart.Origin);
    v3f ArmOffset01 = CoordinateChange(EntityBases.BaseMatrix, 
                                       V3f(0, BodyPart.Box.Depth, 0));
    v3f ArmOffset = CoordinateChange(BodyPart.Bases.BaseMatrix, ArmOffset01);
    
    Entity->Bullet.Position = Origin + ArmOrigin + ArmOffset;
    
    FillEntityHeader(&Entity->Header, Id,
                     EntityState_Visible | EntityState_Move | EntityState_Active, 
                     Entity_Bullet, Height, Entity->Bullet.Position);
    
    Entity->Bullet.Header = &Entity->Header;
    float AbsBulletAngle = Rotation.z + BodyPart.Transform.Rotation.z;
    RotateOrthonormalBases(&Entity->Bullet.Bases, AbsBulletAngle);
    
    Entity->Bullet.Shape.ColorFill = Black;
    Entity->Bullet.Shape.Type = Shape_Circle;
    Entity->Bullet.Shape.Circle.Radius = 3;
    Entity->Bullet.Shape.Origin = v3f{0.0f, 0.0f, 0.0f};
    Entity->Bullet.CastingEntityType = CastingEntityType;
    
    CalcShapePoints(&Entity->Bullet.Shape, 0.0f);
    
    return &Entity->Bullet;
}

inline entity *
CreateInternalWall(entity *Entity, uint Id, v3f Origin, cylinder Cylinder)
{
    FillEntityHeader(&Entity->Header, Id, 0x0F, Entity_CenterLimit, Cylinder.Height, Origin);
    Entity->Static.Header = &Entity->Header;
    Entity->Static.Origin = Origin;
    Entity->Static.Shape.ColorFill = White;
    
    Entity->Static.Shape.Type = Shape_Cylinder;
    Entity->Static.Shape.Cylinder = Cylinder;
    Entity->Static.Shape.Transform.Translation = V3f(0, 0, 0);
    
    CalcShapePoints(&Entity->Static.Shape, 0.0);
    return Entity;
}

inline entity *
CreateExternalWall(entity *Entity, uint Id, v3f Origin, cylinder Cylinder)
{
    FillEntityHeader(&Entity->Header, Id, 0x0F, Entity_Wall, Cylinder.Height, Origin);
    Entity->Static.Header = &Entity->Header;
    Entity->Static.Origin = Origin;
    Entity->Static.Shape.ColorFill = Blue;
    
    Entity->Static.Shape.Type = Shape_Cylinder;
    Entity->Static.Shape.Cylinder = Cylinder;
    Entity->Static.Shape.Transform.Translation = V3f(0, 0, 0);
    
    CalcShapePoints(&Entity->Static.Shape, 0.0);
    return Entity;
}

entity *
CreateEntityStatic(entity *Entity, entity_type Type, svg_color_names Color, int Id, 
                   float Height, float Radius, v3f Origin, bool InvSideNormals, 
                   bool DrawBase, bool DrawTop, bool DrawSide)
{
    FillEntityHeader(&Entity->Header, Id, 0x0F, Type, Height, Origin);
    
    Entity->Static.Header = &Entity->Header;
    Entity->Static.Origin = Origin;
    Entity->Static.Shape.ColorFill = Color;
    
    Entity->Static.Shape.Type = Shape_Cylinder;
    Entity->Static.Shape.Cylinder.Radius = Radius;
    Entity->Static.Shape.Cylinder.Height = Height;
    Entity->Static.Shape.Cylinder.InvSideNormals = InvSideNormals;
    Entity->Static.Shape.Cylinder.DrawBase = DrawBase;
    Entity->Static.Shape.Cylinder.DrawTop = DrawTop;
    Entity->Static.Shape.Cylinder.DrawSide = DrawSide;
    
    Entity->Static.Shape.Transform.Translation = V3f(0, 0, 0);
    
    CalcShapePoints(&Entity->Static.Shape, 0.0);
    return Entity;
}

entity_player *
CreatePlayer(entity *Entity, int Id, float Height, float Radius, v3f Center)
{
    FillEntityHeader(&Entity->Header, Id, 0x0F, Entity_Player, Height, Center);
    
    Entity->Player.Header = &Entity->Header;
    Entity->Player.Position = Center;
    Entity->Player.Transform.Translation = Center;
    
    Entity->Player.ShotVelocity = Game.ShotVelocity;
    Entity->Player.VelocityMagnitude = Game.PlayerVelocity;
    Entity->Player.SpinMagnitude = Game.PlayerVelocity / 90.0;
    
    Entity->Player.Bases.xAxis = v3f{1.0f, 0.0f, 0.0f};
    Entity->Player.Bases.yAxis = v3f{0.0f, 1.0f, 0.0f};
    Entity->Player.Bases.zAxis = v3f{0.0f, 0.0f, 1.0f};
    
    Entity->Player.Transform.Scale = v3f{1.0f, 1.0f, 1.0f};
    
    Entity->Player.Body.RightLeg.ColorFill = Green;
    Entity->Player.Body.RightLeg.Type = Shape_Box;
    Entity->Player.Body.RightLeg.Origin = v3f{0.5f * Radius, 5, 20.5};
    Entity->Player.Body.RightLeg.Box.Width = 12;
    Entity->Player.Body.RightLeg.Box.Height = 10;
    Entity->Player.Body.RightLeg.Box.Depth = 40;
    
    Entity->Player.Body.LeftLeg.ColorFill = Green;
    Entity->Player.Body.LeftLeg.Type = Shape_Box;
    Entity->Player.Body.LeftLeg.Origin = v3f{-0.5f * Radius, -5, 20.5};
    Entity->Player.Body.LeftLeg.Box.Width = 12;
    Entity->Player.Body.LeftLeg.Box.Height = 10;
    Entity->Player.Body.LeftLeg.Box.Depth = 40;
    
    Entity->Player.Body.RightArm.ColorFill = Green;
    Entity->Player.Body.RightArm.Type = Shape_Box;
    Entity->Player.Body.RightArm.Origin = v3f{1.3f * Radius, 0, 50};
    Entity->Player.Body.RightArm.OffsetFromOrigin = V3f(0, 0, 10);
    Entity->Player.Body.RightArm.Box.Width = 10;
    Entity->Player.Body.RightArm.Box.Height = 10;
    Entity->Player.Body.RightArm.Box.Depth = 30;
    
    float AngleToRotate = -PI / 2;
    Entity->Player.Body.RightArm.Transform.Rotation.x = AngleToRotate;
    Entity->Player.Body.RightArm.RotationNormal = V3f(1, 0, 0);
    Entity->Player.Body.RightArm.Transform.Scale = v3f{1.0f, 1.0f, 1.0f};
    
    Entity->Player.Body.LeftArm.ColorFill = Green;
    Entity->Player.Body.LeftArm.Type = Shape_Box;
    Entity->Player.Body.LeftArm.Origin = v3f{-1.3f * Radius, 0, 40};
    Entity->Player.Body.LeftArm.OffsetFromOrigin = V3f(0, 10, 0);
    Entity->Player.Body.LeftArm.Box.Width = 10;
    Entity->Player.Body.LeftArm.Box.Height = 10;
    Entity->Player.Body.LeftArm.Box.Depth = 30;
    
    Entity->Player.ArmHeight = Entity->Player.Body.LeftArm.Origin.z;
    
    Entity->Player.Body.Torso.ColorFill = Green;
    Entity->Player.Body.Torso.Type = Shape_Box;
    Entity->Player.Body.Torso.Origin = v3f{0.0f, 0.0f, 40.0};
    Entity->Player.Body.Torso.Box.Width = 2.0 * Radius;
    Entity->Player.Body.Torso.Box.Height = 0.7 * Radius;
    Entity->Player.Body.Torso.Box.Depth = 1.5 * Radius;
    
    Entity->Player.Body.Head.ColorFill = Green;
    Entity->Player.Body.Head.Type = Shape_Sphere;
    Entity->Player.Body.Head.Sphere.Radius = Radius;
    Entity->Player.Body.Head.Origin = v3f{0.0f, 0.0f, 80.0};
    
    return &Entity->Player;
}

entity_enemy *
CreateEnemy(entity *Entity, int Id, float Height, float Radius, v3f Center)
{
    FillEntityHeader(&Entity->Header, Id, 0x0F, Entity_Enemy, Height, Center);
    
    Entity->Enemy.Header = &Entity->Header;
    Entity->Enemy.Position = Center;
    Entity->Enemy.Transform.Translation = Center;
    
    Entity->Enemy.ShotVelocity = Game.ShotVelocity;
    Entity->Enemy.VelocityMagnitude = Game.PlayerVelocity;
    Entity->Enemy.SpinMagnitude = Game.PlayerVelocity / 90.0;
    
    Entity->Enemy.ShotFrequency = Game.EnemyShotFrequency;
    Entity->Enemy.CyclesToShoot = Game.EnemyCountToShoot;
    
    Entity->Enemy.Bases = {};
    Entity->Enemy.Bases.xAxis = {1.0, 0.0, 0.0};
    Entity->Enemy.Bases.yAxis = {0.0, 1.0, 0.0};
    Entity->Enemy.Bases.zAxis = {0.0, 0.0, 1.0};
    Entity->Enemy.Transform.Scale = v3f{1.0, 1.0, 1.0};
    
    srand((uint)time(0)+Id*101);
    float PiFraction = (rand() / 65536.0) * PI;
    RotateOrthonormalBases(&Entity->Enemy.Bases, -PiFraction);
    Entity->Enemy.Transform.Rotation.z = -PiFraction;
    
    Entity->Enemy.CyclesToChangeWalkingDirection = 180;
    Entity->Enemy.CountToChangeWalkingDirection = 180;
    
    Entity->Enemy.Body.RightLeg.ColorFill = Red;
    Entity->Enemy.Body.RightLeg.Type = Shape_Box;
    Entity->Enemy.Body.RightLeg.Origin = v3f{0.5f * Radius, 5, 20.5};
    Entity->Enemy.Body.RightLeg.Box.Width = 12;
    Entity->Enemy.Body.RightLeg.Box.Height = 10;
    Entity->Enemy.Body.RightLeg.Box.Depth = 40;
    
    Entity->Enemy.Body.LeftLeg.ColorFill = Red;
    Entity->Enemy.Body.LeftLeg.Type = Shape_Box;
    Entity->Enemy.Body.LeftLeg.Origin = v3f{-0.5f * Radius, -5, 20.5};
    Entity->Enemy.Body.LeftLeg.Box.Width = 12;
    Entity->Enemy.Body.LeftLeg.Box.Height = 10;
    Entity->Enemy.Body.LeftLeg.Box.Depth = 40;
    
    Entity->Enemy.Body.RightArm.ColorFill = Red;
    Entity->Enemy.Body.RightArm.Type = Shape_Box;
    Entity->Enemy.Body.RightArm.Origin = v3f{1.3f * Radius, 0, 40};
    Entity->Enemy.Body.RightArm.OffsetFromOrigin = V3f(0, 0, 15);
    Entity->Enemy.Body.RightArm.Box.Width = 10;
    Entity->Enemy.Body.RightArm.Box.Height = 10;
    Entity->Enemy.Body.RightArm.Box.Depth = 30;
    
    Entity->Enemy.Body.RightArm.Bases.xAxis = v3f{1.0f, 0.0f, 0.0f};
    Entity->Enemy.Body.RightArm.Bases.yAxis = v3f{0.0f, 1.0f, 0.0f};
    Entity->Enemy.Body.RightArm.Bases.zAxis = v3f{0.0f, 0.0f, 1.0f};
    
    Entity->Enemy.Body.RightArm.Transform.Scale = v3f{1.0f, 1.0f, 1.0f};
    
    float AngleToRotate = -PI / 2;
    Entity->Enemy.Body.RightArm.Transform.Rotation.x = AngleToRotate;
    Entity->Enemy.Body.RightArm.RotationNormal = V3f(1, 0, 0);
    
    Entity->Enemy.Body.LeftArm.ColorFill = Red;
    Entity->Enemy.Body.LeftArm.Type = Shape_Box;
    Entity->Enemy.Body.LeftArm.Origin = v3f{-1.3f * Radius, 0, 40};
    Entity->Enemy.Body.LeftArm.OffsetFromOrigin = V3f(0, 30/2, 0);
    Entity->Enemy.Body.LeftArm.Box.Width = 10;
    Entity->Enemy.Body.LeftArm.Box.Height = 10;
    Entity->Enemy.Body.LeftArm.Box.Depth = 30;
    
    Entity->Enemy.ArmHeight = 40.0;
    
    Entity->Enemy.Body.Torso.ColorFill = Red;
    Entity->Enemy.Body.Torso.Type = Shape_Box;
    Entity->Enemy.Body.Torso.Origin = v3f{0.0f, 0.0f, 40};
    Entity->Enemy.Body.Torso.Box.Width = 1.6f * Radius;
    Entity->Enemy.Body.Torso.Box.Height = 0.6f * Radius;
    Entity->Enemy.Body.Torso.Box.Depth = 1.5 * Radius;
    
    Entity->Enemy.Body.Head.ColorFill = Red;
    Entity->Enemy.Body.Head.Type = Shape_Sphere;
    Entity->Enemy.Body.Head.Sphere.Radius = Radius;
    Entity->Enemy.Body.Head.Origin = v3f{0.0f, 0.0f, 80};
    
    return &Entity->Enemy;
}

entity_background *
CreateBackground(entity *Entity, int Id, float Height, float Radius, v3f Origin, 
                 svg_color_names Color)
{
    Entity->Header.Id = Id;
    Entity->Header.Type = Entity_Background;
    Entity->Header.Height = Height;
    Entity->Header.State = EntityState_Visible | EntityState_Active;
    
    Entity->Background.Header = &Entity->Header;
    Entity->Background.Origin = Origin;
    Entity->Background.Shape.ColorFill  = Color;
    Entity->Background.Shape.Origin = Origin;
    
    Entity->Background.Shape.Type = Shape_SubdividedRectangle;
    // HACK: Sem essa constante, o cenário nâo preenche corretamente, pois ele
    // encobre 1 a menos o número necessário de grupo de pontos para o chão
    uint MagicConstant = 20;
    uint ChunkSizeX = 20;
    uint ChunkSizeY = 20;
    Entity->Background.Shape.Rectangle.Width = 2 * (Radius+MagicConstant);
    Entity->Background.Shape.Rectangle.Height = 2 * (Radius+MagicConstant);
    Entity->Background.Shape.Rectangle.ChunkSizeX = ChunkSizeX;
    Entity->Background.Shape.Rectangle.ChunkSizeY = ChunkSizeY;
    
    CalcSubdividedRectVertices(&Entity->Background.Shape.Rectangle, Height, 
                               20.0, 20.0);
    
    return &Entity->Background;
}
