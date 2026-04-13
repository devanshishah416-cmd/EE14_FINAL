/**
 * @file pitch.c
 * @brief Pitch Up and Pitch Down effects using FFT bin shifting.
 *
 * ═══════════════════════════════════════════════════════════════════════
 * HOW IT WORKS — read this before touching the code
 * ═══════════════════════════════════════════════════════════════════════
 *
 * Sound is made of frequencies. A 440 Hz sine wave is "concert A."
 * To shift pitch UP by one semitone (e.g. A → A#), you multiply the
 * frequency by 2^(1/12) ≈ 1.0595.
 *
 * We can't just change the frequency of a sample directly in time domain.
 * Instead we use the FFT (Fast Fourier Transform):
 *
 *   STEP 1 — FFT
 *     Convert the time-domain audio (array of samples) into the
 *     frequency domain (array of complex numbers, one per frequency bin).
 *     Each bin represents how much of that frequency is in the signal.
 *
 *   STEP 2 — SHIFT BINS
 *     To pitch UP by one semitone: multiply all bin indices by 1.0595.
 *     Bin 10 (440 Hz) moves to bin 11 (466 Hz = A#). Pitch is higher.
 *     To pitch DOWN: divide by 1.0595 instead.
 *     Bins beyond the buffer edge are discarded (they'd be above Nyquist).
 *
 *   STEP 3 — IFFT
 *     Convert back from frequency domain to time domain.
 *     Output is audio at the new pitch, same length, same speed.
 *
 * ═══════════════════════════════════════════════════════════════════════
 * FFT IMPLEMENTATION
 * ═══════════════════════════════════════════════════════════════════════
 *
 * We implement a Cooley-Tukey radix-2 FFT from scratch (no external lib).
 * It requires the buffer size to be a power of 2.
 * We use FFT_SIZE = 1024 samples per window (about 128 ms at 8 kHz).
 *
 * The audio buffer is processed in overlapping windows (same OLA idea
 * as time-stretching), so the full 8000-sample buffer gets pitch-shifted
 * in chunks and reassembled.
 *
 * ═══════════════════════════════════════════════════════════════════════
 * TUNING
 * ═══════════════════════════════════════════════════════════════════════
 *   PITCH_UP_SEMITONES   — how many semitones to shift up   (default: 2)
 *   PITCH_DOWN_SEMITONES — how many semitones to shift down (default: 2)
 *
 *   1 semitone  = one piano key
 *   12 semitones = one full octave
 */

#include <math.h>
#include <string.h>
#include "dsp_lib.h"

/* ── FFT configuration ───────────────────────────────────────────────── */
#define FFT_SIZE     1024          /* must be power of 2                  */
#define FFT_HALF     (FFT_SIZE/2)
#define HOP_SIZE     (FFT_SIZE/4)  /* 75% overlap for smooth output       */

/* ── Pitch shift amounts ─────────────────────────────────────────────── */
#define PITCH_UP_SEMITONES    2
#define PITCH_DOWN_SEMITONES  2

/* Ratio = 2^(semitones/12) */
#define PITCH_UP_RATIO   1.12246f   /* 2^(2/12) — two semitones up   */
#define PITCH_DOWN_RATIO 0.89090f   /* 2^(-2/12) — two semitones down */

/* ── Complex number type (real + imaginary) ──────────────────────────── */
typedef struct { float r; float i; } Complex;

/* ─────────────────────────────────────────────────────────────────────
   bit_reverse_copy
   The Cooley-Tukey FFT requires input samples in bit-reversed order.
   Example for N=8: index 3 (binary 011) → index 6 (binary 110).
   ───────────────────────────────────────────────────────────────────── */
static void bit_reverse_copy(const Complex *in, Complex *out, int n)
{
    int bits = 0;
    int temp = n;
    while (temp > 1) { bits++; temp >>= 1; }   /* log2(n) */

    for (int i = 0; i < n; i++) {
        int rev = 0;
        int x   = i;
        for (int b = 0; b < bits; b++) {
            rev = (rev << 1) | (x & 1);
            x >>= 1;
        }
        out[rev] = in[i];
    }
}

