
#include <stdio.h>
#include <stdint.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"

#define ADC_PIN 26
#define N 1000
#define INTERVAL_US 313

int main() {
    stdio_init_all();

    adc_init();
    adc_gpio_init(ADC_PIN);
    adc_select_input(0);

    sleep_ms(2000);

    // TEST 1: Without timing control
    uint16_t sample = 0;

    uint64_t start = time_us_64();

    for (int i = 0; i < N; i++) {
        sample = adc_read();
    }

    uint64_t end = time_us_64();
    uint64_t elapsed = end - start;

    double rate = (double)N * 1000000.0 / elapsed;

    printf("TEST 1 - Without timing control\n");
    printf("Elapsed: %llu us\n",
           (unsigned long long)elapsed);
    printf("Sampling rate: %.2f Hz\n\n", rate);

    // TEST 2: Controlled sampling
    uint64_t timestamps[N];

    uint64_t next_sample = time_us_64();

    for (int i = 0; i < N; i++) {
        while (time_us_64() < next_sample) {
            tight_loop_contents();
        }

        timestamps[i] = time_us_64();
        sample = adc_read();

        next_sample += INTERVAL_US;
    }

    uint64_t duration = timestamps[N-1] - timestamps[0];

    double controlled_rate =
        (double)(N-1) * 1000000.0 / duration;

    printf("TEST 2 - Controlled sampling\n");
    printf("Sampling rate: %.2f Hz\n\n", controlled_rate);

    // TEST 3: Measure jitter
    uint64_t min_dt = UINT64_MAX;
    uint64_t max_dt = 0;
    uint64_t max_jitter = 0;

    for (int i = 1; i < N; i++) {
        uint64_t dt = timestamps[i] - timestamps[i-1];

        if (dt < min_dt) min_dt = dt;
        if (dt > max_dt) max_dt = dt;

        uint64_t jitter = dt > INTERVAL_US
            ? dt - INTERVAL_US
            : INTERVAL_US - dt;

        if (jitter > max_jitter)
            max_jitter = jitter;
    }

    printf("TEST 3 - Jitter\n");
    printf("Target interval: %d us\n", INTERVAL_US);
    printf("Minimum interval: %llu us\n",
           (unsigned long long)min_dt);
    printf("Maximum interval: %llu us\n",
           (unsigned long long)max_dt);
    printf("Maximum jitter: %llu us\n",
           (unsigned long long)max_jitter);

   while (true) {
    printf("\nPico 2 W functioneaza!\n");
    printf("Sampling rate: %.2f Hz\n", controlled_rate);
    printf("Maximum jitter: %llu us\n",
           (unsigned long long)max_jitter);

    sleep_ms(2000);
}
}
