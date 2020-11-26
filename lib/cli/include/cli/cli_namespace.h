/*
 * The CLI module integrates with the Mynewt shell to provide a method for
 * creating groups of commands under a common namespace, parse the command line
 * with respect to each individual command's needs, and call back to the
 * command to process the argument values and option inputs.
 *
 * Some notes about creating a namespace:
 * 	- All arguments are required arguments. If a command entered at the
 *	  shell does not have the required number of arguments (given by
 *	  cli_command_s.num_args), it will be rejected at the CLI module
 *	- All options are single character flags. Long options (e.g. --help)
 *	  are invalid
 *	- Command lists must be terminated with a NULL value in the name field
 *
 * Sample Namespace implementation:

 *	Test commands usage
 *	Usage:
 *		test help
 *		test echo <string>
 *		test dictate <message>
 *		test readback
 *		test strikethat
 *		test advertise [-s <something>]
 *		test something [-abc]
 *
 *	Options:
 *		-a 			It's an alright option
 *		-b 			By far the best option
 *		-c 			C's get degrees
 *		-s 			Something to believe in

 static cli_option_s advertise_opts[TEST_ADVERTISE_NUM_OPTS] = {
// 		name 		value 		has_arg 	arg_value
	{ 	's', 		false, 		true, 		NULL },
};

static cli_option_s something_opts[TEST_SOMETHING_NUM_OPTS] = {
// 		name 		value 		has_arg 	arg_value
	{ 	'a', 		false, 		false, 		NULL },
	{ 	'b', 		false, 		false, 		NULL },
	{ 	'c', 		false, 		false, 		NULL },
};

static cli_command_s test_commands[] = {
	// name 		num_args 	num_options 	opt_list		cb
	{ "help", 		0, 			0, 				NULL, 			on_help },
	{ "echo", 		1, 			0, 				NULL, 			on_echo },
	{ "dictate", 	1, 			0, 				NULL, 			on_dictate },
	{ "readback", 	0, 			0, 				NULL, 			on_readback },
	{ "strikethat", 0, 			0, 				NULL, 			on_strikethat },
	{ "advertise", 	1, 			1, 				advertise_opts, on_advertise },
	{ "something", 	0, 			3, 				something_opts, on_something },
	{ NULL, 		0, 			0, 				NULL, 			NULL },
};

static cli_namespace_s test_namespace = {
	.name = "test",
	.commands = test_commands,
};

 *
 */

#ifndef __CLI_NAMESPACE_H__
#define __CLI_NAMESPACE_H__

#include <inttypes.h>
#include "os/os.h"

/* Forward struct declarations */
typedef struct cli_command_s cli_command_s;
typedef struct cli_namespace_s cli_namespace_s;

/* Command callback prototype */
typedef int cli_command_fn(cli_command_s * cmd, char ** args);

/* Command option */
typedef struct
{
	const char 						name;			// Option names are a single character
	bool 							value;			// Indicates if the option was found
	bool 							has_arg;		// Indicates if the option requires an arg
	char * 							arg_value; 		// Arg value if the option was found
} cli_option_s;

/* Command definition */
struct cli_command_s
{
	const char * 					name;			// Command name
	uint8_t							num_args;		// Number of arguments required
	uint8_t							num_options;	// Number of options available
	const cli_option_s *			opt_list;		// Option details
	cli_command_fn * 				cb;				// Function called if the command was entered properly
	const char * 					help; 			// Command help text
};

/* Name space to group multiple commands */
struct cli_namespace_s
{
	const char * 					name;			// Namespace name
	const cli_command_s * 			commands;		// Commands belonging to the namespace
	const char *  					help; 			// Namespace help text
	SLIST_ENTRY(cli_namespace_s) 	next;
};

/* Called by each namespace to register its name with the Mynewt shell */
int cli_namespace_register(cli_namespace_s * new_namespace);

#endif // __CLI_NAMESPACE_H__
