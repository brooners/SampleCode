/*
 * Author:  Brian Warren
 * Email:   brian@juullabs.com
 * 
 * The CLI module integrates with the Mynewt shell to provide a method for
 * creating groups of commands under a common namespace, parse the command line
 * with respect to each individual command's needs, and call back to the
 * command to process the argument values and option inputs.
 *
 * Some notes about creating a namespace:
 *  - All arguments are required arguments. If a command entered at the
 *    shell does not have the required number of arguments (given by 
 *    cli_command_s.num_args), it will be rejected at the CLI module
 *  - All options are single character flags. Long options (e.g. --help)
 *    are invalid
 *  - Command lists must be terminated with a NULL value in the name field
 *  
 * Sample Namespace implementation:

 *  Test commands usage
 *  Usage:
 *      test help
 *      test echo <string>
 *      test dictate <message>
 *      test readback
 *      test strikethat
 *      test advertise [-s <something>]
 *      test something [-abc]
 * 
 *  Options:
 *      -a          It's an alright option
 *      -b          By far the best option
 *      -c          C's get degrees
 *      -s          Something to believe in
 
 static cli_option_s advertise_opts[TEST_ADVERTISE_NUM_OPTS] = {
//      name        value       has_arg     arg_value
    {   's',        false,      true,       NULL },
};

static cli_option_s something_opts[TEST_SOMETHING_NUM_OPTS] = {
//      name        value       has_arg     arg_value
    {   'a',        false,      false,      NULL },
    {   'b',        false,      false,      NULL },
    {   'c',        false,      false,      NULL },
};

static cli_command_s test_commands[] = {
    // name         num_args    num_options     opt_list        cb
    { "help",       0,          0,              NULL,           on_help },
    { "echo",       1,          0,              NULL,           on_echo },
    { "dictate",    1,          0,              NULL,           on_dictate },
    { "readback",   0,          0,              NULL,           on_readback },
    { "strikethat", 0,          0,              NULL,           on_strikethat },
    { "advertise",  1,          1,              advertise_opts, on_advertise },
    { "something",  0,          3,              something_opts, on_something },
    { NULL,         0,          0,              NULL,           NULL },
};

static cli_namespace_s test_namespace = {
    .name = "test",
    .commands = test_commands,
};

 *
 */

#include <string.h>
#include <assert.h>

#include "defs/error.h"
#include "os/os.h"
#include "console/console.h"
#include "shell/shell.h"

#include "cli/cli_namespace.h"
#include "cli/cli_parse.h"

static struct os_mutex g_cli_namespace_list_lock;

SLIST_HEAD(, cli_namespace_s) g_cli_namespace_list;

static cli_option_s g_cmd_options[MYNEWT_VAL(CLI_MAX_NUM_OPTIONS)];
static char * g_cmd_args[MYNEWT_VAL(CLI_MAX_NUM_ARGS)];

// #if MYNEWT_VAL(CLI_DEBUG_ENABLE)
// static void print_command_struct(cli_command_s * cmd);
// #endif

static int cli_namespace_list_lock(void)
{
    int rc;

    if(!os_started())
    {
        return 0;
    }

    rc = os_mutex_pend(&g_cli_namespace_list_lock, OS_WAIT_FOREVER);
    if(rc != 0)
    {
        return rc;
    }

    return 0;
}

static int cli_namespace_list_unlock(void)
{
    int rc;

    if(!os_started())
    {
        return 0;
    }

    rc = os_mutex_release(&g_cli_namespace_list_lock);
    if(rc != 0)
    {
        return rc;
    }

    return 0;
}

/** Finds the namespace invoked at the shell */
static int cli_namespace_find(const char * nmspc_name, cli_namespace_s ** output)
{
    cli_namespace_s * namespace = NULL;
    int rc;

    rc = cli_namespace_list_lock();
    if(rc != 0)
    {
        return rc;
    }

    // Cycle through the list of registered namespaces to determine which
    // namespace was invoked at the command line
    SLIST_FOREACH(namespace, &g_cli_namespace_list, next)
    {
        if (namespace->name != NULL)
        {
            if (!strcmp(namespace->name, nmspc_name))
            {
                break;
            }
        }
    }

    rc = cli_namespace_list_unlock();
    if(rc != 0)
    {
        return rc;
    }

    if(output != NULL)
    {
        *output = namespace;
    }

    if(namespace == NULL)
    {
        return SYS_ENOENT;
    }

    return 0;
}

/** Determines which command was invoked at the command line */
static int cli_command_find(const cli_command_s * cmds, char * arg, int * cmd_index)
{
    int i;

    for(i = 0; cmds[i].name != NULL; i++)
    {
        if(!strcmp(arg, cmds[i].name))
        {
            *cmd_index = i;
            return 0;
        }
    }

    return 1;
}

/**
 * Prints help for an individual command, if enabled.
 */
static void cli_command_print_help(const cli_command_s *cmd)
{
#if MYNEWT_VAL(CLI_HELP_ENABLE)
    if (cmd->help != NULL)
    {
        console_printf(cmd->help);
    }
    else
    {
        console_printf("%s - help not available\n", cmd->name);
    }
#endif
}

/**
 * Prints help for a command namespace, if enabled.
 */
