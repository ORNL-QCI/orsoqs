#ifndef LIBORSOQS_ERROR_H
#define LIBORSOQS_ERROR_H

#include <common.h>

// unreachable code reached
#define ORSOQS_ERR_UCR -99
// socket doesnt support operation
#define ORSOQS_ERR_BSO -2




bool or_check_error(int64_t code);

char const* or_error_str(const int64_t code);

#endif
