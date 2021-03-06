
inline void CreateLegPairs(svg_color_names Color, float Height, float OffsetX, float OffsetY, shape *LegShape)
{
    LegShape[0].ColorFill = Color;
    LegShape[0].Type = Shape_Box;
    LegShape[0].Origin = v3f{OffsetX, OffsetY, Height};
    LegShape[0].OffsetFromOrigin = V3f(0, 0, -Height/2);
    LegShape[0].Box.Width = 12;
    LegShape[0].Box.Height = 10;
    LegShape[0].Box.Depth = Height/2;
    
    LegShape[1].ColorFill = Color;
    LegShape[1].Type = Shape_Box;
    LegShape[1].Origin = v3f{OffsetX, OffsetY, Height*0.5f+0.5f};
    LegShape[1].Box.Width = 10;
    LegShape[1].Box.Height = 8;
    LegShape[1].Box.Depth = Height;
}

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
                   float ShotVelocity, bases EntityBases, v3f Rotation, shape_tree *BodyPart, 
                   v3f Origin)
{
    Entity->Bullet.VelocityMagnitude = ShotVelocity;
    
    float AbsBulletAngle = Rotation.z + BodyPart->Header.Transform.Rotation.z;
    float AbsYAngle = PI/2;
    
    if (CastingEntityType == Entity_Player)
    {
        AbsYAngle = GunTurnY;
        AbsBulletAngle = Rotation.z + BodyPart->Header.Transform.Rotation.z;
    }
    
    m4 Matrix4 = Game.Player->Player.RightArm->Header.ModelViewMatrix;
    Entity->Bullet.Bases.BaseMatrix = m3{
        {cosf(AbsBulletAngle), sinf(AbsBulletAngle), 0},
        {-sinf(AbsBulletAngle), cosf(AbsBulletAngle), 0},
        {0, 0, 1}};
    
    m3 RotMatrixXPlane;
    
    if (DotProduct(V3f(0, 1, 0), EntityBases.yAxis) < 0.0f)
        //if (1)
    {
        // NOTA: Rotaciona bases em torno de x!
        RotMatrixXPlane = {
            1, 0, 0,
            0, cosf(AbsYAngle-PI/2), -sinf(AbsYAngle-PI/2), 
            0, sinf(AbsYAngle-PI/2), cosf(AbsYAngle-PI/2)
        };
    }
    
    else
    {
        RotMatrixXPlane = {
            1, 0, 0,
            0, cosf(-AbsYAngle+PI/2), -sinf(-AbsYAngle+PI/2), 
            0, sinf(-AbsYAngle+PI/2), cosf(-AbsYAngle+PI/2)
        };
    }
    
    Entity->Bullet.Bases.BaseMatrix = Entity->Bullet.Bases.BaseMatrix * RotMatrixXPlane;
    
    v3f ArmOrigin = CoordinateChange(EntityBases.BaseMatrix, BodyPart->Header.Origin);
    float OffsetToGunEnd = 0.0f;
    v3f ArmOffset01 = CoordinateChange(EntityBases.BaseMatrix, 
                                       V3f(0, 0, OffsetToGunEnd));
    
    v3f ArmOffset = Entity->Bullet.Bases.yAxis*OffsetToGunEnd;
    
    Entity->Bullet.Position = Origin + ArmOrigin + ArmOffset;
    //Entity->Bullet.Position = Origin + ArmOrigin;
    
    FillEntityHeader(&Entity->Header, Id,
                     EntityState_Visible | EntityState_Active, 
                     Entity_Bullet, Height, Entity->Bullet.Position);
    
    Entity->Bullet.Header = &Entity->Header;
    
    Entity->Bullet.Shape.ColorFill = Black;
    Entity->Bullet.Shape.Type = Shape_Circle;
    Entity->Bullet.Shape.Circle.Radius = 2;
    Entity->Bullet.Position.z = Height;
    Entity->Bullet.Shape.Origin = v3f{0.0f, 0.0f, Height};
    Entity->Bullet.CastingEntityType = CastingEntityType;
    
    CalcShapePoints(&Entity->Bullet.Shape, 0.0f);
    
    return &Entity->Bullet;
}

