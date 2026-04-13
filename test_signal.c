/**
 * @file test_signal.c
 * @brief Generates a synthetic sine wave — no microphone needed.
 *
 * Every effect file is tested against this signal.
 * When real hardware arrives, replace generate_test_signal() calls
 * in main.c with actual ADC capture. Everything else stays the same.
 */

#include <math.h>
#include "dsp_lib.h"

/**
 * @brief Fill a DSP_Buffer with a sine wave.
 *
 * @param buf        Buffer to fill. Must not be NULL.
 * @param freq_hz    Frequency in Hz (e.g. 440.0 for concert A).
 * @param amplitude  0.0 to 1.0. Use 0.5 to start — leaves headroom for
 *                   volume_up without clipping.
 */
DSP_Err generate_test_signal(DSP_Buffer *buf, float freq_hz, float amplitude)
{
    if (buf == NULL) return DSP_ERR_NULL;

    buf->length = DSP_MAX_SAMPLES;
    float peak  = amplitude * 32767.0f;

    for (size_t i = 0; i < buf->length; i++) {
        float t       = (float)i / (float)DSP_SAMPLE_RATE;
        float sample  = peak * sinf(2.0f * (float)M_PI * freq_hz * t);
        buf->data[i]  = (dsp_sample_t)sample;
    }

    return DSP_OK;
}
