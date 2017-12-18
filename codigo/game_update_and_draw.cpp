
void CameraUpdate(camera *Camera)
{
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    
    ASSERT(Game.Player);
    
    bases PlayerBases = Game.Player->Player.Bases;
    v3f PlayerP = Game.Player->Header.Origin;
    v3f RightArmPos = Game.Player->Player.RightArm->Header.Origin;
    
    if (Camera->Type == Camera_FirstPersonGun)
    {
        float FacingAngle = RadsToDegrees(Game.Player->Player.Transform.Rotation.z+GunTurnX);
        
        float GunHeight = 71.0;
        
        float xAxisOffset = 20.0f;
        float FirstPersonCamOffset = 10.0f;
        
        v3f FirstPersonCamFinalPoint;
        
        FirstPersonCamFinalPoint.x = PlayerP.x;
        FirstPersonCamFinalPoint.y = PlayerP.y;
        FirstPersonCamFinalPoint.z = GunHeight+PlayerP.z;
        
        // TODO: Transicao na mudanca de cameras
        if (ToFirstPersonCamTransition)
        {
            v3f TargetCamPoint = FirstPersonCamFinalPoint;
            ToFirstPersonCamTransition = CameraLerp(&Camera->P, &TargetCamPoint);
        }
        
        else
        {
            Camera->P = FirstPersonCamFinalPoint;
        }
        
        v3f BulletRot = {Game.Player->Player.Transform.Rotation.x, 
            Game.Player->Player.Transform.Rotation.y, Game.Player->Player.Transform.Rotation.z+GunTurnX};
        
        float AbsBulletAngle = BulletRot.z+Game.Player->Player.RightArm->Header.Transform.Rotation.z;
        
        //glMatrixMode(GL_PROJECTION);
        //glLoadIdentity();
        glRotatef(-FacingAngle, 0, 1, 0);
        //glRotatef(-RadsToDegrees(GunTurnY), 1, 0, 0);
        glRotatef(-90, 1, 0, 0);
        glTranslatef(-xAxisOffset*PlayerBases.xAxis.x-FirstPersonCamOffset*PlayerBases.yAxis.x, 
                     -FirstPersonCamOffset*PlayerBases.yAxis.y-xAxisOffset*PlayerBases.xAxis.y, 
                     0);
        glTranslatef(-Camera->P.x, -Camera->P.y, -Camera->P.z);
        //glMatrixMode(GL_MODELVIEW);
        //glLoadIdentity();
    }
    
    else if (Camera->Type == Camera_ThirdPerson)
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
                (NewCameraPhi < 0))
            {
                CameraPerspectiveTheta += PI * Camera->Sensitivity * RelativeDistXTraveled;
                CameraPerspectivePhi += PI * Camera->Sensitivity * RelativeDistYTraveled;
            }
            
            GlobalLastXCoordinate = Game.Input.Mouse.Position.x;
            GlobalLastYCoordinate = Game.Input.Mouse.Position.y;
            
            CameraPerspMoved = false;
        }
        
        float CameraMotionVelocity = 0.8;
        
        // NOTA: Câmera em terceira pessoa olhando para o jogador
        float CamBoundingSphereR = 180.0;
        
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
                  PlayerP.x, PlayerP.y, PlayerP.z + 100.0,
                  Camera->Up.x,Camera->Up.y,Camera->Up.z);
    }
    
}

