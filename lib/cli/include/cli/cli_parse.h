/*
 * Utility used by the CLI module to parse the command line arguments and
 * options
 */

#ifndef __CLI_PARSE_H__
#define __CLI_PARSE_H__

#include <inttypes.h>
#include "cli/cli_namespace.h"

#define CLI_ERROR_NONE                      0
#define CLI_ERROR_BAD_ARG                   1
#define CLI_ERROR_OPTION_NOT_FOUND          2
#define CLI_ERROR_HELP_REQUESTED            3

/* Parses arguments and options with respect to the given command */
int cli_parse_command_args(cli_command_s * cmd, int argc, char ** argv,
	char ** arg_list);

/**
 * @brief Indicates whether the provided CLI argument is a request for help.
 *
 * @param arg                   The CLI argument to inspect.
 *
 * @return                      true if help is being requested; else false.
 */
bool cli_parse_arg_is_help(const char *arg);

#endif // __CLI_PARSE_H__
