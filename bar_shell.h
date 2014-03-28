#ifndef _BAR_SHELL_H_
#define _BAR_SHELL_H_

#include <string.h>

#include "ch.h"
#include "hal.h"

#include "chprintf.h"
#include "shell.h"

void cmd_bar(BaseSequentialStream *chp, int argc, char *argv[]);

#endif /* _BAR_SHELL_H_ */
