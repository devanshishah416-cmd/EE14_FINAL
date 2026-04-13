/**
 * @file echo.c
 * @brief Echo / Reverb effect using a delay line.
 *
 * HOW IT WORKS:
 *   The formula for echo is:
 *       y[n] = x[n] + alpha * y[n - D]
 *
 *   Where:
 *     x[n]      = current dry (original) sample
 *     y[n-D]    = what we output D samples ago (the delayed echo)
 *     alpha     = decay factor (0.0 to 1.0). Lower = echo fades faster.
 *     D         = delay length in samples. At 8000 Hz:
 *                   D=4000 → 0.5 second echo
 *                   D=8000 → 1.0 second echo
 *
 *   We store the last D output samples in a CIRCULAR BUFFER (delay_line[]).
 *   A circular buffer is just an array we treat like a ring:
 *   when the index reaches the end, it wraps back to 0.
 *   This avoids shifting the entire array every sample (which would be slow).
 *
 *   CIRCULAR BUFFER DIAGRAM (D=4, just to show the concept):
 *
 *   write_pos moves forward one step each sample:
 *   [ y0 | y1 | y2 | y3 ]
 *          ^
 *       write_pos=1
 *   Reading D steps back: read_pos = (write_pos - D + D) % D
 *
 * TUNING:
 *   Change ECHO_DELAY_SAMPLES to make the echo longer or shorter.
 *   Change ECHO_ALPHA to make it fade faster (lower) or longer (higher).
 *   ECHO_ALPHA must stay below 1.0 or the echo grows forever (feedback loop).
 */

#include "dsp_lib.h"

/* ── Tuning knobs ────────────────────────────────────────────────────── */
#define ECHO_DELAY_SAMPLES  4000    /* 0.5 seconds at 8000 Hz            */
#define ECHO_ALPHA          0.6f    /* each echo is 60% as loud as prior */

/**
 * @brief Apply echo/reverb to the buffer in-place.
 * @param buf  Audio buffer. Must not be NULL or empty.
 * @return DSP_OK on success.
 */
DSP_Err echo_effect(DSP_Buffer *buf)
{
    if (buf == NULL)      return DSP_ERR_NULL;
    if (buf->length == 0) return DSP_ERR_EMPTY;

    /* Delay line: stores the last ECHO_DELAY_SAMPLES output values.
       Declared static so it lives in RAM and doesn't eat stack space.
       On real hardware this would go in SRAM. */
    static dsp_sample_t delay_line[ECHO_DELAY_SAMPLES];
    static int initialized = 0;

    /* Zero out the delay line on first use (or each new call) */
    if (!initialized) {
        for (int i = 0; i < ECHO_DELAY_SAMPLES; i++) delay_line[i] = 0;
        initialized = 1;
    }
    /* Reset it each time so each new recording starts with a clean echo */
    for (int i = 0; i < ECHO_DELAY_SAMPLES; i++) delay_line[i] = 0;

    int write_pos = 0;  /* where in delay_line we write next */

    for (size_t n = 0; n < buf->length; n++) {

        /* Read the sample from D steps ago (the echo) */
        /* write_pos is WHERE WE ARE ABOUT TO WRITE, so it currently
           holds the oldest sample — exactly D steps ago. Perfect. */
        float echo_sample = (float)delay_line[write_pos];

        /* Mix: current dry signal + decayed echo */
        float output = (float)buf->data[n] + ECHO_ALPHA * echo_sample;

        /* Clamp to int16 range */
        if      (output >  32767.0f) output =  32767.0f;
        else if (output < -32768.0f) output = -32768.0f;

        dsp_sample_t out_sample = (dsp_sample_t)output;

        /* Store this output into the delay line for future echo */
        delay_line[write_pos] = out_sample;

        /* Advance write position, wrapping around at the end */
        write_pos = (write_pos + 1) % ECHO_DELAY_SAMPLES;

        /* Write the mixed output back to the buffer */
        buf->data[n] = out_sample;
    }

    return DSP_OK;
}
