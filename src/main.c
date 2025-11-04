#include <assert.h>
#include <complex.h>
#include <math.h>
#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>
#include <stdio.h>
#include <sys/types.h>
// TRY WALL should be aesier
#define OBSTACLES_NUM (32 * 32)
#include "obstacle.h"
#include "stage.h"
#define STAGE_WALL_HEIGHT 5
#define STAGE_WALL_LENGHT 5
Stage stage = {OBSTACLES_NUM,
               OBSTACLES_NUM,
               {
                   {0}
                   /*  (Obstacle){
                         WALL,
                         (Vector3){10, i++, 0},
                         (Vector3){0, 0, 0},
                     },
                     (Obstacle){
                         WALL,
                         (Vector3){0, 10, -10},
                         (Vector3){0, 0, 0},
                     }
                                                                     */
               }};

void init_stage(Stage *stage) {
  for (int i = 0; i < stage->count; ++i) {
    float wall_height = 0;
    if (stage_indexes_truth[i]) {
      wall_height = STAGE_WALL_HEIGHT;
    }

    Obstacle *obstacle = &(stage->obstacles[i]);
    obstacle->position = (Vector3){((i / 2) % 32) * STAGE_WALL_LENGHT, 0,
                                   ((i / 2) / 32) * STAGE_WALL_LENGHT};
    if (i % 2 == 0) {
      // even
      obstacle->size = Vec3Add(obstacle->position,
                               (Vector3){STAGE_WALL_LENGHT, wall_height, 0});

    } else {
      // odd
      obstacle->size = Vec3Add(obstacle->position,
                               (Vector3){0, wall_height, STAGE_WALL_LENGHT});
    }
    obstacle->cute = stage_indexes_cute[i];
    obstacle->type = WALL;
    obstacle->appearence.id = 1;
  }
}
void DrawPlaneTexture4points(Texture2D texture, Rectangle source,
                             Vector3 leftTop, Vector3 rightTop,
                             Vector3 leftBottom, Vector3 rightBottom,
                             Color color);
