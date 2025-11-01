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

int main(int argc, char **argv) {
  NOB_GO_REBUILD_URSELF(argc, argv);

  nob_cc(cmd);
  nob_cc_flags(cmd);
  nob_cc_inputs(cmd, SRC_FOLDER "main.c");
  nob_cc_output(cmd, RUNNABLE);
	cmd_append(cmd, "-lraylib");
	cmd_append(cmd, "-lm");
  if (!cmd_run(cmd))
    return 1;
  return test();

  return 0;
}

int test() {
  cmd_append(cmd, RUNNABLE);
  if (!cmd_run(cmd))
    return 1;
  return 0;
}
