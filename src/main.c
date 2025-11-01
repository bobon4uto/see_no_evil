#include <complex.h>
#include <math.h>
#include <raylib.h>
#include <stdio.h>
#define VEC_ONE ((Vector3){1, 1, 1})
// TRY WALL should be aesier
typedef enum { CUBE, SPHERE } ObstacleType;

typedef struct {
  int sm;
} Appearence;

typedef struct {
  ObstacleType type;
  Vector3 position;
  Vector3 size; // for Cube - all matters, for Sphere only x (defines radius)
  Appearence appearence;
} Obstacle;

#define OBSTACLES_NUM 3

typedef struct {
  int count;
  int capacity;
  Obstacle obstacles[OBSTACLES_NUM];
} Stage; // it can be dynamic but ill use static probably

Stage stage = {
    OBSTACLES_NUM,
    OBSTACLES_NUM,
    {(Obstacle){CUBE, (Vector3){1, 1, 1}, (Vector3){1, 1, 1}, (Appearence){1}},
     (Obstacle){CUBE, (Vector3){5, 0, 5}, (Vector3){5, 5, 5},
                (Appearence){1}}}};

Vector3 Vec3Add(Vector3 a, Vector3 b) {

  return (Vector3){a.x + b.x, a.y + b.y, a.z + b.z};
}
Vector3 Vec3Sub(Vector3 a, Vector3 b) {

  return (Vector3){a.x - b.x, a.y - b.y, a.z - b.z};
}
Vector3 Vec3Mul(Vector3 a, float b) {

  return (Vector3){a.x * b, a.y * b, a.z * b};
}
Vector3 Vec3Div(Vector3 a, float b) {

  return (Vector3){a.x / b, a.y / b, a.z / b};
}

BoundingBox GetObstacleBox(Obstacle obstacle) {

  Vector3 min = Vec3Sub(obstacle.position, Vec3Div(obstacle.size, 2));
  Vector3 max = Vec3Add(obstacle.position, Vec3Div(obstacle.size, 2));
  return (BoundingBox){min, max};
}
BoundingBox GetCamBox(Camera camera) {
  Vector3 min = Vec3Sub(camera.position, Vec3Div(VEC_ONE, 2));
  Vector3 max = Vec3Add(camera.position, Vec3Div(VEC_ONE, 2));
  return (BoundingBox){min, max};
}

float absf(float a) {
  if (a < 0)
    return -a;
  else
    return a;
}
float minf(float a, float b) {
  if (a < b)
    return a;
  else
    return b;
}
float maxf(float a, float b) {
  if (a > b)
    return a;
  else
    return b;
}

Vector3 Vec3Norm(Vector3 v) {
  Vector3 norm = {0};
  float len = sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
  return (Vector3){v.x / len, v.y / len, v.z / len};
};

Vector3 Vec3ToXYPlane(Vector3 v) { return (Vector3){v.x, v.y, 0}; }

Vector3 Vec3MovToLook(Vector3 l, Vector3 m) {
  Vector3 mulx = Vec3Mul(l, m.x);
  Vector3 muly = Vec3Mul(l, m.y);
  Vector3 sc = {0};
  sc.x = muly.y;
  sc.y = muly.x;
  return Vec3Add(mulx, sc);
};
float Vec3Dot(Vector3 a, Vector3 b) {
  return a.x * b.x + a.y * b.y + a.z * b.z;
}
Vector3 Vec3LookToMov(Vector3 l, Vector3 look) {

  Vector3 res = {0};
  Vector3 sc = {0};
  sc.x = l.y;
  sc.y = l.x;

  res.x = Vec3Dot(look, l);
  res.y = Vec3Dot(look, sc);

  return res;
};

bool EqEp(float a, float b, float e) {
  if (a + e > b && a - e < b)
    return true;
  return false;
}

bool Vec3Eq(Vector3 a, Vector3 b, float epsilon) {
  if (!EqEp(a.x, b.x, epsilon))
    return false;
  if (!EqEp(a.y, b.y, epsilon))
    return false;
  if (!EqEp(a.z, b.z, epsilon))
    return false;

  return true;
}