void printStage(Stage stage) {
  for (int i = 0; i < stage.count; ++i) {
    printf("\nWALL%d = \n", i);
    VecPrint(stage.obstacles[i].position);
    printf("\n");
    VecPrint(stage.obstacles[i].size);
  }
}
int main(int argc, char **argv) {
  printf("%s%d\n", argv[0], argc);
  const int screenWidth = 800;
  const int screenHeight = 450;
  const int gameScreenWidth = 800;
  const int gameScreenHeight = 450;

  InitWindow(screenWidth, screenHeight, "see_no_evil");
  InitAudioDevice();
  DisableCursor(); // Limit cursor to relative movement inside the window

  SetTargetFPS(60); // Set our game to run at 60 frames-per-second

  RenderTexture2D target = LoadRenderTexture(gameScreenWidth, gameScreenHeight);
  SetTextureFilter(target.texture, TEXTURE_FILTER_BILINEAR);
  RenderTexture2D target2 =
      LoadRenderTexture(gameScreenWidth, gameScreenHeight);
  SetTextureFilter(target2.texture, TEXTURE_FILTER_BILINEAR);

  init_stage(&stage);
#ifndef PLATFORM_ANDROID
  ChangeDirectory("assets");
#endif
  for (int i = 0; i < appearences.count; ++i) {
    appearences.items[i].id = i;

    appearences.items[i].source = (Rectangle){0, 0, 16, 16};
    Image block_img;

    if (i == 1) {
      block_img =
          LoadImage("wall.png"); // make it so i can use more than one textue

      appearences.items[i].source = (Rectangle){0, 0, 512, 512};
    } else if (i == 2) {
      block_img = LoadImage(
          "anatomically_inaccurate_skeleton.png"); // make it so i can use
                                                   // more than one textue

    } else if (i == 3) {
      block_img = LoadImage("finish.png"); // make it so i can use
                                           // more than one textue
    } else {

      block_img =
          LoadImage("blk.png"); // make it so i can use more than one textue
    }
    Texture2D block_tex = LoadTextureFromImage(block_img);
    UnloadImage(block_img);
    appearences.items[i].texture = block_tex;
  }
  /*Image block_img =
      LoadImage("blk.png"); // make it so i can use more than one textue
  Texture2D block_tex = LoadTextureFromImage(block_img);
  UnloadImage(block_img);
  appearences.items[0].texture = block_tex;
  Image wall_img =
      LoadImage("wall.png"); // make it so i can use more than one textue
  Texture2D wall_tex = LoadTextureFromImage(block_img);
  UnloadImage(block_img);
  appearences.items[1].texture = wall_tex;
*/
  Image arrow_img = LoadImage("arrow.png");
  Texture2D arrow_tex = LoadTextureFromImage(arrow_img);
  UnloadImage(arrow_img);
  Rectangle arrow_source = (Rectangle){0, 0, arrow_tex.width, arrow_tex.height};
  Rectangle arrow_dst =
      (Rectangle){25, gameScreenHeight - arrow_source.height / 15,
                  arrow_source.width / 15, arrow_source.height / 15};
  Vector2 arrow_orogin = {arrow_dst.width / 2, arrow_dst.height * 2};

  float arrow_rotation = 90.0f;

  Image meter_img = LoadImage("meter.png");

  Texture2D meter_tex = LoadTextureFromImage(meter_img);
  UnloadImage(meter_img);
  Rectangle meter_source = (Rectangle){0, 0, meter_tex.width, meter_tex.height};
  Rectangle meter_dst =
      (Rectangle){0, gameScreenHeight - meter_source.height / 5,
                  meter_source.width / 5, meter_source.height / 5};
  Vector2 meter_orogin = {0, 0};
  float meter_rotation = 0.0f;

  Sound ghostbeat = LoadSound("beat.mp3"); // https://www.bfxr.net/

  Music music =
      LoadMusicStream("Unease.wav"); // https://comigo.itch.io/music-loops

  PlayMusicStream(music);
#ifndef PLATFORM_ANDROID
  ChangeDirectory("..");
#endif

  // Images to textures
  //
  Camera camera = {0};
  camera.position = (Vector3){10.0f, 2.0f, 10.0f}; // Camera position
  camera.target = (Vector3){0.0f, 2.0f, 0.0f};     // Camera looking at point
  camera.up =
      (Vector3){0.0f, 1.0f, 0.0f}; // Camera up vector (rotation towards target)
  camera.fovy = 60.0f;             // Camera field-of-view Y
  camera.projection = CAMERA_PERSPECTIVE; // Camera projection type
  // int cameraMode = CAMERA_FIRST_PERSON;
  setupCollision(&stage);

  Vector3 bill = {-2, 2, -2};
  float ghost_distance = 100;
  bool died = false;
  bool won = false;
  Rectangle retryButton = {80, 300, 500, 100};
  InitPlayerAndGhost(&camera, &bill);

  float ghosttimer = 0.0f;

  Vector3 finish = {145, 2, 30};
  // Sound sound = LoadSound("assets/beat.mp3");
  PlayMusicStream(music);

  Color tintedBlack = {0x00, 0x00, 0x00, 0x00};
  while (!WindowShouldClose()) // Detect window close button or ESC key
  {

    float scale = maxf((float)GetScreenWidth() / gameScreenWidth,
                       (float)GetScreenHeight() / gameScreenHeight);

    UpdateMusicStream(music); // Update music buffer with new stream data
                              // printStage(stage);

    if (IsKeyDown(KEY_V)) {
      arrow_rotation -= 1.0f;
    }
    if (IsKeyDown(KEY_SPACE)) {
      arrow_rotation += 1.0f;
    }
    UpdateCameraPro(&camera, CalcMovmentWithCollision(&camera, stage),
                    (Vector3){
                        GetMouseDelta().x * 0.05f, // Rotation: yaw
                        GetMouseDelta().y * 0.05f, // Rotation: pitch
                        0.0f                       // Rotation: roll
                    },
                    0.0f); // Move to target (zoom)

    bill = Vec3Approach(bill, camera.position, 0.01);
    ghost_distance = Vec3Len(Vec3Sub(bill, camera.position));
    if (Vec3Len(Vec3Sub(finish, camera.position)) < 1.0f) {
      died = true;
      won = true;
      continue;
    };
    if (ghost_distance < 10.0f) {
      tintedBlack.a = (u_int)(255.0f / ghost_distance);
      arrow_rotation = 9.0f * ghost_distance;
    } else {
      tintedBlack.a = 0x01;
      arrow_rotation = 90.0f;
    }
    // SetMusicVolume(music, 3.0f / ghost_distance);
    SetMusicVolume(music, 0.0f);
    ghosttimer -= 0.1f;
    if (ghost_distance < 1.0) {
      died = true;
    }
    if (died) {
      SetMusicVolume(music, 0.0f);
      if (IsCursorHidden()) {
        EnableCursor(); // Limit cursor to relative movement inside the window
      }
      Vector2 mouse = GetMousePosition();
      Vector2 virtualMouse = {0};
      virtualMouse.x =
          (mouse.x - (GetScreenWidth() - (gameScreenWidth * scale)) * 0.5f) /
          scale;
      virtualMouse.y =
          (mouse.y - (GetScreenHeight() - (gameScreenHeight * scale)) * 0.5f) /
          scale;
      virtualMouse = Vector2Clamp(
          virtualMouse, (Vector2){0, 0},
          (Vector2){(float)gameScreenWidth, (float)gameScreenHeight});
      if ((isInsideRec(virtualMouse, retryButton) &&
           IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) ||
          IsKeyPressed(KEY_ENTER)) {
        died = false;
        won = false;
        DisableCursor(); // Limit cursor to relative movement inside the
                         // window
        InitPlayerAndGhost(&camera, &bill);
      }
      BeginTextureMode(target);
      ClearBackground(BLACK);
      if (won) {

        DrawText("YOU WON!", 100, 100, 100, GREEN);
        retryButton.width = 800;
        DrawRectangleRec(retryButton, BLUE);
        DrawText("RESTART?", 100, 300, 100, RED);
      } else {
        retryButton.width = 500;
        DrawText("YOU DIED", 100, 100, 100, RED);
        DrawRectangleRec(retryButton, GREEN);
        DrawText("RETRY?", 100, 300, 100, BLUE);
      }
      EndTextureMode();

      BeginDrawing();
      DrawTexturePro(
          target.texture,
          (Rectangle){0.0f, 0.0f, (float)target.texture.width,
                      (float)-target.texture.height},
          (Rectangle){
              (GetScreenWidth() - ((float)gameScreenWidth * scale)) * 0.5f,
              (GetScreenHeight() - ((float)gameScreenHeight * scale)) * 0.5f,
              (float)gameScreenWidth * scale, (float)gameScreenHeight * scale},
          (Vector2){0, 0}, 0.0f, WHITE);

      EndDrawing();

      continue;
    }
    if (ghosttimer < 0.0f) {
      PlaySound(ghostbeat);
      ghosttimer = (ghost_distance + ((float)GetRandomValue(-10, 10)) / 10) / 5;
    }
    BeginTextureMode(target2);
    DrawTexturePro(meter_tex, meter_source, meter_dst, meter_orogin,
                   meter_rotation, DARKGRAY);
    DrawTexturePro(arrow_tex, arrow_source, arrow_dst, arrow_orogin,
                   arrow_rotation, DARKGRAY);
    DrawRectangle(0, 0, gameScreenWidth, gameScreenHeight, tintedBlack);
    // DrawText(TextFormat("%d FPS", GetFPS()), 10, 10, 50, RED);
    // DrawText(TextFormat("%.2f %.2f", camera.position.x, camera.position.z),
    // 10,
    //         10, 50, RED);
    EndTextureMode();
    BeginDrawing();

    ClearBackground(BLACK);

    BeginMode3D(camera);

    DrawPlane(
        (Vector3){0.0f, 0.0f, 0.0f}, (Vector2){512.0f, 512.0f},
        (Color){.r = 0x11, .g = 0x22, .b = 0x33, .a = 0xFF}); // Draw ground
    DrawPlaneTexture4points(appearences.items[1].texture,
                            (Rectangle){0, 0, 512, 512}, (Vector3){-5, 5, -5},
                            (Vector3){-5, 5, 150}, (Vector3){150, 5, 150},
                            (Vector3){150, 5, -5}, WHITE);
    DrawObstacles(stage, camera);
    DrawBillboard(camera, appearences.items[3].texture, finish, 4.0f, WHITE);
    //    DrawPlaneTextureRec(appearences.items[0].texture,
    //                        (Rectangle){0, 0, 512, 512}, (Vector3){0, 0, 0},
    //                        (Vector3){1, 1, 1}, WHITE);

    // DrawBillboard(camera, appearences.items[1].texture, bill, 1, WHITE);
    EndMode3D();
    DrawTexturePro(
        target2.texture,
        (Rectangle){0.0f, 0.0f, (float)target2.texture.width,
                    (float)-target2.texture.height},
        (Rectangle){
            (GetScreenWidth() - ((float)gameScreenWidth * scale)) * 0.5f,
            (GetScreenHeight() - ((float)gameScreenHeight * scale)) * 0.5f,
            (float)gameScreenWidth * scale, (float)gameScreenHeight * scale},
        (Vector2){0, 0}, 0.0f, WHITE);

    EndDrawing();
  }

  UnloadMusicStream(music); // Unload music stream buffers from RAM
  CloseAudioDevice();
  CloseWindow();

  return 0;
}
void rlVertexv(Vector3 v) { rlVertex3f(v.x, v.y, v.z); };
void DrawPlaneTexture4points(Texture2D texture, Rectangle source,
                             Vector3 leftTop, Vector3 rightTop,
                             Vector3 leftBottom, Vector3 rightBottom,
                             Color color) {

  float texWidth = (float)texture.width;
  float texHeight = (float)texture.height;
  // Set desired texture to be enabled while drawing following vertex data
  rlSetTexture(texture.id);

  // We calculate the normalized texture coordinates for the desired
  // texture-source-rectangle It means converting from (tex.width, tex.height)
  // coordinates to [0.0f, 1.0f] equivalent
  //
  rlBegin(RL_QUADS);
  rlColor4ub(color.r, color.g, color.b, color.a);

  // Front face
  rlNormal3f(0.0f, 1.0f, 0.0f);
  rlTexCoord2f((source.x + source.width) / texWidth,
               (source.y + source.height) / texHeight);
  rlVertexv(rightBottom);
  rlTexCoord2f(source.x / texWidth, (source.y + source.height) / texHeight);
  rlVertexv(leftBottom);
  rlTexCoord2f((source.x + source.width) / texWidth, source.y / texHeight);
  rlVertexv(rightTop);
  rlTexCoord2f(source.x / texWidth, source.y / texHeight);
  rlVertexv(leftTop);

  rlEnd();
  rlSetTexture(0);
}