/* ─────────────────────────────────────────────────────────────────────
   fft_inplace
   Cooley-Tukey radix-2 DIT FFT, in-place.
   If inverse=1, computes the IFFT (divide by N handled by caller).
   ───────────────────────────────────────────────────────────────────── */
static void fft_inplace(Complex *buf, int n, int inverse)
{
    float sign = inverse ? 1.0f : -1.0f;

    for (int len = 2; len <= n; len <<= 1) {
        float ang = sign * 2.0f * (float)M_PI / (float)len;
        Complex wlen = { cosf(ang), sinf(ang) };

        for (int i = 0; i < n; i += len) {
            Complex w = { 1.0f, 0.0f };

            for (int j = 0; j < len / 2; j++) {
                /* Butterfly operation */
                Complex u = buf[i + j];
                Complex v = {
                    buf[i + j + len/2].r * w.r - buf[i + j + len/2].i * w.i,
                    buf[i + j + len/2].r * w.i + buf[i + j + len/2].i * w.r
                };
                buf[i + j]          = (Complex){ u.r + v.r, u.i + v.i };
                buf[i + j + len/2]  = (Complex){ u.r - v.r, u.i - v.i };

                /* Advance twiddle factor */
                float wr = w.r * wlen.r - w.i * wlen.i;
                float wi = w.r * wlen.i + w.i * wlen.r;
                w.r = wr;
                w.i = wi;
            }
        }
    }

    /* Normalize output for IFFT */
    if (inverse) {
        for (int i = 0; i < n; i++) {
            buf[i].r /= (float)n;
            buf[i].i /= (float)n;
        }
    }
}

/* ─────────────────────────────────────────────────────────────────────
   hann_window
   Apply a Hann window to a block of samples before FFT.
   This tapers the edges of each window to zero, which prevents
   "spectral leakage" — frequencies bleeding into the wrong bins.
   ───────────────────────────────────────────────────────────────────── */
static void hann_window(Complex *block, int n)
{
    for (int i = 0; i < n; i++) {
        float w = 0.5f * (1.0f - cosf(2.0f * (float)M_PI * i / (float)(n - 1)));
        block[i].r *= w;
        block[i].i *= w;
    }
}

/* ─────────────────────────────────────────────────────────────────────
   apply_pitch_shift
   Core engine: shifts all frequency bins by a ratio, then reconstructs.

   HOW BIN SHIFTING WORKS:
     The FFT gives us FFT_SIZE bins.
     Bin k represents frequency: k * (sample_rate / FFT_SIZE)
     At 8000 Hz with FFT_SIZE=1024: bin 1 = 7.8 Hz, bin 56 = 437 Hz, etc.

     To shift pitch UP by ratio R:
       For each output bin k_out, copy from input bin k_in = k_out / R
       (we're stretching the spectrum toward higher frequencies)

     To shift pitch DOWN:
       k_in = k_out / R  where R < 1.0  (compresses toward lower freqs)

     We interpolate between adjacent bins for smoother results.
   ───────────────────────────────────────────────────────────────────── */