void VecPrint(Vector3 v) { printf("%f %f %f", v.z, v.y, v.z); }
Vector3 RestrictMovement(Camera camera, Vector3 movement, Obstacle obstacle) {
  // basic for wall
  BoundingBox obstacleBox = GetObstacleBox(obstacle);
  camera.position.x = maxf(camera.position.x, obstacleBox.max.x);
  return camera.position;
}
Vector3 CalcMovmentWithCollision(Camera *camera, Stage stage) {
  Vector3 res = {
      ((IsKeyDown(KEY_W) || IsKeyDown(KEY_UP))) *
              0.1f - // Move forward-backward
          (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) * 0.1f,
      (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) * 0.1f - // Move right-left
          (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) * 0.1f,
      0.0f // Move up-down
  };
  for (int i = 0; i < stage.count; ++i) {
    Obstacle obstacle = stage.obstacles[i];
    Obstacle scaled = obstacle;
    scaled.size = Vec3Mul(scaled.size, 1.1);
    switch (obstacle.type) {
    case CUBE:
      if (CheckCollisionBoxes(GetObstacleBox(scaled), GetCamBox(*camera))) {

        DrawFPS(10, 10);
        camera->position = RestrictMovement(*camera, res, scaled);
      }

      break;
    case SPHERE:
      // TODO: spheres
      break;
    }
  }
  return res;
}
void DrawObstacle(Obstacle obstacle) {
  switch (obstacle.type) {
  case CUBE:
    DrawCubeV(obstacle.position, obstacle.size, WHITE);
    break;
  case SPHERE:
    DrawSphere(obstacle.position, obstacle.size.x, WHITE);
    break;
  }
}
void DrawObstacles(Stage stage) {
  for (int i = 0; i < stage.count; ++i) {
    DrawObstacle(stage.obstacles[i]);
  }
}

int main(int argc, char **argv) {

  const int screenWidth = 800;
  const int screenHeight = 450;

  InitWindow(screenWidth, screenHeight, "see_no_evil");
  DisableCursor(); // Limit cursor to relative movement inside the window

  SetTargetFPS(60); // Set our game to run at 60 frames-per-second

#ifndef PLATFORM_ANDROID
  ChangeDirectory("assets");
#endif
//  Image don_img_move = LoadImage(
//      "characters/DonQuixote/LCB/10301_Donquixote_BaseAppearance/move.png");
#ifndef PLATFORM_ANDROID
  ChangeDirectory("..");
#endif

  // Images to textures
  //  Texture2D don_tex = LoadTextureFromImage(don_img);
  //  UnloadImage(don_img);
  //  Texture2D don_tex_move = LoadTextureFromImage(don_img_move);
  //  UnloadImage(don_img_move);
  //
  Camera camera = {0};
  camera.position = (Vector3){0.0f, 2.0f, 4.0f}; // Camera position
  camera.target = (Vector3){0.0f, 2.0f, 0.0f};   // Camera looking at point
  camera.up =
      (Vector3){0.0f, 1.0f, 0.0f}; // Camera up vector (rotation towards target)
  camera.fovy = 60.0f;             // Camera field-of-view Y
  camera.projection = CAMERA_PERSPECTIVE; // Camera projection type
  int cameraMode = CAMERA_FIRST_PERSON;

  while (!WindowShouldClose()) // Detect window close button or ESC key
  {
    UpdateCameraPro(&camera, CalcMovmentWithCollision(&camera, stage),
                    (Vector3){
                        GetMouseDelta().x * 0.05f, // Rotation: yaw
                        GetMouseDelta().y * 0.05f, // Rotation: pitch
                        0.0f                       // Rotation: roll
                    },
                    GetMouseWheelMove() * 2.0f); // Move to target (zoom)

    BeginDrawing();

    ClearBackground(BLACK);

    BeginMode3D(camera);

    DrawPlane((Vector3){0.0f, 0.0f, 0.0f}, (Vector2){32.0f, 32.0f},
              LIGHTGRAY); // Draw ground
    DrawCube((Vector3){-16.0f, 2.5f, 0.0f}, 1.0f, 5.0f, 32.0f,
             BLUE); // Draw a blue wall
    DrawCube((Vector3){16.0f, 2.5f, 0.0f}, 1.0f, 5.0f, 32.0f,
             LIME); // Draw a green wall
    DrawCube((Vector3){0.0f, 2.5f, 16.0f}, 32.0f, 5.0f, 1.0f,
             GOLD); // Draw a yellow wall
    DrawObstacles(stage);
    EndMode3D();

    EndDrawing();
  }

  CloseWindow();

  return 0;
}