void DrawPlaneTextureRec(Texture2D texture, Rectangle source,
                         Vector3 leftBottom, Vector3 rightTop, Color color) {

  float texWidth = (float)texture.width;
  float texHeight = (float)texture.height;
  // Set desired texture to be enabled while drawing following vertex data
  rlSetTexture(texture.id);

  // We calculate the normalized texture coordinates for the desired
  // texture-source-rectangle It means converting from (tex.width, tex.height)
  // coordinates to [0.0f, 1.0f] equivalent
  //
  rlBegin(RL_QUADS);
  rlColor4ub(color.r, color.g, color.b, color.a);

  Vector3 horizontal = Vec3Sub(rightTop, leftBottom);

  // Front face
  rlNormal3f(horizontal.z, 0.0f, horizontal.x);
  rlTexCoord2f(source.x / texWidth, (source.y + source.height) / texHeight);
  rlVertex3f(leftBottom.x, leftBottom.y, leftBottom.z);
  rlTexCoord2f((source.x + source.width) / texWidth,
               (source.y + source.height) / texHeight);
  rlVertex3f(rightTop.x, leftBottom.y, rightTop.z);
  rlTexCoord2f((source.x + source.width) / texWidth, source.y / texHeight);
  rlVertex3f(rightTop.x, rightTop.y, rightTop.z);
  rlTexCoord2f(source.x / texWidth, source.y / texHeight);
  rlVertex3f(leftBottom.x, rightTop.y, leftBottom.z);

  // Back face
  rlNormal3f(-horizontal.z, 0.0f, -horizontal.x);
  rlTexCoord2f((source.x + source.width) / texWidth, source.y / texHeight);
  rlVertex3f(rightTop.x, rightTop.y, rightTop.z);

  rlTexCoord2f((source.x + source.width) / texWidth,
               (source.y + source.height) / texHeight);
  rlVertex3f(rightTop.x, leftBottom.y, rightTop.z);

  rlTexCoord2f(source.x / texWidth, (source.y + source.height) / texHeight);
  rlVertex3f(leftBottom.x, leftBottom.y, leftBottom.z);

  rlTexCoord2f(source.x / texWidth, source.y / texHeight);
  rlVertex3f(leftBottom.x, rightTop.y, leftBottom.z);

  rlEnd();
  rlSetTexture(0);
}

