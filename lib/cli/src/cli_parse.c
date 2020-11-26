/*
 * Author:  Brian Warren
 * Email:   brian@juullabs.com
 *
 * Utility used by the CLI module to parse the command line arguments and
 * options
 */

#include <string.h>
#include "console/console.h"
#include "cli/cli_namespace.h"
#include "cli/cli_parse.h"

/* Parses option flags */
static int cli_parse_option(cli_command_s * cmd, int argc, char ** argv,
	bool * has_arg)
{
	bool opt_found = false;
	int i;
    cli_option_s *cur_opt;

    /* NOTE: cmd->opt_list is a non-constant temporary! */
    cur_opt = (cli_option_s *)cmd->opt_list;

	// Check each command option to see if it exists within the option token.
	// Many options can exist within a single option token
	for(i = 0; i < cmd->num_options; i++)
	{
		if(strchr(argv[0], cur_opt[i].name) != NULL)
		{
			// If the option is expecting an argument, ensure that it exists and
			// that it does not take the form of an option flag
			if(cur_opt[i].has_arg)
			{
				if(argc < 2)
				{
					#if MYNEWT_VAL(CLI_HELP_ENABLE)
					console_printf("Option %s expects an argument\n", argv[0]);
					#endif
					return CLI_ERROR_BAD_ARG;
				}

				// BW_2018-0118 (fix/cli_negative_values): Option arguments CAN begin with '-'
				// if(argv[1][0] == '-')
				// {
				// 	#if MYNEWT_VAL(CLI_HELP_ENABLE)
				// 	console_printf("Option argument cannot begin with \'-\'\n");
				// 	#endif
				// 	return 1;
				// }
				cur_opt[i].value = true;
				cur_opt[i].arg_value = argv[1];
				*has_arg = true;

				#if MYNEWT_VAL(CLI_DEBUG_ENABLE)
				console_printf("Found option %s with argument %s\n", argv[0],
						argv[1]);
				#endif

				return CLI_ERROR_NONE;
			}
			else
			{
				opt_found = true;
				cur_opt[i].value = true;

				#if MYNEWT_VAL(CLI_DEBUG_ENABLE)
				console_printf("Found option %s\n", argv[0]);
				#endif
			}
		}
	}

	return opt_found ? CLI_ERROR_NONE : CLI_ERROR_OPTION_NOT_FOUND;
}

bool cli_parse_arg_is_help(const char *arg)
{
    return strcmp(arg, "help") == 0 ||
            strcmp(arg, "--help") == 0 ||
            strcmp(arg, "-h") == 0;
}

/* Parses arguments and options with respect to the given command */
int cli_parse_command_args(cli_command_s * cmd, int argc, char ** argv,
	char ** arg_list)
{
	int i, rc;
	int args_found = 0;
	bool skip_next_arg = false;

	// Examine each token in the command line. If it begins with '-', try to
	// parse it as an option first. Otherwise, parse it as an argument
	for(i = 0; i < argc; i++)
	{
		#if MYNEWT_VAL(CLI_DEBUG_ENABLE)
		console_printf("Parsing argument %s\n", argv[i]);
		#endif

        if (cli_parse_arg_is_help(argv[i]))
		{
            return CLI_ERROR_HELP_REQUESTED;
		}

		if(argv[i][0] == '-')
		{
			if(cmd->num_options == 0)
			{
				#if MYNEWT_VAL(CLI_HELP_ENABLE)
				console_printf("Command %s does not accept any options\n",
					cmd->name);
				#endif
				return CLI_ERROR_BAD_ARG;
			}

			// Try to parse the argument as an option.
			rc = cli_parse_option(cmd, argc - i, &argv[i], &skip_next_arg);
			if(rc == CLI_ERROR_BAD_ARG)
			{
				#if MYNEWT_VAL(CLI_HELP_ENABLE)
				console_printf("Bad option or option structure: %s\n", argv[0]);
				#endif
				return rc;
			}
			// If the argument is not found in the option list, try to add it
			// to the command argument list.
			else if(rc == CLI_ERROR_OPTION_NOT_FOUND)
			{
				if((cmd->num_args != 0) && (args_found < cmd->num_args))
				{
					#if MYNEWT_VAL(CLI_DEBUG_ENABLE)
					console_printf("Found argument %d of %s: %s\n", args_found,
						cmd->name, argv[i]);
					#endif
					arg_list[args_found] = argv[i];
					args_found++;
				}
				else
				{
					// Command was not expecting an argument or received too many
					// arguments
					#if MYNEWT_VAL(CLI_HELP_ENABLE)
					console_printf("Argument not expected or too many arguments\n");
					#endif
					return CLI_ERROR_BAD_ARG;
				}
			}

			// If the option has an argument, skip the next token in the
			// command line
			if(skip_next_arg)
			{
				#if MYNEWT_VAL(CLI_DEBUG_ENABLE)
				console_printf("Skip next argument\n");
				#endif
				i++;
				skip_next_arg = false;
			}
		}
		else
		{
			// Add the encountered argument to the argument list if the command
			// accepts arguments and the number of arguments encountered has
			// not exceeded the command's expectations
			if((cmd->num_args != 0) && (args_found < cmd->num_args))
			{
				#if MYNEWT_VAL(CLI_DEBUG_ENABLE)
				console_printf("Found argument %d of %s: %s\n", args_found,
					cmd->name, argv[i]);
				#endif
				arg_list[args_found] = argv[i];
				args_found++;
			}
			else
			{
				// Command was not expecting an argument or received too many
				// arguments
				#if MYNEWT_VAL(CLI_HELP_ENABLE)
				console_printf("Argument not expected or too many arguments\n");
				#endif
				return CLI_ERROR_BAD_ARG;
			}
		}
	}

	// Ensure that the number of arguments found is equal to the number of
	// arguments expected by the command
	if(args_found != cmd->num_args)
	{
		#if MYNEWT_VAL(CLI_HELP_ENABLE)
		console_printf("Expected %d arguments but found %d\n",
			cmd->num_args, args_found);
		#endif
		return CLI_ERROR_BAD_ARG;
	}

	return CLI_ERROR_NONE;
}
