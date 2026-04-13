/**
 * @file volume.c
 * @brief Volume Up and Volume Down effects.
 *
 * HOW IT WORKS:
 *   Multiply every sample by a scale factor.
 *   > 1.0 = louder,  < 1.0 = quieter.
 *   A CLAMP prevents overflow — if the result exceeds int16 range it
 *   gets capped instead of wrapping (wrapping sounds like static).
 */

#include "dsp_lib.h"

#define VOLUME_UP_FACTOR    2.0f   /* +6 dB — doubles the amplitude  */
#define VOLUME_DOWN_FACTOR  0.5f   /* -6 dB — halves  the amplitude  */
#define SAMPLE_MAX  32767
#define SAMPLE_MIN -32768

static DSP_Err apply_volume(DSP_Buffer *buf, float factor)
{
    if (buf == NULL)      return DSP_ERR_NULL;
    if (buf->length == 0) return DSP_ERR_EMPTY;

    for (size_t i = 0; i < buf->length; i++) {
        float scaled = (float)buf->data[i] * factor;
        if      (scaled >  SAMPLE_MAX) scaled =  SAMPLE_MAX;
        else if (scaled <  SAMPLE_MIN) scaled =  SAMPLE_MIN;
        buf->data[i] = (dsp_sample_t)scaled;
    }
    return DSP_OK;
}

DSP_Err volume_up  (DSP_Buffer *buf) { return apply_volume(buf, VOLUME_UP_FACTOR);   }
DSP_Err volume_down(DSP_Buffer *buf) { return apply_volume(buf, VOLUME_DOWN_FACTOR); }