inline entity *
CreateInternalWall(entity *Entity, uint Id, v3f Origin, cylinder Cylinder, texture *Texture)
{
    FillEntityHeader(&Entity->Header, Id, 0x0F, Entity_CenterLimit, Cylinder.Height, Origin);
    Entity->Static.Header = &Entity->Header;
    Entity->Static.Origin = Origin;
    Entity->Static.Shape.ColorFill = White;
    
    Entity->Static.Shape.Type = Shape_Cylinder;
    Entity->Static.Shape.Cylinder = Cylinder;
    Entity->Static.Shape.Transform.Translation = V3f(0, 0, 0);
    Entity->Static.Shape.Transform.Scale = V3f(1.0f, 1.0f, 1.0f);
    Entity->Static.Shape.Texture = Texture;
    
    CalcShapePoints(&Entity->Static.Shape, 0.0);
    return Entity;
}

inline entity *
CreateExternalWall(entity *Entity, uint Id, v3f Origin, cylinder Cylinder, texture *Texture)
{
    FillEntityHeader(&Entity->Header, Id, 0x0F, Entity_Wall, Cylinder.Height, Origin);
    Entity->Static.Header = &Entity->Header;
    Entity->Static.Origin = Origin;
    Entity->Static.Shape.ColorFill = Blue;
    
    Entity->Static.Shape.Type = Shape_Cylinder;
    Entity->Static.Shape.Cylinder = Cylinder;
    Entity->Static.Shape.Transform.Translation = V3f(0, 0, 0);
    Entity->Static.Shape.Transform.Scale = V3f(1.0f, 1.0f, 1.0f);
    Entity->Static.Shape.Texture = Texture;
    
    CalcShapePoints(&Entity->Static.Shape, 0.0);
    return Entity;
}

