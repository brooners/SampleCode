/**
 *  @file   hsm.h
 *  @brief  Hierarchical state machine implementation.
 *
 *  A hierarchical state machine facilitates nested states to process signals
 *  in the same way as their parent states without repeating the code to process
 *  those signals. If a nested state should process a signal as its parent
 *  would, it should simply ignore the signal, allowing one of its parent states
 *  to handle the signal instead. A nested state may still choose to process a
 *  signal in a different way.
 *
 */

#ifndef __HSM_H__
#define __HSM_H__

#include <stdlib.h>
#include <inttypes.h>

#include "os/os.h"

// =================================================================
// ====================== TYPEDEFS AND MACROS ======================
// =================================================================

/* Forward declaration of the hsm_s structure */
typedef struct hsm_s hsm_s;
/* Forward declaration of the hsm_state_s structure */
typedef struct hsm_state_s hsm_state_s;

typedef enum 
{
    HSM_SIG_STATUS_HANDLED  =   0,
    HSM_SIG_STATUS_NOT_HANDLED
} hsm_sig_status_e;

/** @brief Function executed upon entry into a state or a state machine 
 *
 *  If defined by the state machine, the entry function is executed when the
 *  hsm_enter function is called for that state machine.
 *  If defined by a state, the entry function is executed during a transition
 *  into that state (after the exit function of the state being transitioned
 *  out of is executed).
 *
 *  @param hsm          State machine to which the entry function belongs
 */
typedef void (*hsm_entry_fn)(hsm_s * hsm);

/** @brief Function executed upon exit from a state or a state machine 
 *
 *  If defined by the state machine, the exit function is executed when the
 *  hsm_exit function is called for that state machine.
 *  If defined by a state, the exit function is executed during a transition
 *  out of that state (before the entry function of the state being transitioned
 *  into is executed, if applicable)
 *
 *  @param hsm          State machine to which the exit function belongs
 */
typedef void (*hsm_exit_fn)(hsm_s * hsm);

/** @brief Function executed when a signal is raised 
 *
 *  Each state MUST define a handler for incoming signals. 
 *  If a state handles the specific signal which has been raised, it shall
 *  return HSM_SIG_STATUS_HANDLED.
 *  If a state does not handle the specific signal which has been raised, it
 *  shall return HSM_SIG_STATUS_NOT_HANDLED.
 *  If a state does not handle the specific signal, the signal will then be
 *  raised to the parent of that state (and so on, until it is handled OR the
 *  signal reaches the top state in the hierarchy).
 *
 *  @return HSM_SIG_STATUS_HANDLED or HSM_SIG_STATUS_NOT_HANDLED
 */
typedef int (*hsm_signal_fn)(hsm_s * hsm, int signal);

/** Representation of a state in the hsm library */
struct hsm_state_s
{
    /** The parent of this state; NULL if the state does not have a parent */
    const hsm_state_s *     hst_parent;
    /** Function to be executed upon entry into the state */
    hsm_entry_fn            hst_on_entry;
    /** Function to be executed upon exit from the state */
    hsm_exit_fn             hst_on_exit;
    /** Function to be executed by this state when a signal is raised */
    hsm_signal_fn           hst_on_signal;
    /** Enumeration attached to the state to be queried by external modules */
    int                     hst_state_num;
};

/** Representation of a hierarchical state machine */
struct hsm_s
{
    /** The state into which the state machine will transition upon the 
     *  hsm_enter function */
    const hsm_state_s *     h_top;
    /** Optional function which is executed upon entry into the state machine */
    hsm_entry_fn            h_on_entry;
    /** Optional function which is executed upon exit from the state machine */
    hsm_exit_fn             h_on_exit;

    /** Current state of the state machine */
    hsm_state_s *           h_cur_state;
    /** Mutex to ensure thread safety of state machine operations */
    struct os_mutex         h_lock;
};

// =================================================================
// ====================== API ======================================
// =================================================================

/** @brief Initialize the state machine structure and (optionally) execute a 
 *  state-machine-specific init function. 
 *
 *  @param hsm          State machine to initialize
 *  @param top          State to enter when hsm_enter is called
 *  @param entry        Optional implementation-specific entry function to call 
 *                      when the state machine is entered
 *  @param exit         Optional implementation-specific exit function to call
 *                      when the state machine is exited
 *
 *  @return 0 on success, non-zero on failure
 */
int hsm_init(hsm_s * hsm, const hsm_state_s * top, hsm_entry_fn entry, 
        hsm_exit_fn exit);

/** @brief Allow the state machine to begin processing signals and (optionally)
 *  execute a state-machine-specific entry function
 *
 *  @param hsm          State machine to enter
 */
void hsm_enter(hsm_s * hsm);

/** @brief Stop a state machine from processing signals and (optionally) 
 *  execute a state-machine-specific exit function 
 *
 *  @param hsm          State machine to exit
 */
void hsm_exit(hsm_s * hsm);

/** @brief Raise a signal to be processed by the state machine 
 *
 *  @param hsm          State machine to process the signal
 *  @param signal       State-machine-specific signal value
 *  @param context      Optional state-machine-specific context argument
 */
void hsm_raise(hsm_s * hsm, int signal);

/** @brief Transition to a new state. The state being transitioned out of will
 *  execute its exit function and the state being transitioned into will
 *  execute its entry function (if applicable)
 *
 *  @param hsm          State machine to perform the transition
 *  @param dst          Destination state
 */
void hsm_transition(hsm_s * hsm, const hsm_state_s * dst);

/** @brief Queries the active/inactive status of the state machine
 *
 *  @param hsm          State machine to query
 *
 *  @return true if the state machine has been entered, false otherwise
 */
bool hsm_is_active(hsm_s * hsm);

/** @brief Returns the state-machine-specific enumerated value associated with
 *  its current state
 *
 *  @param hsm          State machine to query
 *
 *  @return Enumerated value associated with the current state or -1 if the
 *      state machine is inactive.
 */
int hsm_get_current_state(hsm_s * hsm);

// =================================================================
// ====================== EOF ======================================
// =================================================================

#endif // __HSM_H__