static void cli_namespace_print_help(const cli_namespace_s *namespace)
{
#if MYNEWT_VAL(CLI_HELP_ENABLE)
    if (namespace->help != NULL)
    {
        console_printf(namespace->help);
    }
    else
    {
        console_printf("%s - help not available\n", namespace->name);
    }
#endif
}

/** Callback called when the Shell encounters any namespace registered through
 *  the cmd_namespace module. Parses the command line with respect to the given
 *  namespace command. If the command was entered properly, execution is
 *  directed to the command's specific callback.
 */
static int cli_namespace_on_shell_rx(int argc, char ** argv)
{
    cli_namespace_s * namespace;
    cli_command_s command;
    int rc, i;
    int cmd_index = 0xFFFF;

    // Find the namespace being invoked
    rc = cli_namespace_find(argv[0], &namespace);
    if(rc != 0)
    {
        return rc;
    }

    // Print help text (if available) if the namespace was the only token on the
    // command line
    if(argc == 1)
    {
        #if MYNEWT_VAL(CLI_HELP_ENABLE)
        console_printf("No command given\n");
        #endif

        cli_namespace_print_help(namespace);
        return 1;
    }

    // Print help text if requested by user.
    if (cli_parse_arg_is_help(argv[1]))
    {
        cli_namespace_print_help(namespace);
        return 0;
    }

    // Find the command being invoked
    rc = cli_command_find(namespace->commands, argv[1], &cmd_index);
    if(rc != 0)
    {
        #if MYNEWT_VAL(CLI_HELP_ENABLE)
        console_printf("Command %s not found\n", argv[1]);
        #endif

        cli_namespace_print_help(namespace);
        return rc;
    }

    // Copy the statically-defined command to a temporary command
    memcpy(&command, &namespace->commands[cmd_index], sizeof(cli_command_s));

    // Copy the contents of the statically-defined options to the temp command
    if(namespace->commands[cmd_index].opt_list != NULL)
    {
        for(i = 0; i < namespace->commands[cmd_index].num_options; i++)
        {
            memcpy(&g_cmd_options[i], 
                &namespace->commands[cmd_index].opt_list[i], 
                sizeof(cli_option_s));
        }
        
        command.opt_list = g_cmd_options;
        // #if MYNEWT_VAL(CLI_DEBUG_ENABLE)
        // print_command_struct(&command);
        // #endif
    }

    // Parse all arguments and populate the command structure
    rc = cli_parse_command_args(&command, argc - 2, &argv[2], g_cmd_args);
    if (rc == CLI_ERROR_HELP_REQUESTED)
    {
        cli_command_print_help(&command);
        return 0;
    }
    else if (rc != 0)
    {
        #if MYNEWT_VAL(CLI_HELP_ENABLE)
        console_printf("Bad command or argument structure\n");
        #endif
        cli_command_print_help(&command);
        return rc;
    }

    // Call the namespace with populated arguments for processing
    if(command.cb != NULL)
    {
        rc = command.cb(&command, g_cmd_args);
        if(rc != 0)
        {
            #if MYNEWT_VAL(CLI_HELP_ENABLE)
            console_printf("Bad argument or option struture in %s command\n",
                command.name);
            #endif
            cli_command_print_help(&command);
            return rc;
        }
    }

    return rc != 0 ? rc : 0;
}

/** Register a new namespace with the Mynewt shell. The shell will call back to
 *  the cmd_namespace module as an interim to direct execution to the
 *  appropriate place within the namespace
 */
int cli_namespace_register(cli_namespace_s * new_namespace)
{
    int rc, i;
    struct shell_cmd new_cmd;

    // Ensure the new namespace does not require more args/options than the
    // maximums allowed
    for(i = 0; new_namespace->commands[i].name != NULL; i++)
    {
        if(new_namespace->commands[i].num_args > MYNEWT_VAL(CLI_MAX_NUM_ARGS))
        {
            assert(0);
        }
        if(new_namespace->commands[i].num_options > MYNEWT_VAL(CLI_MAX_NUM_OPTIONS))
        {
            assert(0);
        }
    }

    new_cmd = (struct shell_cmd) {
        .sc_cmd = new_namespace->name,
        .sc_cmd_func = cli_namespace_on_shell_rx,
        .help = NULL,
    };

    // Ensure the new namespace command can be registered with the shell
    assert(shell_cmd_register(&new_cmd) == 0);

    // Ensure new namespace command is not already registered
    assert(cli_namespace_find(new_namespace->name, NULL) == SYS_ENOENT);

    // Ensure the new namespace has an associated command list
    assert(new_namespace->commands != NULL);

    rc = cli_namespace_list_lock();
    if(rc != 0)
    {
        return rc;
    }

    // Add the new namespace to a list to be referenced upon callback from the
    // shell
    SLIST_INSERT_HEAD(&g_cli_namespace_list, new_namespace, next);

    rc = cli_namespace_list_unlock();
    if(rc != 0)
    {
        return rc;
    }

    return 0;
}

// #if MYNEWT_VAL(CLI_DEBUG_ENABLE)
// static void print_command_struct(cli_command_s * cmd)
// {
//     int i;

//     console_printf("Command %s\n", cmd->name);
//     console_printf("%d Arguments\n", cmd->num_args);
//     console_printf("Options:\n");
//     for(i = 0; i < cmd->num_options; i++)
//     {
//         console_printf("\t-%c\n", cmd->opt_list[i].name);
//     }
// }
// #endif
