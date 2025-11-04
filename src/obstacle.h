#include <math.h>
#include <raylib.h>
#include <stdio.h>
typedef enum { CUBE, SPHERE, WALL } ObstacleType;
#define VEC_ONE ((Vector3){1, 1, 1})

typedef struct {
  int id;
  Texture2D texture;
  Rectangle source;
} AppearenceData;

#define APPNUM 10

typedef struct {
  int capacity;
  int count;
  AppearenceData items[APPNUM];
} AppearenceDA;

typedef struct {
  int id;
} Appearence;

typedef struct {
  ObstacleType type;
  Vector3 position;
  Vector3
      size; // for Cube - all matters, for Sphere only x (defines radius)
            // for Wall - one is left bottom position, the other is right top.
  Appearence appearence;
  BoundingBox collision;
  bool cute;
  bool zblock;
} Obstacle;

typedef struct {
  int count;
  int capacity;
  Obstacle obstacles[OBSTACLES_NUM];
} Stage; // it can be dynamic but ill use static probably
/*
Stage stage = {OBSTACLES_NUM,
               OBSTACLES_NUM,
               {//(Obstacle){CUBE, (Vector3){1, 1, 1}, (Vector3){1, 1, 1},
                //(Appearence){1}},
                //  (Obstacle){CUBE, (Vector3){5, 0, 5}, (Vector3){5, 5, 5},
                //  (Appearence){0}},
                (Obstacle){WALL, (Vector3){10, 10, 0}, (Vector3){0, 0, 0},
                           (Appearence){0}, (BoundingBox){0}, true},
                (Obstacle){WALL, (Vector3){0, 10, -10}, (Vector3){0, 0, 0},
                           (Appearence){0}, (BoundingBox){0}, true}}};
*/
AppearenceDA appearences = {APPNUM, APPNUM, {(AppearenceData){0}}};

void DrawPlaneTextureRec(Texture2D texture, Rectangle source,
                         Vector3 leftBottom, Vector3 rightTop, Color color);
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
  Vector3 min = Vec3Sub(camera.position, Vec3Div(VEC_ONE, 4));
  Vector3 max = Vec3Add(camera.position, Vec3Div(VEC_ONE, 4));
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
float Vec3Len(Vector3 v) { return sqrt(v.x * v.x + v.y * v.y + v.z * v.z); }
Vector3 Vec3Norm(Vector3 v) {
  // Vector3 norm = {0};
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

void VecPrint(Vector3 v) { printf("%f %f %f", v.x, v.y, v.z); }
Vector3 RestrictMovement(Camera camera, BoundingBox boundary, Obstacle obstacle,
                         bool zblock) {
  // basic for wall
  if (zblock) {
    // z blocking
    if (camera.position.z > obstacle.position.z) {
      camera.position.z = maxf(camera.position.z, boundary.max.z);
    } else {
      camera.position.z = minf(camera.position.z, boundary.min.z);
    }
  } else {
    // x blocking
    if (camera.position.x > obstacle.position.x) {
      camera.position.x = maxf(camera.position.x, boundary.max.x);
    } else {
      camera.position.x = minf(camera.position.x, boundary.min.x);
    }
  }
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
    switch (obstacle.type) {
    case CUBE:
      // if (CheckCollisionBoxes(GetObstacleBox(scaled), GetCamBox(*camera))) {

      //   DrawFPS(10, 10);
      //   camera->position = RestrictMovement(*camera, obstacleBox, scaled);
      // }

      break;
    case SPHERE:
      // TODO: spheres
      break;
    case WALL:
      if (CheckCollisionBoxes(obstacle.collision, GetCamBox(*camera))) {

        // DrawFPS(10, 10);
        camera->position = RestrictMovement(*camera, obstacle.collision,
                                            obstacle, obstacle.zblock);
      }
      break;
    }
  }
  return res;
}

void setupCollision(Stage *stage) {

  for (int i = 0; i < stage->count; ++i) {
    Obstacle obstacle = stage->obstacles[i];
    // Obstacle scaled = obstacle;

    // scaled.size = Vec3Mul(scaled.size, 1.1);
    //  basic for wall
    float dx = absf(obstacle.position.x - obstacle.size.x);
    float dz = absf(obstacle.position.z - obstacle.size.z);
    float minx = minf(obstacle.position.x, obstacle.size.x);
    float minz = minf(obstacle.position.z, obstacle.size.z);
    float maxx = maxf(obstacle.position.x, obstacle.size.x);
    float maxz = maxf(obstacle.position.z, obstacle.size.z);
    float miny = minf(obstacle.position.y, obstacle.size.y);
    float maxy = maxf(obstacle.position.y, obstacle.size.y);
    bool zblock = dx > dz;
    if (zblock) {
      // z blocking
      // float idz = minz;
      minz -= 0.5;
      maxz += 0.5;
    } else {
      // x blocking
      // float midx = minx;
      minx -= 0.5;
      maxx += 0.5;
    }
    BoundingBox obstacleBox = {(Vector3){minx, miny, minz},
                               (Vector3){maxx, maxy, maxz}};
    stage->obstacles[i].collision = obstacleBox;
    stage->obstacles[i].zblock = zblock;
  }
}

void DrawObstacle(Obstacle obstacle) {
  int id = obstacle.appearence.id;
  switch (obstacle.type) {
  case CUBE:
    DrawCubeV(obstacle.position, obstacle.size, WHITE);
    break;
  case SPHERE:
    DrawSphere(obstacle.position, obstacle.size.x, WHITE);
    break;
  case WALL:
    DrawPlaneTextureRec(
        appearences.items[id].texture,
        /*appearences.items[id].source*/ (Rectangle){0, 0, 512, 512},
        obstacle.position, obstacle.size, WHITE);
    // DrawPlaneTextureRec(appearences.items[id].texture,
    //                     appearences.items[id].source, obstacle.size,
    //                     obstacle.position, WHITE);
  }
}
bool isInsideRec(Vector2 v, Rectangle rec) {
  return ((v.y < rec.y + rec.height) && (v.y > rec.y) &&
          (v.x < rec.x + rec.width) && (v.x > rec.x));
}
void DrawObstacles(Stage stage, Camera camera) {
  for (int i = 0; i < stage.count; ++i) {
    DrawObstacle(stage.obstacles[i]);

    if (stage.obstacles[i].cute) {
      Vector3 pos = stage.obstacles[i].position;
      pos.x += 2.5;
      pos.z += 2.5;
      pos.y = 2;
      DrawBillboard(camera, appearences.items[2].texture, pos, 4.0f, WHITE);
    }
  }
}
Vector3 Vec3Approach(Vector3 source, Vector3 target, float speed) {
  return Vec3Add(source, Vec3Mul(Vec3Sub(target, source), speed));
};
void InitPlayerAndGhost(Camera *camera, Vector3 *ghost) {

  camera->position = (Vector3){10.0f, 2.0f, 10.0f}; // Camera position
  camera->target = (Vector3){0.0f, 2.0f, 0.0f};     // Camera looking at point
  camera->up =
      (Vector3){0.0f, 1.0f, 0.0f}; // Camera up vector (rotation towards target)
  camera->fovy = 60.0f;            // Camera field-of-view Y
  camera->projection = CAMERA_PERSPECTIVE; // Camera projection type
  ghost->x = -2;
  ghost->y = 2;
  ghost->z = -2;
}
