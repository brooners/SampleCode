/**
 *  @file   moving_avg.h
 *  @brief  Windowed moving average filter implementation
 *
 */

#ifndef __MOVING_AVG_H__
#define __MOVING_AVG_H__

#include <stdlib.h>
#include <inttypes.h>

// =================================================================
// ====================== API ======================================
// =================================================================

/** @brief Initialize a moving average with the first sample
 *
 *  @param avg          Average to initialize
 *  @param first_sample First sample of the average
 */
void moving_avg_init(float * avg, float first_sample);

/** @brief Compute the average of the input values, given the new sample
 *
 *  @param avg          Average to which the sample is applied
 *  @param sample       New sample added to the filter
 *  @param window_size  Sample window size
 */
void moving_avg_compute(float * avg, float sample, uint32_t window_size);

// =================================================================
// ====================== EOF ======================================
// =================================================================

#endif /* __MOVING_AVG_H__ */
