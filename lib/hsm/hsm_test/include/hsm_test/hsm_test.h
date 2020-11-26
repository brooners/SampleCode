/**
 *  @file   hsm_test.h
 *  @brief  Test / Demonstration for the hsm package
 *
 */

#ifndef __HSM_TEST_H__
#define __HSM_TEST_H__

#include <stdlib.h>
#include <inttypes.h>

#include "hsm/hsm.h"

/** Test state machine */
extern hsm_s hsm_test_sm;

/** Test state machine signals */
typedef enum 
{
    HSM_TEST_SIGNAL_FLIP        =   0,
    HSM_TEST_SIGNAL_FLOP,
    HSM_TEST_SIGNAL_FLOOP
} hsm_test_signal_e;

/** Register the CLI for the test state machine */
void hsm_test_cli_init(void);

#endif // __HSM_TEST_H__
