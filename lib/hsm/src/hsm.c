/**
 *  @file   hsm.c
 *  @brief  
 */

#include "os/os.h"
#include "hsm/hsm.h"

// =================================================================
// ====================== API ======================================
// =================================================================

int hsm_init(hsm_s * hsm, const hsm_state_s * top, hsm_entry_fn entry, 
        hsm_exit_fn exit)
{
    int rc;

    if (top == NULL)
    {
        return 1; 
    }

    hsm->h_top = top;
    hsm->h_on_entry = entry;
    hsm->h_on_exit = exit;

    rc = os_mutex_init(&hsm->h_lock);
    if (rc)
    {
        return rc;
    }
    hsm->h_cur_state = NULL;

    return 0;
}

void hsm_enter(hsm_s * hsm)
{
    if (hsm_is_active(hsm))
    {
        return;
    }

    if (hsm->h_on_entry != NULL)
    {
        hsm->h_on_entry(hsm);
    }

    hsm_transition(hsm, hsm->h_top);
}

void hsm_exit(hsm_s * hsm)
{
    if (!hsm_is_active(hsm))
    {
        return;
    }

    os_mutex_pend(&hsm->h_lock, OS_TIMEOUT_NEVER);
    hsm_transition(hsm, NULL);

    if (hsm->h_on_exit != NULL)
    {
        hsm->h_on_exit(hsm);
    }

    os_mutex_release(&hsm->h_lock);
}

void hsm_raise(hsm_s * hsm, int signal)
{
    const hsm_state_s * current;
    int rc;

    if (!hsm_is_active(hsm))
    {
        return;
    }

    os_mutex_pend(&hsm->h_lock, OS_TIMEOUT_NEVER);

    current = hsm->h_cur_state;

    do
    {
        rc = current->hst_on_signal(hsm, signal);
        current = current->hst_parent;
    } while((current != NULL) && (rc != 0));

    os_mutex_release(&hsm->h_lock);
}

void hsm_transition(hsm_s * hsm, const hsm_state_s * dst)
{
    hsm_state_s * src;
    os_mutex_pend(&hsm->h_lock, OS_TIMEOUT_NEVER);

    src = hsm->h_cur_state;

    if (src != NULL)
    {
        if (src->hst_on_exit != NULL)
        {
            src->hst_on_exit(hsm);
        }
    }

    hsm->h_cur_state = (hsm_state_s *)dst;

    if (dst != NULL)
    {
        if (dst->hst_on_entry != NULL)
        {
            dst->hst_on_entry(hsm);
        }
    }

    os_mutex_release(&hsm->h_lock);
}

bool hsm_is_active(hsm_s * hsm)
{
    return hsm->h_cur_state != NULL;
}

int hsm_get_current_state(hsm_s * hsm)
{
    if (!hsm_is_active(hsm))
    {
        return -1;
    }

    return hsm->h_cur_state->hst_state_num;
}

// =================================================================
// ====================== EOF ======================================
// =================================================================
