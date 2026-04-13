/**
 * @file main.c
 * @brief Test harness for all EE14 DSP effects.
 *
 * HOW TO USE:
 *   1. Flash this to your Nucleo-L432KC (same board from every lab).
 *   2. Open a serial terminal at 9600 baud (same as Lab 6/7/8/9).
 *   3. You'll see before/after sample values printed for every effect.
 *   4. To test a specific effect, find its section below and confirm
 *      the numbers look right (see expected output comments).
 *
 * WHEN HARDWARE ARRIVES:
 *   Replace generate_test_signal() with real ADC capture.
 *   All effect calls stay exactly the same.
 */

#include <stdio.h>
#include <stm32l432xx.h>
#include "ee14lib.h"
#include "dsp_lib.h"

/* Required by printf() to send output over the serial terminal */
int _write(int file, char *data, int len) {
    return serial_write(USART2, data, len);
}

/* ── How many samples to print per test ─────────────────────────────── */
#define PRINT_N  8

static void print_samples(const DSP_Buffer *buf, const char *label)
{
    printf("\r\n-- %s --\r\n", label);
    for (int i = 0; i < PRINT_N && (size_t)i < buf->length; i++)
        printf("  [%d]: %d\r\n", i, (int)buf->data[i]);
    printf("  length: %d samples\r\n", (int)buf->length);
}

/* Single global buffer — static so it goes in BSS, not on the stack */
static DSP_Buffer buf;

int main(void)
{
    host_serial_init(9600);
    printf("\r\n==============================\r\n");
    printf("  EE14 DSP Effects Test Suite\r\n");
    printf("==============================\r\n");

    /* ── TEST 1: Volume Up ───────────────────────────────────────────── */
    printf("\r\n[1] VOLUME UP\r\n");
    generate_test_signal(&buf, 440.0f, 0.5f);
    print_samples(&buf, "Original");
    volume_up(&buf);
    print_samples(&buf, "After volume_up (expect ~2x each value)");

    /* ── TEST 2: Volume Down ─────────────────────────────────────────── */
    printf("\r\n[2] VOLUME DOWN\r\n");
    generate_test_signal(&buf, 440.0f, 0.5f);
    print_samples(&buf, "Original");
    volume_down(&buf);
    print_samples(&buf, "After volume_down (expect ~0.5x each value)");

    /* ── TEST 3: Reverse ─────────────────────────────────────────────── */
    printf("\r\n[3] REVERSE\r\n");
    generate_test_signal(&buf, 440.0f, 0.5f);
    printf("  First sample before: %d\r\n", (int)buf.data[0]);
    printf("  Last  sample before: %d\r\n", (int)buf.data[buf.length - 1]);
    reverse(&buf);
    printf("  First sample after:  %d\r\n", (int)buf.data[0]);
    printf("  Last  sample after:  %d\r\n", (int)buf.data[buf.length - 1]);
    printf("  (first/last should be swapped)\r\n");

    /* ── TEST 4: Halve ───────────────────────────────────────────────── */
    printf("\r\n[4] HALVE\r\n");
    generate_test_signal(&buf, 440.0f, 0.5f);
    printf("  Length before: %d\r\n", (int)buf.length);
    halve(&buf);
    printf("  Length after:  %d\r\n", (int)buf.length);
    printf("  (should be exactly half)\r\n");

    /* ── TEST 5: Echo ────────────────────────────────────────────────── */
    printf("\r\n[5] ECHO\r\n");
    generate_test_signal(&buf, 440.0f, 0.5f);
    print_samples(&buf, "Original");
    echo_effect(&buf);
    print_samples(&buf, "After echo (early samples same, later ones show echo mixed in)");

    printf("\r\n==============================\r\n");
    printf("  All tests complete.\r\n");
    printf("==============================\r\n");

    while(1) {}
    return 0;
}
