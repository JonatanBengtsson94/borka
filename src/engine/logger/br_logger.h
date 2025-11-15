#ifndef BR_LOGGER_H
#define BR_LOGGER_H

#include <stdbool.h>

bool br_logger_init(const char *game_name);
void br_logger_shutdown(void);

#endif // BR_LOGGER_H