static DSP_Err apply_pitch_shift(DSP_Buffer *buf, float ratio)
{
    if (buf == NULL)      return DSP_ERR_NULL;
    if (buf->length == 0) return DSP_ERR_EMPTY;

    /* Working buffers — static to avoid stack overflow on STM32 */
    static Complex fft_buf[FFT_SIZE];
    static Complex shifted[FFT_SIZE];
    static float   output_accum[DSP_MAX_SAMPLES + FFT_SIZE];
    static float   window_sum  [DSP_MAX_SAMPLES + FFT_SIZE];

    /* Zero the accumulation buffers */
    memset(output_accum, 0, sizeof(output_accum));
    memset(window_sum,   0, sizeof(window_sum));

    int num_samples = (int)buf->length;

    /* ── Process audio in overlapping windows ── */
    for (int frame_start = 0;
         frame_start + FFT_SIZE <= num_samples;
         frame_start += HOP_SIZE)
    {
        /* Load this window's samples into fft_buf as complex numbers
           (imaginary part = 0, real part = audio sample) */
        for (int i = 0; i < FFT_SIZE; i++) {
            fft_buf[i].r = (float)buf->data[frame_start + i];
            fft_buf[i].i = 0.0f;
        }

        /* Apply Hann window to reduce spectral leakage */
        hann_window(fft_buf, FFT_SIZE);

        /* Bit-reverse copy then FFT → now in frequency domain */
        Complex temp[FFT_SIZE];
        bit_reverse_copy(fft_buf, temp, FFT_SIZE);
        memcpy(fft_buf, temp, sizeof(Complex) * FFT_SIZE);
        fft_inplace(fft_buf, FFT_SIZE, 0);

        /* ── Shift the bins ── */
        memset(shifted, 0, sizeof(Complex) * FFT_SIZE);

        for (int k_out = 0; k_out < FFT_SIZE; k_out++) {
            /* Where in the input spectrum does this output bin come from? */
            float k_in_f = (float)k_out / ratio;
            int   k_in   = (int)k_in_f;

            /* Skip if out of range */
            if (k_in < 0 || k_in >= FFT_SIZE - 1) continue;

            /* Linear interpolation between adjacent bins for smoothness */
            float frac = k_in_f - (float)k_in;
            shifted[k_out].r = fft_buf[k_in].r * (1.0f - frac)
                              + fft_buf[k_in + 1].r * frac;
            shifted[k_out].i = fft_buf[k_in].i * (1.0f - frac)
                              + fft_buf[k_in + 1].i * frac;
        }

        /* IFFT → back to time domain */
        Complex temp2[FFT_SIZE];
        bit_reverse_copy(shifted, temp2, FFT_SIZE);
        memcpy(shifted, temp2, sizeof(Complex) * FFT_SIZE);
        fft_inplace(shifted, FFT_SIZE, 1);

        /* Apply Hann window again (overlap-add synthesis window) */
        hann_window(shifted, FFT_SIZE);

        /* Overlap-add into the output accumulator */
        for (int i = 0; i < FFT_SIZE; i++) {
            output_accum[frame_start + i] += shifted[i].r;
            /* Track window sum for normalization */
            float w = 0.5f * (1.0f - cosf(2.0f * (float)M_PI * i / (float)(FFT_SIZE - 1)));
            window_sum[frame_start + i] += w * w;
        }
    }

    /* ── Normalize and write back to buffer ── */
    for (int i = 0; i < num_samples; i++) {
        float val = (window_sum[i] > 1e-6f)
                    ? output_accum[i] / window_sum[i]
                    : output_accum[i];

        /* Clamp to int16 range */
        if      (val >  32767.0f) val =  32767.0f;
        else if (val < -32768.0f) val = -32768.0f;

        buf->data[i] = (dsp_sample_t)val;
    }

    return DSP_OK;
}

/* ─────────────────────────────────────────────────────────────────────
   Public API — called from main.c when the pitch buttons are pressed
   ───────────────────────────────────────────────────────────────────── */

/**
 * @brief Shift pitch up by PITCH_UP_SEMITONES (default: 2 semitones).
 *        Speed and duration of audio are unchanged.
 * @param buf  Audio buffer to process in-place.
 * @return DSP_OK on success.
 */
DSP_Err pitch_up(DSP_Buffer *buf)
{
    return apply_pitch_shift(buf, PITCH_UP_RATIO);
}

/**
 * @brief Shift pitch down by PITCH_DOWN_SEMITONES (default: 2 semitones).
 *        Speed and duration of audio are unchanged.
 * @param buf  Audio buffer to process in-place.
 * @return DSP_OK on success.
 */
DSP_Err pitch_down(DSP_Buffer *buf)
{
    return apply_pitch_shift(buf, PITCH_DOWN_RATIO);
}
