#include "bar_main.h"
#include "bar_shell.h"

void cmd_bar(BaseSequentialStream *chp, int argc, char *argv[]) {

  if (argc == 0) {
    goto ERROR;
  }

  if (argc == 1) {
    if (strcmp(argv[0], "get") == 0) {
      chprintf(chp, "got\r\n");
      return;
    } else if ((argc == 2) && (strcmp(argv[0], "set") == 0)) {
      chprintf(chp, "set\r\n");
      return;
    }
  }

ERROR:
  chprintf(chp, "Usage: bar get\r\n");
  chprintf(chp, "       bar set x\r\n");
  chprintf(chp, "where x is something\r\n");
  chprintf(chp, "and that's it\r\n");
  return;
}
