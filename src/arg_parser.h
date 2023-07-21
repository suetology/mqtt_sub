#ifndef ARG_PARSER_H
#define ARG_PARSER_H

#include <stddef.h>

#include "connection.h"

int parse_arguments(int argc, char* argv[], unsigned *is_daemon, struct Connection *connection);

#endif