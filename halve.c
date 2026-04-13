/**
 * @file halve.c
 * @brief Halve effect — plays only the first half of the recording.
 *
 * HOW IT WORKS:
 *   The audio data in the buffer is NOT touched at all.
 *   We just update buf->length to stop halfway through.
 *   When main.c loops over buf->length to play back, it naturally
 *   stops at the halfway point — so it sounds like a shorter clip.
 *
 *   This is the simplest possible DSP "effect": one line of math.
 *
 *   Example:
 *     buf->length was 8000 (1 second at 8 kHz)
 *     After halve(): buf->length = 4000 (0.5 seconds)
 *     The second 4000 samples are still in memory, just ignored.
 */

#include "dsp_lib.h"

/**
 * @brief Halve the playback length of the buffer.
 * @param buf  Audio buffer. Must not be NULL or empty.
 * @return DSP_OK on success.
 */
DSP_Err halve(DSP_Buffer *buf)
{
    if (buf == NULL)      return DSP_ERR_NULL;
    if (buf->length == 0) return DSP_ERR_EMPTY;

    buf->length = buf->length / 2;

    return DSP_OK;
}
