
#include "hsm/hsm.h"
#include "hsm_test/hsm_test.h"
#include "console/console.h"
#include "parse/parse.h"
#include "cli/cli_namespace.h"

/** hsm_test commands usage
 *  Usage:
 *      hsm enter
 *      hsm exit
 *      hsm flip
 *      hsm flop
 *      hsm floop
 */

#define NUM_ARGS_ENTER                  0
#define NUM_ARGS_EXIT                   0
#define NUM_ARGS_FLIP                   0
#define NUM_ARGS_FLOP                   0
#define NUM_ARGS_FLOOP                  0

#define NUM_OPTS_ENTER                  0
#define NUM_OPTS_EXIT                   0
#define NUM_OPTS_FLIP                   0
#define NUM_OPTS_FLOP                   0
#define NUM_OPTS_FLOOP                  0

/* Command Callbacks */
static int on_enter(cli_command_s * cmd, char ** args);
static int on_exit(cli_command_s * cmd, char ** args);
static int on_flip(cli_command_s * cmd, char ** args);
static int on_flop(cli_command_s * cmd, char ** args);
static int on_floop(cli_command_s * cmd, char ** args);

/* Help */
const char hsm_test_help_dialog[] =
    "\nusage:\n"
    "\thsm enter\t\t- Enter the test state machine\n"
    "\thsm exit\t\t- Exit the test state machine\n"
    "\thsm flip\t\t- Raise the flip signal\n"
    "\thsm flop\t\t- Raise the flop signal\n"
    "\thsm floop\t\t- Raise the floop signal\n"
    "\n";

static cli_command_s hsm_test_commands[] = {
    // name                 num_args                    num_options                 
    // opt_list             cb
    { "enter",              NUM_ARGS_ENTER,             NUM_OPTS_ENTER,
      NULL,                 on_enter,                   NULL },
    { "exit",               NUM_ARGS_EXIT,              NUM_OPTS_EXIT,
      NULL,                 on_exit,                    NULL },
    { "flip",               NUM_ARGS_FLIP,              NUM_OPTS_FLIP,
      NULL,                 on_flip,                    NULL },
    { "flop",               NUM_ARGS_FLOP,              NUM_OPTS_FLOP,
      NULL,                 on_flop,                    NULL },  
    { "floop",              NUM_ARGS_FLOOP,             NUM_OPTS_FLOOP,
      NULL,                 on_floop,                   NULL },
    { NULL,                 0,                          0, 
      NULL,                 NULL,                       NULL },
};

/** Namespace Definition */
static cli_namespace_s hsm_test_namespace = {
    .name = "hsm",
    .commands = hsm_test_commands,
    .help = hsm_test_help_dialog,
};

/* Command callback implementations */

static int on_enter(cli_command_s * cmd, char ** args)
{
    hsm_enter(&hsm_test_sm);
    return 0;
}

static int on_exit(cli_command_s * cmd, char ** args)
{
    hsm_exit(&hsm_test_sm);
    return 0;
}

static int on_flip(cli_command_s * cmd, char ** args)
{
    hsm_raise(&hsm_test_sm, HSM_TEST_SIGNAL_FLIP);
    return 0;
}

static int on_flop(cli_command_s * cmd, char ** args)
{
    hsm_raise(&hsm_test_sm, HSM_TEST_SIGNAL_FLOP);
    return 0;
}

static int on_floop(cli_command_s * cmd, char ** args)
{
    hsm_raise(&hsm_test_sm, HSM_TEST_SIGNAL_FLOOP);
    return 0;
}

void hsm_test_cli_init(void)
{
    cli_namespace_register(&hsm_test_namespace);
}
