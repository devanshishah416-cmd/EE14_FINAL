/**
 * @file dsp_lib.h
 * @brief Shared types and prototypes for EE14 DJ Controller DSP effects.
 *
 * ADD A NEW EFFECT:
 *   1. Create a new .c file (e.g. reverb.c)
 *   2. Add its prototype at the bottom of this file
 *   3. Call it from main.c
 *   That's it — nothing else changes.
 */

#ifndef DSP_LIB_H
#define DSP_LIB_H

#include <stdint.h>
#include <stddef.h>

/* ── Sample type ─────────────────────────────────────────────────────────
   16-bit signed integer, range -32768 to +32767.
   This matches STM32 ADC output once centered around 0.              */
typedef int16_t dsp_sample_t;

/* ── Buffer ──────────────────────────────────────────────────────────────
   At 8000 Hz, 8000 samples = 1 second of audio.
   Bump DSP_MAX_SAMPLES once the SRAM is wired up.                    */
#define DSP_SAMPLE_RATE   8000
#define DSP_MAX_SAMPLES   8000

typedef struct {
    dsp_sample_t data[DSP_MAX_SAMPLES];
    size_t       length;    /* number of valid samples currently in buf */
} DSP_Buffer;

/* ── Error codes ─────────────────────────────────────────────────────── */
typedef int DSP_Err;
#define DSP_OK           0
#define DSP_ERR_NULL    -1   /* buffer pointer was NULL                  */
#define DSP_ERR_EMPTY   -2   /* buffer has no samples                    */

/* ═══════════════════════════════════════════════════════════════════════
   FUNCTION PROTOTYPES  — one line per effect file
   ═════════════════════════════════════════════════════════════════════ */

/* test_signal.c */
DSP_Err generate_test_signal(DSP_Buffer *buf, float freq_hz, float amplitude);

/* volume.c */
DSP_Err volume_up  (DSP_Buffer *buf);
DSP_Err volume_down(DSP_Buffer *buf);

/* reverse.c */
DSP_Err reverse(DSP_Buffer *buf);

/* halve.c */
DSP_Err halve(DSP_Buffer *buf);

/* echo.c */
DSP_Err echo_effect(DSP_Buffer *buf);

#endif /* DSP_LIB_H */
