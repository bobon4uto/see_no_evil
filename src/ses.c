// simply edit stage

#include <math.h>
#include <raylib.h>

#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "../nob.h"
#include <raymath.h>
typedef struct {
  Rectangle shape;
  bool active;
  bool cute;
} Wall;
#define WALLSNUM (32 * 32)
#define WALLEN 10.0
#define OBSTACLES_NUM (32 * 32)
#include "../src/stage.h"

void init_stage_carcas(Wall walls[WALLSNUM]) {
  for (int i = 0; i < WALLSNUM; ++i) {
    Wall *wall = &(walls[i]);
    wall->shape.x = ((i / 2) % 32) * WALLEN;
    wall->shape.y = ((i / 2) / 32) * WALLEN;
    if (i % 2 == 0) {
      // even
      wall->shape.width = WALLEN;
      wall->shape.height = WALLEN / 2;
    } else {
      // odd
      wall->shape.height = WALLEN;
      wall->shape.width = WALLEN / 2;
    }
  }
}
void getFromFile(Wall walls[WALLSNUM]) {
  for (int i = 0; i < WALLSNUM; ++i) {
    walls[i].active = stage_indexes_truth[i];
    walls[i].cute = stage_indexes_cute[i];
  }
}
void write_to_file(Wall walls[WALLSNUM]) {

  String_Builder sb = {0};
  String_Builder sb_cute = {0};
  sb_append_cstr(&sb, "bool stage_indexes_truth[] = {");
  sb_append_cstr(&sb_cute, "bool stage_indexes_cute[] = {");
  for (int i = 0; i < WALLSNUM - 1; ++i) {
    if (walls[i].active) {
      sb_append_cstr(&sb, "true,");
    } else {
      sb_append_cstr(&sb, "false,");
    }
    if (walls[i].cute) {
      sb_append_cstr(&sb_cute, "true,");
    } else {
      sb_append_cstr(&sb_cute, "false,");
    }
  }
  if (walls[WALLSNUM - 1].active) {
    sb_append_cstr(&sb, "true");
  } else {
    sb_append_cstr(&sb, "false");
  }
  if (walls[WALLSNUM - 1].cute) {
    sb_append_cstr(&sb_cute, "true");
  } else {
    sb_append_cstr(&sb_cute, "false");
  }

  sb_append_cstr(&sb, "};");
  sb_append_cstr(&sb_cute, "};");
  sb_append_null(&sb_cute);

  sb_append_cstr(&sb, sb_cute.items);
  nob_write_entire_file("src/stage.h", sb.items, sb.count);
};

int main() {
  int windowHeight = 800;
  int windowWidth = 800;
  InitWindow(windowWidth, windowHeight, "SimplyEditStage");
  ToggleFullscreen();
  SetTargetFPS(60);
  Wall walls[WALLSNUM] = {0};
  init_stage_carcas(walls);
  getFromFile(walls);
  Vector2 user = {0, 0};
  Camera2D camera = {0};
  camera.target = (Vector2){user.x, user.y};
  camera.offset = (Vector2){windowWidth / 2.0f, windowHeight / 2.0f};
  camera.rotation = 0.0f;
  camera.zoom = 1.0f;
  Color col = RED;
  int movespeed = 2;

  Vector2 mouseinworld = {0};

  while (!WindowShouldClose()) {
    camera.target = (Vector2){user.x, user.y};
    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
      Vector2 delta = GetMouseDelta();
      delta = Vector2Scale(delta, -1.0f / camera.zoom);
      user = Vector2Add(camera.target, delta);
    }

    if (IsKeyPressed(KEY_E)) {
      mouseinworld = GetScreenToWorld2D(GetMousePosition(), camera);
      for (int i = 0; i < WALLSNUM; ++i) {
        Rectangle tmp = walls[i].shape;
        tmp.x += WALLEN / 2;
        tmp.y += WALLEN / 2;
        tmp.height = WALLEN / 2;
        tmp.width = WALLEN / 2;
        if (CheckCollisionPointRec(mouseinworld, tmp)) {
          walls[i].cute = !(walls[i].cute);
        }
      }
    }
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
      mouseinworld = GetScreenToWorld2D(GetMousePosition(), camera);
      for (int i = 0; i < WALLSNUM; ++i) {
        if (CheckCollisionPointRec(mouseinworld, walls[i].shape)) {
          walls[i].active = !(walls[i].active);
        }
        Rectangle tmp = walls[i].shape;
        tmp.x += WALLEN / 2;
        tmp.y += WALLEN / 2;
        tmp.height = WALLEN / 2;
        tmp.width = WALLEN / 2;
        if (CheckCollisionPointRec(mouseinworld, tmp)) {
          walls[i].cute = !(walls[i].cute);
        }
      }
    }
    if (IsKeyPressed(KEY_SPACE)) {
      write_to_file(walls);
    };
    if (IsKeyDown(KEY_LEFT_SHIFT)) {
      movespeed = 5;
    } else {
      movespeed = 2;
    }
    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D))
      user.x += movespeed;
    if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A))
      user.x -= movespeed;
    if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S))
      user.y += movespeed;
    if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W))
      user.y -= movespeed;
    camera.zoom = expf(logf(camera.zoom) + ((float)GetMouseWheelMove() * 0.1f));

    if (camera.zoom > 9.0f)
      camera.zoom = 9.0f;
    else if (camera.zoom < 0.1f)
      camera.zoom = 0.1f;

    BeginDrawing();
    ClearBackground(RAYWHITE);
    BeginMode2D(camera);
    for (int i = 0; i < WALLSNUM; ++i) {
      col = walls[i].active ? RED : GRAY;
      DrawRectangleRec(walls[i].shape, col);

      col = walls[i].cute ? YELLOW : RAYWHITE;
      Rectangle tmp = walls[i].shape;
      tmp.x += WALLEN / 2;
      tmp.y += WALLEN / 2;
      tmp.height = WALLEN / 2;
      tmp.width = WALLEN / 2;
      DrawRectangleRec(tmp, col);

      DrawRectangleRec((Rectangle){walls[i].shape.x, walls[i].shape.y,
                                   WALLEN / 2, WALLEN / 2},
                       RED);
    }
    EndMode2D();

    EndDrawing();
  }
}
