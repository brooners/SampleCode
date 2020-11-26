
#include "hsm_test/hsm_test.h"
#include "hsm/hsm.h"
#include "console/console.h"

// =================================================================
// ====================== STATE DECLARATIONS =======================
// =================================================================

enum hsm_test_states
{
    HSM_TEST_STATE_FLIP,
    HSM_TEST_STATE_FLOP,
    HSM_TEST_STATE_FLOOP,
};

// =================================================================
//                        FLIP
// =================================================================

static void on_flip_enter(hsm_s * hsm);
static void on_flip_exit(hsm_s * hsm);
static int on_flip_signal(hsm_s * hsm, int signal);
hsm_state_s hsm_test_state_flip = 
{
    .hst_parent = NULL,
    .hst_on_entry = on_flip_enter,
    .hst_on_exit = on_flip_exit,
    .hst_on_signal = on_flip_signal,
    .hst_state_num = HSM_TEST_STATE_FLIP,
};

// =================================================================
//                        FLOP
// =================================================================

static void on_flop_enter(hsm_s * hsm);
static void on_flop_exit(hsm_s * hsm);
static int on_flop_signal(hsm_s * hsm, int signal);
hsm_state_s hsm_test_state_flop = 
{
    .hst_parent = NULL,
    .hst_on_entry = on_flop_enter,
    .hst_on_exit = on_flop_exit,
    .hst_on_signal = on_flop_signal,
    .hst_state_num = HSM_TEST_STATE_FLOP,
};

// =================================================================
//                        FLOOP
// =================================================================

static void on_floop_enter(hsm_s * hsm);
static void on_floop_exit(hsm_s * hsm);
static int on_floop_signal(hsm_s * hsm, int signal);
hsm_state_s hsm_test_state_floop =
{
    .hst_parent = &hsm_test_state_flop,
    .hst_on_entry = on_floop_enter,
    .hst_on_exit = on_floop_exit,
    .hst_on_signal = on_floop_signal,
    .hst_state_num = HSM_TEST_STATE_FLOOP,
};

// =================================================================
// ====================== STATE MACHINE DECLARATION ================
// =================================================================

static void on_hsm_test_enter(hsm_s * hsm);
static void on_hsm_test_exit(hsm_s * hsm);

hsm_s hsm_test_sm = 
{
    .h_top = &hsm_test_state_flip,
    .h_on_entry = on_hsm_test_enter,
    .h_on_exit = on_hsm_test_exit,
};

// =================================================================
// ====================== STATE DEFINITIONS ========================
// =================================================================

// =================================================================
//                        FLIP
// =================================================================

static void on_flip_enter(hsm_s * hsm)
{
    console_printf("You flipped\n");
}

static void on_flip_exit(hsm_s * hsm)
{
    console_printf("After flipping...\n");
}

static int on_flip_signal(hsm_s * hsm, int signal)
{
    switch (signal)
    {
        case HSM_TEST_SIGNAL_FLIP:
            console_printf("Already flipped\n");
        break;

        case HSM_TEST_SIGNAL_FLOP:
            hsm_transition(hsm, &hsm_test_state_flop);
        break;

        case HSM_TEST_SIGNAL_FLOOP:
            console_printf("Can\'t floop until you flop\n");
        break;

        default:
            return 1;
    }

    return 0;
}

// =================================================================
//                        FLOP
// =================================================================

static void on_flop_enter(hsm_s * hsm)
{
    console_printf("You flopped\n");
}

static void on_flop_exit(hsm_s * hsm)
{
    console_printf("After flopping...\n");
}

static int on_flop_signal(hsm_s * hsm, int signal)
{
    switch (signal)
    {
        case HSM_TEST_SIGNAL_FLIP:
            hsm_transition(hsm, &hsm_test_state_flip);
        break;

        case HSM_TEST_SIGNAL_FLOP:
            console_printf("Already flopped\n");
        break;

        case HSM_TEST_SIGNAL_FLOOP:
            hsm_transition(hsm, &hsm_test_state_floop);
        break;

        default:
            return 1;
    }

    return 0;
}

// =================================================================
//                        FLOOP
// =================================================================

static void on_floop_enter(hsm_s * hsm)
{
    console_printf("You flooped\n");
}

static void on_floop_exit(hsm_s * hsm)
{
    console_printf("After flooping...\n");
}

static int on_floop_signal(hsm_s * hsm, int signal)
{
    switch (signal)
    {
        case HSM_TEST_SIGNAL_FLOP:
            hsm_transition(hsm, &hsm_test_state_flop);
        break;

        case HSM_TEST_SIGNAL_FLOOP:
            console_printf("Already flooped\n");
        break;

        default:
            return 1;
    }

    return 0;
}

// =================================================================
// ====================== STATE MACHINE DEFINITION =================
// =================================================================

static void on_hsm_test_enter(hsm_s * hsm)
{
    console_printf("Flip and flop, but don\'t floop until you flop\n");
}

static void on_hsm_test_exit(hsm_s * hsm)
{
    console_printf("Done with the flip, flop, floop\n");
}