entity *
CreatePlatform(entity *Entity, entity_type Type, svg_color_names Color, int Id, 
               float Height, float Radius, v3f Origin, bool InvSideNormals, 
               bool DrawBase, bool DrawTop, bool DrawSide)
{
    FillEntityHeader(&Entity->Header, Id, 0x0F, Type, Height, Origin);
    
    Entity->Static.Header = &Entity->Header;
    Entity->Static.Origin = Origin;
    Entity->Static.Shape.ColorFill = Color;
    
    Entity->Static.Shape.Type = Shape_Box;
    Entity->Static.Shape.Box.Width = 2*Radius;
    Entity->Static.Shape.Box.Depth = 2*Radius;
    Entity->Static.Shape.Box.Height = Height;
    
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

shape_tree * AllocBodyTree()
{
    shape_tree * Body = (shape_tree*)malloc(sizeof(shape_tree));
    Body->Header = {};
    Body->Content = {};
    Body->ChildrenLeft = 0;
    Body->ChildrenRight = 0;
}

shape_tree * InserIntoBodyTree(shape_tree * Body, shape_header Header, shape_content Content)
{
    Body->Header = Header;
    Body->Content = Content;
    Body->ChildrenLeft = AllocBodyTree();
    Body->ChildrenRight = AllocBodyTree();
}

shape_tree * FreeBodyTree()
{
    // TODO: Implementar o free da árvore
}

void 
ShapeTreeWalk(shape_tree * Tree, void (*Operator)(shape_tree*))
{
    if (!Tree)
    {
        return;
    }
    
    glPushMatrix();
    {
        Operator(Tree);
        ShapeTreeWalk(Tree->ChildrenLeft, Operator);
        ShapeTreeWalk(Tree->ChildrenRight, Operator);
    }
    glPopMatrix();
    return;
}

entity_player *
CreatePlayerTree(entity *Entity, int Id, float Height, float Radius, v3f Center)
{
    FillEntityHeader(&Entity->Header, Id, 0x0F, Entity_Player, Height, Center);
    
    Entity->Player.Header = &Entity->Header;
    Entity->Player.Position = Center;
    Entity->Player.Transform.Translation = Center;
    
    Entity->Player.ShotVelocity = Game.ShotVelocity;
    Entity->Player.VelocityMagnitude = Game.PlayerVelocity;
    Entity->Player.SpinMagnitude = Game.PlayerVelocity / 60.0f;
    
    Entity->Player.Bases.xAxis = v3f{1.0f, 0.0f, 0.0f};
    Entity->Player.Bases.yAxis = v3f{0.0f, 1.0f, 0.0f};
    Entity->Player.Bases.zAxis = v3f{0.0f, 0.0f, 1.0f};
    
    Entity->Player.Transform.Scale = v3f{1.0f, 1.0f, 1.0f};
    
#if 0
    Entity->Player.BodyTree = AllocBodyTree();
    shape_tree *Pivot= Entity->Player.BodyTree;
    
    Pivot->ChildrenLeft = AllocBodyTree();
    Pivot->ChildrenRight = AllocBodyTree();
    shape_tree *UpperBody= Pivot->ChildrenLeft;
    shape_tree *Pelvis = Pivot->ChildrenRight;
    
    UpperBody->ChildrenLeft = AllocBodyTree();
    UpperBody->ChildrenRight = AllocBodyTree();
    shape_tree *Head = UpperBody->ChildrenLeft;
    shape_tree *Torso = UpperBody->ChildrenRight;
    
    Torso->ChildrenLeft = AllocBodyTree();
    Torso->ChildrenRight = AllocBodyTree();
    shape_tree *LeftArm = Torso->ChildrenLeft;
    shape_tree *RightArm = Torso->ChildrenRight;
    
    RightArm->ChildrenLeft = AllocBodyTree();
    shape_tree *RightHand = RightArm->ChildrenLeft;
    
    Pelvis->ChildrenLeft = AllocBodyTree();
    Pelvis->ChildrenRight = AllocBodyTree();
    shape_tree *LeftLeg = Pelvis->ChildrenLeft;
    shape_tree *RightLeg = Pelvis->ChildrenRight;
    
    Pivot->Header.Type = Shape_Undefined;
    Pivot->Header.Origin = v3f{0.0f, 0.0f, 0.0f};
    Pivot->Header.Transform.Scale = v3f{1.0f, 1.0f, 1.0f};
    Pivot->Header.OffsetFromOrigin = V3f(0, 0, 0);
    
    UpperBody->Header.Type = Shape_Undefined;
    UpperBody->Header.Origin = v3f{0.0f, 0.0f, 40.0f};
    UpperBody->Header.Transform.Scale = v3f{1.0f, 1.0f, 1.0f};
    UpperBody->Header.OffsetFromOrigin = V3f(0, 0, 0);
    
#endif
    
    Entity->Player.BodyTree = AllocBodyTree();
    shape_tree *Head = Entity->Player.BodyTree;
    
    Head->ChildrenLeft = AllocBodyTree();
    Head->ChildrenRight = AllocBodyTree();
    shape_tree *Torso = Head->ChildrenLeft;
    shape_tree *Pelvis = Head->ChildrenRight;
    
    Torso->ChildrenLeft = AllocBodyTree();
    Torso->ChildrenRight = AllocBodyTree();
    shape_tree *LeftArm = Torso->ChildrenLeft;
    shape_tree *RightArm = Torso->ChildrenRight;
    
    RightArm->ChildrenLeft = AllocBodyTree();
    shape_tree *RightHand = RightArm->ChildrenLeft;
    
    Pelvis->ChildrenLeft = AllocBodyTree();
    Pelvis->ChildrenRight = AllocBodyTree();
    shape_tree *LeftLeg = Pelvis->ChildrenLeft;
    shape_tree *RightLeg = Pelvis->ChildrenRight;
    
    LeftLeg->ChildrenLeft = AllocBodyTree();
    shape_tree *LeftLowerLeg = LeftLeg->ChildrenLeft;
    RightLeg->ChildrenLeft = AllocBodyTree();
    shape_tree *RightLowerLeg = RightLeg->ChildrenLeft;
    
    Entity->Player.Head = Head;
    Entity->Player.RightArm = RightArm;
    Entity->Player.RightLeg = RightLeg;
    Entity->Player.LeftLeg = LeftLeg;
    
    float LegZ = 60.0;
    float TorsoZ = LegZ + 1.8* Radius;
    float HeadZ = TorsoZ;
    float ArmWidth = 50.0f;
    
    float AngleToRotate = PI * 0.5f;
    
    Head->Header.ColorFill = Green;
    Head->Header.Type = Shape_Sphere;
    Head->Header.Origin = v3f{0.0f, 0.0f, HeadZ+10.0f};
    //Head->Header.RotationNormal = V3f(0, 0, 1);
    //Head->Header.Transform.Rotation.x = -PI;
    Head->Content.Sphere.Radius = Radius;
    Head->Header.Texture = &HeadTexture;
    Head->Header.Transform.Scale = v3f{1.0f, 1.0f, 1.0f};
    Head->Header.LocalRotate.z = 180;
    Head->Header.LocalRotate.y = 180;
    
    Torso->Header.ColorFill = Green;
    Torso->Header.Type = Shape_Box;
    Torso->Header.Origin = V3f(0.0f, 0.0f, -32.0f);
    Torso->Header.Transform.Scale = v3f{1.0f, 1.0f, 1.0f};
    Torso->Header.OffsetFromOrigin = V3f(0, 0, 0);
    Torso->Content.Box.Width = 1.8 * Radius;
    Torso->Content.Box.Height = 1.0 * Radius;
    Torso->Content.Box.Depth = 2.0 * Radius;
    Torso->Header.Texture = &MilitaireTexture;
    
    RightArm->Header.ColorFill = Green;
    RightArm->Header.Type = Shape_Box;
    RightArm->Header.Origin = v3f{1.2f * Radius, 0.0f, 0.0};
    RightArm->Header.OffsetFromOrigin = V3f(0, 0, -ArmWidth/2);
    RightArm->Header.RotationNormal = V3f(1, 0, 0);
    RightArm->Header.Bases.BaseMatrix = {{1.0,0.0, 0.0}, {0.0, 1.0, 0.0}, {0.0, 0.0, 1.0}};
    RightArm->Header.Transform.Scale = v3f{1.0f, 1.0f, 1.0f};
    RightArm->Header.Transform.Rotation.x = AngleToRotate;
    //RightArm->Header..Bases;
    RightArm->Header.Texture = &MilitaireTexture;
    
    RightArm->Content.Box.Width = 10;
    RightArm->Content.Box.Height = 10;
    RightArm->Content.Box.Depth = ArmWidth*0.9f;
    
    RightHand->Header.ColorFill = Green;
    RightHand->Header.Type = Shape_MeshObject;
    RightHand->Header.Origin = v3f{0.0f, -5.0f, -40.0};
    RightHand->Header.OffsetFromOrigin = V3f(0.0f, 0, 0);
    RightHand->Header.RotationNormal = V3f(1, 0, 0);
    RightHand->Header.Transform.Scale = v3f{0.5f, .5f, .5f};
    RightHand->Header.Transform.Rotation.x = 0;
    RightHand->Header.Transform.Rotation.y = -PI/2;
    //RightHand->Header..Bases;
    RightHand->Header.Texture = 0;
    RightHand->Header.BuiltinTexture = true;
    
    LeftArm->Header.ColorFill = Green;
    LeftArm->Header.Type = Shape_Box;
    LeftArm->Header.Origin = v3f{-1.2f * Radius, 0.0f, -10.0f};
    LeftArm->Header.OffsetFromOrigin = V3f(0, 0, 0);
    //LeftArm->Header.RotationNormal = V3f(1, 0, 0);
    LeftArm->Header.Transform.Scale = v3f{1.0f, 1.0f, 1.0f};
    LeftArm->Header.Transform.Rotation.x = 0.0f;
    //LeftArm->Header..Bases;
    LeftArm->Header.Texture = &MilitaireTexture;
    
    LeftArm->Content.Box.Width = 10;
    LeftArm->Content.Box.Height = 10;
    LeftArm->Content.Box.Depth = ArmWidth;
    
    Pelvis->Header.Type = Shape_Undefined;
    Pelvis->Header.Origin = v3f{0, 0, -70.0f};
    Pelvis->Header.OffsetFromOrigin = V3f(0, 0, 0);
    Pelvis->Header.Transform.Scale = v3f{1.0f, 1.0f, 1.0f};
    
    float OffsetX = 0.5f * Radius;
    float OffsetY = 5.0f;
    
    LeftLeg->Header.ColorFill = Black;
    LeftLeg->Header.Type = Shape_Box;
    LeftLeg->Header.Origin = v3f{-OffsetX, -OffsetY, 10.3f};
    LeftLeg->Header.OffsetFromOrigin = V3f(0, 0, -12.5f);
    LeftLeg->Header.Transform.Scale = v3f{1.0f, 1.0f, 1.0f};
    LeftLeg->Content.Box.Width = 12;
    LeftLeg->Content.Box.Height = 10;
    LeftLeg->Content.Box.Depth = 25.0f;
    LeftLeg->Header.Texture = &MilitaireTexture;
    
    LeftLowerLeg->Header.ColorFill = Black;
    LeftLowerLeg->Header.Type = Shape_Box;
    LeftLowerLeg->Header.Origin = v3f{0, 0, -12.5f};
    LeftLowerLeg->Header.OffsetFromOrigin = V3f(0, 0, 0);
    LeftLowerLeg->Header.LocalTranslate = {0, 0, -12.5f};
    LeftLowerLeg->Header.Transform.Scale = v3f{1.0f, 1.0f, 1.0f};
    LeftLowerLeg->Content.Box.Width = 10;
    LeftLowerLeg->Content.Box.Height = 8;
    LeftLowerLeg->Content.Box.Depth = 25.0f;
    LeftLowerLeg->Header.Texture = &MilitaireTexture;
    
    RightLeg->Header.ColorFill = Black;
    RightLeg->Header.Type = Shape_Box;
    RightLeg->Header.Origin = v3f{OffsetX, OffsetY, 10.3f};
    RightLeg->Header.LocalTranslate = {0, 0, -12.5f};
    RightLeg->Header.Transform.Scale = v3f{1.0f, 1.0f, 1.0f};
    RightLeg->Content.Box.Width = 12;
    RightLeg->Content.Box.Height = 10;
    RightLeg->Content.Box.Depth = 25.0f;
    RightLeg->Header.Texture = &MilitaireTexture;
    
    RightLowerLeg->Header.ColorFill = Black;
    RightLowerLeg->Header.Type = Shape_Box;
    RightLowerLeg->Header.Origin = v3f{0, 0, -12.5f};
    RightLowerLeg->Header.OffsetFromOrigin = V3f(0, 0, 0.0f);
    RightLowerLeg->Header.LocalTranslate = {0, 0, -12.5f};
    RightLowerLeg->Header.Transform.Scale = v3f{1.0f, 1.0f, 1.0f};
    RightLowerLeg->Content.Box.Width = 10;
    RightLowerLeg->Content.Box.Height = 8;
    RightLowerLeg->Content.Box.Depth = 25.0f;
    RightLowerLeg->Header.Texture = &MilitaireTexture;
    
    Entity->Player.ArmHeight = 85.0f;
    
    ShapeTreeWalk(Head, CalcShapeTreePoints);
    
    return &Entity->Player;
}

entity_enemy *
CreateEnemyTree(entity *Entity, int Id, float Height, float Radius, v3f Center)
{
    FillEntityHeader(&Entity->Header, Id, 0x0F, Entity_Enemy, Height, Center);
    
    Entity->Enemy.Header = &Entity->Header;
    Entity->Enemy.Position = Center;
    Entity->Enemy.Transform.Translation = Center;
    Entity->Enemy.Transform.Rotation = {};
    
    Entity->Enemy.ShotVelocity = Game.ShotVelocity;
    Entity->Enemy.VelocityMagnitude = Game.PlayerVelocity;
    Entity->Enemy.SpinMagnitude = Game.PlayerVelocity / 60.0f;
    
    Entity->Enemy.Bases.xAxis = v3f{1.0f, 0.0f, 0.0f};
    Entity->Enemy.Bases.yAxis = v3f{0.0f, 1.0f, 0.0f};
    Entity->Enemy.Bases.zAxis = v3f{0.0f, 0.0f, 1.0f};
    
    Entity->Enemy.Transform.Scale = v3f{1.0f, 1.0f, 1.0f};
    
    Entity->Enemy.BodyTree = AllocBodyTree();
    shape_tree *Head = Entity->Enemy.BodyTree;
    
    Head->ChildrenLeft = AllocBodyTree();
    Head->ChildrenRight = AllocBodyTree();
    shape_tree *Torso = Head->ChildrenLeft;
    shape_tree *Pelvis = Head->ChildrenRight;
    
    Torso->ChildrenLeft = AllocBodyTree();
    Torso->ChildrenRight = AllocBodyTree();
    shape_tree *LeftArm = Torso->ChildrenLeft;
    shape_tree *RightArm = Torso->ChildrenRight;
    
    RightArm->ChildrenLeft = AllocBodyTree();
    shape_tree *RightHand = RightArm->ChildrenLeft;
    
    Pelvis->ChildrenLeft = AllocBodyTree();
    Pelvis->ChildrenRight = AllocBodyTree();
    shape_tree *LeftLeg = Pelvis->ChildrenLeft;
    shape_tree *RightLeg = Pelvis->ChildrenRight;
    
    LeftLeg->ChildrenLeft = AllocBodyTree();
    shape_tree *LeftLowerLeg = LeftLeg->ChildrenLeft;
    RightLeg->ChildrenLeft = AllocBodyTree();
    shape_tree *RightLowerLeg = RightLeg->ChildrenLeft;
    
    srand((uint)time(0)+Id*101);
    float PiFraction = (rand() / 65536.0) * PI;
    RotateOrthonormalBases(&Entity->Enemy.Bases, -PiFraction);
    Entity->Enemy.Transform.Rotation.z = -PiFraction;
    
    Entity->Enemy.CyclesToChangeWalkingDirection = 180;
    Entity->Enemy.CountToChangeWalkingDirection = 180;
    
    Entity->Enemy.Head = Head;
    Entity->Enemy.RightArm = RightArm;
    Entity->Enemy.RightLeg = RightLeg;
    Entity->Enemy.LeftLeg = LeftLeg;
    
    float LegZ = 60.0;
    float TorsoZ = LegZ + 1.5 * Radius;
    float HeadZ = TorsoZ;
    float ArmWidth = 50.0f;
    float OffsetX = 0.5f * Radius;
    float OffsetY = 5.0f;
    
    float AngleToRotate = PI * 0.5f;
    
    Head->Header.ColorFill = Red;
    Head->Header.Type = Shape_Sphere;
    Head->Header.Origin = v3f{0.0f, 0.0f, HeadZ};
    //Head->Header.RotationNormal = V3f(0, 0, 1);
    //Head->Header.Transform.Rotation.x = -PI;
    Head->Content.Sphere.Radius = Radius;
    Head->Header.Texture = &HeadTexture;
    Head->Header.Transform.Scale = v3f{1.0f, 1.0f, 1.0f};
    Head->Header.Transform.Rotation = v3f{0.0f, 0.0f, -PiFraction};
    Head->Header.LocalRotate.z = 180;
    Head->Header.LocalRotate.y = 180;
    
    Torso->Header.ColorFill = Red;
    Torso->Header.Type = Shape_Box;
    Torso->Header.Origin.x = 0.0;
    Torso->Header.Origin.y = 0.0;
    Torso->Header.Origin.z = -Radius-10.0f;
    Torso->Header.Transform.Scale = v3f{1.0f, 1.0f, 1.0f};
    Torso->Content.Box.Width = 2.0 * Radius;
    Torso->Content.Box.Height = 1.2 * Radius;
    Torso->Content.Box.Depth = 1.5 * Radius;
    Torso->Header.Texture = &MilitaireTexture;
    
    RightArm->Header.ColorFill = Red;
    RightArm->Header.Type = Shape_Box;
    RightArm->Header.Origin = v3f{1.2f * Radius, 0.0f, 0.0};
    RightArm->Header.OffsetFromOrigin = V3f(0, 0, -ArmWidth/2);
    RightArm->Header.RotationNormal = V3f(1, 0, 0);
    RightArm->Header.Bases.BaseMatrix = {{1.0,0.0, 0.0}, {0.0, 1.0, 0.0}, {0.0, 0.0, 1.0}};
    RightArm->Header.Transform.Scale = v3f{1.0f, 1.0f, 1.0f};
    RightArm->Header.Transform.Rotation.x = AngleToRotate;
    //RightArm->Header..Bases;
    RightArm->Header.Texture = &MilitaireTexture;
    
    RightArm->Content.Box.Width = 10;
    RightArm->Content.Box.Height = 10;
    RightArm->Content.Box.Depth = ArmWidth*0.9f;
    
    RightHand->Header.ColorFill = Red;
    RightHand->Header.Type = Shape_MeshObject;
    RightHand->Header.Origin = v3f{0.0f, -5.0f, -40.0};
    RightHand->Header.OffsetFromOrigin = V3f(0.0f, 0, 0);
    RightHand->Header.RotationNormal = V3f(1, 0, 0);
    RightHand->Header.Transform.Scale = v3f{0.5f, .5f, .5f};
    RightHand->Header.Transform.Rotation.x = 0;
    RightHand->Header.Transform.Rotation.y = -PI/2;
    //RightHand->Header..Bases;
    RightHand->Header.Texture = 0;
    RightHand->Header.BuiltinTexture = true;
    
    LeftArm->Header.ColorFill = Red;
    LeftArm->Header.Type = Shape_Box;
    LeftArm->Header.Origin = v3f{-1.2f * Radius, 0.0f, -10.0f};
    LeftArm->Header.OffsetFromOrigin = V3f(0, 0, 0);
    //LeftArm->Header.RotationNormal = V3f(1, 0, 0);
    LeftArm->Header.Transform.Scale = v3f{1.0f, 1.0f, 1.0f};
    LeftArm->Header.Transform.Rotation.x = 0.0f;
    //LeftArm->Header..Bases;
    LeftArm->Header.Texture = &MilitaireTexture;
    
    LeftArm->Content.Box.Width = 10;
    LeftArm->Content.Box.Height = 10;
    LeftArm->Content.Box.Depth = ArmWidth;
    
    Pelvis->Header.Type = Shape_Undefined;
    Pelvis->Header.Origin = v3f{0, 0, -60.0f};
    Pelvis->Header.OffsetFromOrigin = V3f(0, 0, 0);
    Pelvis->Header.Transform.Scale = v3f{1.0f, 1.0f, 1.0f};
    
    LeftLeg->Header.ColorFill = Black;
    LeftLeg->Header.Type = Shape_Box;
    LeftLeg->Header.Origin = v3f{-OffsetX, -OffsetY, 10.3f};
    LeftLeg->Header.OffsetFromOrigin = V3f(0, 0, -12.5f);
    LeftLeg->Header.Transform.Scale = v3f{1.0f, 1.0f, 1.0f};
    LeftLeg->Content.Box.Width = 12;
    LeftLeg->Content.Box.Height = 10;
    LeftLeg->Content.Box.Depth = 25.0f;
    LeftLeg->Header.Texture = &MilitaireTexture;
    
    LeftLowerLeg->Header.ColorFill = Black;
    LeftLowerLeg->Header.Type = Shape_Box;
    LeftLowerLeg->Header.Origin = v3f{0, 0, -12.5f};
    LeftLowerLeg->Header.OffsetFromOrigin = V3f(0, 0, 0);
    LeftLowerLeg->Header.LocalTranslate = {0, 0, -12.5f};
    LeftLowerLeg->Header.Transform.Scale = v3f{1.0f, 1.0f, 1.0f};
    LeftLowerLeg->Content.Box.Width = 10;
    LeftLowerLeg->Content.Box.Height = 8;
    LeftLowerLeg->Content.Box.Depth = 25.0f;
    LeftLowerLeg->Header.Texture = &MilitaireTexture;
    
    RightLeg->Header.ColorFill = Black;
    RightLeg->Header.Type = Shape_Box;
    RightLeg->Header.Origin = v3f{OffsetX, OffsetY, 10.3f};
    RightLeg->Header.LocalTranslate = {0, 0, -12.5f};
    RightLeg->Header.Transform.Scale = v3f{1.0f, 1.0f, 1.0f};
    RightLeg->Content.Box.Width = 12;
    RightLeg->Content.Box.Height = 10;
    RightLeg->Content.Box.Depth = 25.0f;
    RightLeg->Header.Texture = &MilitaireTexture;
    
    RightLowerLeg->Header.ColorFill = Black;
    RightLowerLeg->Header.Type = Shape_Box;
    RightLowerLeg->Header.Origin = v3f{0, 0, -12.5f};
    RightLowerLeg->Header.OffsetFromOrigin = V3f(0, 0, 0.0f);
    RightLowerLeg->Header.LocalTranslate = {0, 0, -12.5f};
    RightLowerLeg->Header.Transform.Scale = v3f{1.0f, 1.0f, 1.0f};
    RightLowerLeg->Content.Box.Width = 10;
    RightLowerLeg->Content.Box.Height = 8;
    RightLowerLeg->Content.Box.Depth = 25.0f;
    RightLowerLeg->Header.Texture = &MilitaireTexture;
    
    Entity->Enemy.ArmHeight = 80.0f;
    
    ShapeTreeWalk(Head, CalcShapeTreePoints);
    
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
