/**
 *  @file   moving_avg.c
 *
 */

#include "moving_avg/moving_avg.h"

// =================================================================
// ====================== TYPEDEFS AND MACROS ======================
// =================================================================

// =================================================================
// ====================== API ======================================
// =================================================================

void moving_avg_init(float * avg, float first_sample)
{
    *avg = first_sample;
}

void moving_avg_compute(float * avg, float sample, uint32_t window_size)
{
    float prev_avg = *avg;
    float inv_window_size = 1.0 / ((float)window_size);
    // Calculate the new average
    *avg = prev_avg + inv_window_size * (sample - prev_avg);    
}

// =================================================================
// ====================== EOF ======================================
// =================================================================