///
///
///
////
///
///
/// From Raylib examplesA
#if NOTTRUE
// this looks like opengl bru whyyyy
void DrawCubeTextureRec(Texture2D texture, Rectangle source, Vector3 position,
                        float width, float height, float length, Color color) {
  float x = position.x;
  float y = position.y;
  float z = position.z;
  float texWidth = (float)texture.width;
  float texHeight = (float)texture.height;

  // Set desired texture to be enabled while drawing following vertex data
  rlSetTexture(texture.id);

  // We calculate the normalized texture coordinates for the desired
  // texture-source-rectangle It means converting from (tex.width, tex.height)
  // coordinates to [0.0f, 1.0f] equivalent
  //
  rlBegin(RL_QUADS);
  rlColor4ub(color.r, color.g, color.b, color.a);

  // Front face
  rlNormal3f(0.0f, 0.0f, 1.0f);
  rlTexCoord2f(source.x / texWidth, (source.y + source.height) / texHeight);
  rlVertex3f(x - width / 2, y - height / 2, z + length / 2);
  rlTexCoord2f((source.x + source.width) / texWidth,
               (source.y + source.height) / texHeight);
  rlVertex3f(x + width / 2, y - height / 2, z + length / 2);
  rlTexCoord2f((source.x + source.width) / texWidth, source.y / texHeight);
  rlVertex3f(x + width / 2, y + height / 2, z + length / 2);
  rlTexCoord2f(source.x / texWidth, source.y / texHeight);
  rlVertex3f(x - width / 2, y + height / 2, z + length / 2);

  // Back face
  rlNormal3f(0.0f, 0.0f, -1.0f);
  rlTexCoord2f((source.x + source.width) / texWidth,
               (source.y + source.height) / texHeight);
  rlVertex3f(x - width / 2, y - height / 2, z - length / 2);
  rlTexCoord2f((source.x + source.width) / texWidth, source.y / texHeight);
  rlVertex3f(x - width / 2, y + height / 2, z - length / 2);
  rlTexCoord2f(source.x / texWidth, source.y / texHeight);
  rlVertex3f(x + width / 2, y + height / 2, z - length / 2);
  rlTexCoord2f(source.x / texWidth, (source.y + source.height) / texHeight);
  rlVertex3f(x + width / 2, y - height / 2, z - length / 2);

  // Top face
  rlNormal3f(0.0f, 1.0f, 0.0f);
  rlTexCoord2f(source.x / texWidth, source.y / texHeight);
  rlVertex3f(x - width / 2, y + height / 2, z - length / 2);
  rlTexCoord2f(source.x / texWidth, (source.y + source.height) / texHeight);
  rlVertex3f(x - width / 2, y + height / 2, z + length / 2);
  rlTexCoord2f((source.x + source.width) / texWidth,
               (source.y + source.height) / texHeight);
  rlVertex3f(x + width / 2, y + height / 2, z + length / 2);
  rlTexCoord2f((source.x + source.width) / texWidth, source.y / texHeight);
  rlVertex3f(x + width / 2, y + height / 2, z - length / 2);

  // Bottom face
  rlNormal3f(0.0f, -1.0f, 0.0f);
  rlTexCoord2f((source.x + source.width) / texWidth, source.y / texHeight);
  rlVertex3f(x - width / 2, y - height / 2, z - length / 2);
  rlTexCoord2f(source.x / texWidth, source.y / texHeight);
  rlVertex3f(x + width / 2, y - height / 2, z - length / 2);
  rlTexCoord2f(source.x / texWidth, (source.y + source.height) / texHeight);
  rlVertex3f(x + width / 2, y - height / 2, z + length / 2);
  rlTexCoord2f((source.x + source.width) / texWidth,
               (source.y + source.height) / texHeight);
  rlVertex3f(x - width / 2, y - height / 2, z + length / 2);

  // Right face
  rlNormal3f(1.0f, 0.0f, 0.0f);
  rlTexCoord2f((source.x + source.width) / texWidth,
               (source.y + source.height) / texHeight);
  rlVertex3f(x + width / 2, y - height / 2, z - length / 2);
  rlTexCoord2f((source.x + source.width) / texWidth, source.y / texHeight);
  rlVertex3f(x + width / 2, y + height / 2, z - length / 2);
  rlTexCoord2f(source.x / texWidth, source.y / texHeight);
  rlVertex3f(x + width / 2, y + height / 2, z + length / 2);
  rlTexCoord2f(source.x / texWidth, (source.y + source.height) / texHeight);
  rlVertex3f(x + width / 2, y - height / 2, z + length / 2);

  // Left face
  rlNormal3f(-1.0f, 0.0f, 0.0f);
  rlTexCoord2f(source.x / texWidth, (source.y + source.height) / texHeight);
  rlVertex3f(x - width / 2, y - height / 2, z - length / 2);
  rlTexCoord2f((source.x + source.width) / texWidth,
               (source.y + source.height) / texHeight);
  rlVertex3f(x - width / 2, y - height / 2, z + length / 2);
  rlTexCoord2f((source.x + source.width) / texWidth, source.y / texHeight);
  rlVertex3f(x - width / 2, y + height / 2, z + length / 2);
  rlTexCoord2f(source.x / texWidth, source.y / texHeight);
  rlVertex3f(x - width / 2, y + height / 2, z - length / 2);

  rlEnd();

  rlSetTexture(0);
}
#endif
