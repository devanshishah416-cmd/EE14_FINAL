/**
 * @file reverse.c
 * @brief Reverse effect — plays the audio backwards.
 *
 * HOW IT WORKS:
 *   No math at all. We use a two-pointer swap:
 *     - left  starts at index 0 (beginning of buffer)
 *     - right starts at index length-1 (end of buffer)
 *   Swap the two samples, then move both pointers inward.
 *   Stop when they meet in the middle.
 *
 *   Before: [A, B, C, D, E]
 *   After:  [E, D, C, B, A]
 *
 *   This modifies the buffer IN-PLACE, no extra memory needed.
 *   On real hardware, the SRAM buffer gets reversed then played back normally.
 */

#include "dsp_lib.h"

/**
 * @brief Reverse all samples in the buffer in-place.
 * @param buf  Audio buffer. Must not be NULL or empty.
 * @return DSP_OK on success.
 */
DSP_Err reverse(DSP_Buffer *buf)
{
    if (buf == NULL)      return DSP_ERR_NULL;
    if (buf->length == 0) return DSP_ERR_EMPTY;

    size_t left  = 0;
    size_t right = buf->length - 1;

    while (left < right) {
        /* Swap the two samples */
        dsp_sample_t temp  = buf->data[left];
        buf->data[left]    = buf->data[right];
        buf->data[right]   = temp;

        left++;
        right--;
    }

    return DSP_OK;
}
