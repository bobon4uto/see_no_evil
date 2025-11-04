#define THE_NAME "main"

#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "nob.h"

Cmd cmd0 = {0};
Cmd *cmd = &cmd0;

#define SRC_FOLDER "src/"
#define BUILD_FOLDER "build/"

#define RUNNABLE BUILD_FOLDER THE_NAME

int test();

bool build_web();
bool build_linux();
bool build_ses();

int main(int argc, char **argv) {
  NOB_GO_REBUILD_URSELF(argc, argv);

  if (argc > 1) {
    switch (argv[1][0]) {
    case 'l':
      if (!build_linux())
        return 1;
      break;
    case 'w':
      if (!build_web())
        return 1;
      break;
    case 's':
      if (!build_ses())
        return 1;
      break;
    }
  } else {
    if (!build_linux())
      return 1;
  }

  return 0;
}

int test() {
  cmd_append(cmd, RUNNABLE);
  if (!cmd_run(cmd))
    return 1;
  return 0;
}
bool build_linux() {
  nob_cc(cmd);
  nob_cc_flags(cmd);
  nob_cc_inputs(cmd, SRC_FOLDER "main.c");
  nob_cc_output(cmd, RUNNABLE);
  // nob_cc_output(cmd, RUNNABLE);
  cmd_append(cmd, "-lraylib");
  cmd_append(cmd, "-lm");
  if (!cmd_run(cmd))
    return false;
  return true;
}
bool build_ses() {
  nob_cc(cmd);
  nob_cc_flags(cmd);
  nob_cc_inputs(cmd, SRC_FOLDER "ses.c");
  nob_cc_output(cmd, BUILD_FOLDER "ses");
  // nob_cc_output(cmd, RUNNABLE);
  cmd_append(cmd, "-lraylib");
  cmd_append(cmd, "-lm");
  if (!cmd_run(cmd))
    return false;
  return true;
}

bool build_web() { // idk what im doing btw
  if (!nob_mkdir_if_not_exists(BUILD_FOLDER "web"))
    return false;
  cmd_append(cmd, "emcc");
  nob_cc_output(cmd, BUILD_FOLDER "web/index.html");
  cmd_append(cmd, "src/main.c");
  cmd_append(cmd, "-Wall");
  cmd_append(cmd, "-std=c23");
  cmd_append(cmd, "-D_DEFAULT_SOURCE");
  cmd_append(cmd, "-O3");
  cmd_append(cmd, "-I.");
  cmd_append(cmd, "-I", "../raylib/src");
  cmd_append(cmd, "-I", "../raylib/src/external");
  cmd_append(cmd, "-L.");
  cmd_append(cmd, "-L", "../raylib/src");
  cmd_append(cmd, "-s", "USE_GLFW=3");
  cmd_append(cmd, "-s", "ASYNCIFY");
  cmd_append(cmd, "-s", "TOTAL_MEMORY=67108864");
  cmd_append(cmd, "-s", "STACK_SIZE=655360");
  cmd_append(cmd, "-s", "ALLOW_MEMORY_GROWTH=1");
  cmd_append(cmd, "-s", "FORCE_FILESYSTEM=1");
  cmd_append(cmd, "../raylib/src/libraylib.web.a");
  cmd_append(cmd, "-DPLATFORM_WEB", "-s", "EXPORTED_RUNTIME_METHODS=ccall");
  cmd_append(cmd, "--preload-file", "./assets");
  cmd_append(cmd, "--shell-file", "src/shell.html");
  // cmd_append(&cmd, "-s",
  // "'EXPORTED_FUNCTIONS=[\"_free\",\"_malloc\",\"_main\"]'");

  if (!cmd_run(cmd))
    return false;

  cmd_append(cmd, "rm", BUILD_FOLDER "web/web.zip");
  cmd_run(cmd);
  cmd_append(cmd, "zip", "-j", BUILD_FOLDER "web/web.zip",
             BUILD_FOLDER "web/index.html", BUILD_FOLDER "web/index.data",
             BUILD_FOLDER "web/index.js", BUILD_FOLDER "web/index.wasm", );
  if (!cmd_run(cmd))
    return false;
  printf("web!");
  return true;
}