inline void UpdateLight(v3f PlayerP, bases PlayerBases)
{
    float LightP[] = {600.0, 600.0, 1500.0, 1.0};
    glLightfv(GL_LIGHT0, GL_POSITION, LightP);
    
    v3f BulletRot = {Game.Player->Player.Transform.Rotation.x, 
        Game.Player->Player.Transform.Rotation.y, Game.Player->Player.Transform.Rotation.z+GunTurnX};
    
    float AbsBulletAngle = BulletRot.z+Game.Player->Player.RightArm->Header.Transform.Rotation.z;
    
    m3 Matrix3 = {
        {cosf(AbsBulletAngle), -sinf(AbsBulletAngle), 0},
        {sinf(AbsBulletAngle), cosf(AbsBulletAngle), 0},
        {0, 0, 1}};
    
    m3 RotMatrixXPlane;
    if (DotProduct(V3f(0, 1, 0), PlayerBases.yAxis) < 0.0f)
    {
        // NOTA: Rotaciona bases em torno de x!
        RotMatrixXPlane = {
            1, 0, 0,
            0, cosf(GunTurnY-PI/2), sinf(GunTurnY-PI/2), 
            0, -sinf(GunTurnY-PI/2), cosf(GunTurnY-PI/2)
        };
    }
    
    else
    {
        RotMatrixXPlane = {
            1, 0, 0,
            0, cosf(GunTurnY-PI/2), -sinf(GunTurnY-PI/2), 
            0, sinf(GunTurnY-PI/2), cosf(GunTurnY-PI/2)
        };
    }
    
    m3 SpotlightMatrix = Matrix3 * RotMatrixXPlane;
    
    v3f SpotlightDirection = {SpotlightMatrix.a12, SpotlightMatrix.a22, SpotlightMatrix.a32};
    
    CameraRotationNormal = CrossProduct(Game.Player->Player.Bases.xAxis, SpotlightDirection);
    
    float SpotlightHeight = 69.0f;
    v4f SpotlightOrigin = v4f{PlayerP.x, PlayerP.y, PlayerP.z + SpotlightHeight, 1};
    
    glLightfv(GL_LIGHT1, GL_POSITION, SpotlightOrigin.fv);
    glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, SpotlightDirection.fv);
}

void UpdateMinimap()
{
    for (uint i = 0; i < MinimapEntitiesCount; i++)
    {
        switch(MinimapEntities[i].Header.Type)
        {
            case Entity_MiniPlayer:
            {
                v3f Center = Game.Player->Player.Position;
                MinimapEntities[i].Header.Origin.x = 0.75f + Center.x / MinimapReduxFactor;
                MinimapEntities[i].Header.Origin.y = Center.y / MinimapReduxFactor;
                MinimapEntities[i].Static.Origin = MinimapEntities[i].Header.Origin;
                MinimapEntities[i].Static.Shape.Origin = MinimapEntities[i].Header.Origin;
                MinimapEntities[i].Static.Shape.Circle.Radius = MinimapEntities[i].Static.Shape.Circle.CollisionRadius + 0.0003f*Game.Player->Player.Position.z;
                
                CalcShapePoints(&MinimapEntities[i].Static.Shape, 0.0f);
            } break;
            
            case Entity_MiniEnemy:
            {
                for(uint j = 0; j < Game.EntityCount; j++)
                {
                    if (Game.Entities[j].Header.Id == MinimapEntities[i].Header.Id &&
                        Game.Entities[j].Header.State & EntityState_Visible)
                    {
                        v3f Center = Game.Entities[j].Enemy.Position;
                        MinimapEntities[i].Header.Origin.x = 0.75f + Center.x / MinimapReduxFactor;
                        MinimapEntities[i].Header.Origin.y = Center.y / MinimapReduxFactor;
                        MinimapEntities[i].Static.Origin = MinimapEntities[i].Header.Origin;
                        MinimapEntities[i].Static.Shape.Origin = MinimapEntities[i].Header.Origin;
                        MinimapEntities[i].Static.Shape.Circle.Radius = MinimapEntities[i].Static.Shape.Circle.CollisionRadius + 0.0001f*Game.Player->Enemy.Position.z;
                        
                        CalcShapePoints(&MinimapEntities[i].Static.Shape, 0.0f);
                    }
                    
                    else if (!(Game.Entities[j].Header.State & EntityState_Visible))
                    {
                        
                    }
                }
            } break;
        }
    }
}
