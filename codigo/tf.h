#ifndef _MAIN_
#define _MAIN_

#include "svg_colors.h"

typedef struct transform
{
    v3f Translation;
    v3f Rotation;
    v3f Scale;
} transform;

#include "shape_definitions.h"

#define STRCMP(STR, LSTR) strncmp(STR, LSTR, strlen(LSTR))
#define PI 3.14159265359

#define MAX_ENTITIES 100
#define GRAVITY_ACCELERATION 9.8
#define GravityAccelerationScale 12.0
#define AccelMagnitude 500.0

#define ASSERT(COND) if (!(COND)) {*(int *)0 = 0;}
#define CONCAT(A, B) A #B
#define ArrayCount(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))

typedef unsigned int uint;

typedef enum file_type
{
    FileType_Svg,
    FileType_Obj,
} file_type;

typedef enum entity_type
{
    Entity_ShortObstacle,
    Entity_TallObstacle,
    Entity_CenterLimit,
    Entity_Player,
    Entity_Background,
    Entity_Enemy,
    Entity_Bullet,
    Entity_Static,
    Entity_Wall,
} entity_type;

typedef enum entity_state
{
    EntityState_Visible = 0x01,
    EntityState_Move = 0x02,
    EntityState_ChangeSize = 0x04,
    EntityState_Active = 0x08,
} entity_state;

enum camera_type
{
    Camera_FirstPersonGun,
    Camera_FirstPersonEye,
    Camera_ThirdPerson,
}; 

// Definição SVG de um círculo
// NOTA: Difere de circle pelo fato de que circle é a definição de um círclulo
// para essa aplicação em particular
typedef struct svg_circle
{
    int Id;
    float CenterX;
    float CenterY;
    float Radius;
    char Fill[16];
} svg_circle;

typedef struct svg_header
{
    char Xmlns[128];
    char Version[16];
} svg_header;

typedef union body
{
    shape Components[6];
    struct
    {
        shape RightLeg;
        shape LeftLeg;
        shape RightArm;
        shape LeftArm;
        shape Torso;
        shape Head;
    };
} body;

typedef struct game_dynamics
{
    bool PlatformAllow;
    bool FreeFalling;
    bool Ascending;
    bool Collision;
} game_dynamics;

typedef struct entity entity;

struct entity_header
{
    int Id;
    entity_type Type;
    float Height;
    v3f Origin;
    uint State;
};

typedef struct entity_background
{
    entity_header *Header;
    
    v3f Origin;
    shape Shape;
    bases Bases;
} entity_background;

typedef struct entity_static
{
    entity_header *Header;
    
    v3f Origin;
    shape Shape;
    shape CollisionGeometry;
    
} entity_static;

typedef struct entity_bullet
{
    entity_header *Header;
    
    v3f Position;
    float VelocityMagnitude;
    shape Shape;
    bases Bases;
    
    entity_type CastingEntityType;
} entity_bullet;

struct entity_enemy
{
    entity_header *Header;
    
    v3f Velocity;
    v3f Position;
    float VelocityMagnitude;
    float SpinMagnitude;
    
    float ShotFrequency;
    int CyclesToShoot;
    
    uint CyclesToChangeWalkingDirection;
    uint CountToChangeWalkingDirection;
    
    int CyclesToChangeFootDirection;
    bool Jumping;
    game_dynamics Dynamics;
    
    body Body;
    bases Bases;
    transform Transform;
    
    v3f Acceleration;
    float ShotVelocity;
    v3f RightArmDir;
    
    float ArmHeight;
};

struct entity_player
{
    entity_header *Header;
    
    v3f Velocity;
    v3f Position;
    float VelocityMagnitude;
    float SpinMagnitude;
    int CyclesToChangeFootDirection;
    bool Jumping;
    body Body;
    game_dynamics Dynamics;
    bases Bases;
    transform Transform;
    
    v3f Acceleration;
    float ShotVelocity;
    v3f RightArmDir;
    
    float ArmHeight;
};

struct entity
{
    entity_header Header;
    
    union
    {
        entity_enemy Enemy;
        entity_player Player;
        entity_bullet Bullet;
        entity_static Static;
        entity_background Background;
    };
};

struct entity_list
{
    entity *Content;
    entity_list *Next;
    
    entity_list()
    {
        Content = 0;
        Next = 0;
    }
    
    ~entity_list() {};
};

typedef struct arena
{
    char AppName[128];
    file_type Type;
    char FilePath[128];
    svg_header SVGFile;
    entity_background *Background;
    entity *CenterLimit;
    
} arena;

typedef struct mouse
{
    int ButtonState[3];
    bool IsHovering;
    v3f Position;
} mouse;

typedef struct input
{
    mouse Mouse;
    bool Keyboard[256];
} input;

struct timing
{
    float deltaTimeMs;
    float deltaTime;
    float FramesPerSecond;
};

struct light
{
    v3f Origin;
    v3f Direction;
    bool IsLightOn;
};

struct camera
{
    v3f P;
    v3f Up;
    float Sensitivity;
    camera_type Type;
};

typedef struct game
{
    arena Arena;
    entity *Player;
    timing Timing;
    input Input;
    camera Camera;
    
    int FramesPerSecond;
    int EntityCount;
    entity *Entities;
    
    int EnemyCount;
    entity_enemy *Enemies;
    
    float ShotVelocity;
    float PlayerVelocity;
    
    float EnemyShotFrequency;
    float EnemyCountToShoot;
    float EnemyShotVelocity;
    float EnemyVelocity;
    
    float ObstaclesHeight;
} game;

float JumpHeight = 40.0f;
float HeightWhenPressedJumpButton = 0.0;

uint WindowWidth = 600;
uint WindowHeight = 600;
game Game = {};

uint GlobalBulletsCount;
entity *GlobalBullets;
entity *SaveGlobalBullets;

bool ShotFired = false;

material StandardMaterial;
texture FloorTexture;
texture BulletTexture;
texture FloorNormalTexture;
texture WallTexture;

bool GameEndedWithVictory = false;
bool GameEndedWithFailure = false;

bool GlobalAllowPerspCameraToMove = false;
bool CameraPerspMoved = false;
float GlobalLastXCoordinate = 0;
float GlobalLastYCoordinate = 0;
v2f DistTraveledLastFrame = {};
float CameraPerspectiveTheta = -4.696;
float CameraPerspectivePhi = -0.900;

bool ToFirstPersonCamTransition = false;
bool ToThirdPersonCamTransition = false;


#endif