
#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"

#define PWM_PIN 16
#define ADC_PIN 26

#define N 200
#define SAMPLE_INTERVAL_US 10000
#define SIGNAL_FREQ 1.0f
#define PI 3.14159265358979323846f

float samples[N];

int main() {
    stdio_init_all();

    // Initialize ADC
    adc_init();
    adc_gpio_init(ADC_PIN);
    adc_select_input(0);

    // Initialize PWM at approximately 100 kHz
    gpio_set_function(PWM_PIN, GPIO_FUNC_PWM);

    uint slice = pwm_gpio_to_slice_num(PWM_PIN);

    uint32_t clock = clock_get_hz(clk_sys);
    uint16_t wrap = clock / 100000 - 1;

    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, 1.0f);
    pwm_config_set_wrap(&config, wrap);

    pwm_init(slice, &config, true);
    pwm_set_gpio_level(PWM_PIN, (wrap + 1) / 2);

    sleep_ms(3000);

    while (true) {

        // STEP 1: Generate and capture 200 samples
        uint64_t start = time_us_64();

        for (int i = 0; i < N; i++) {
            uint64_t target =
                start + (uint64_t)i * SAMPLE_INTERVAL_US;

            while (time_us_64() < target) {
                tight_loop_contents();
            }

            float t = i * 0.01f;

            float sine =
                (sinf(2.0f * PI * SIGNAL_FREQ * t) + 1.0f)
                / 2.0f;

            uint16_t duty =
                (uint16_t)(sine * (wrap + 1));

            pwm_set_gpio_level(PWM_PIN, duty);

            uint16_t raw = adc_read();
            samples[i] = raw * 3.3f / 4096.0f;
        }

        // STEP 2: Calculate mean
        float sum = 0;

        for (int i = 0; i < N; i++) {
            sum += samples[i];
        }

        float mean = sum / N;

        // STEP 3: Calculate RMS
        float sum_sq = 0;
        float sum_ac_sq = 0;

        for (int i = 0; i < N; i++) {
            sum_sq += samples[i] * samples[i];

            float ac = samples[i] - mean;
            sum_ac_sq += ac * ac;
        }

        float rms = sqrtf(sum_sq / N);
        float rms_ac = sqrtf(sum_ac_sq / N);

        // STEP 4: DFT at 1 Hz
        // N=200, fs=100 Hz, k=2 corresponds to 1 Hz

        float real = 0;
        float imag = 0;
        int k = 2;

        for (int i = 0; i < N; i++) {
            float angle = 2.0f * PI * k * i / N;
            float ac = samples[i] - mean;

            real += ac * cosf(angle);
            imag -= ac * sinf(angle);
        }

        float amplitude =
            2.0f * sqrtf(real * real + imag * imag) / N;

        // STEP 5: Display results
        printf("\n===== TASK 6 =====\n");
        printf("Samples: %d\n", N);
        printf("Sampling frequency: 100 Hz (target)\n");

        printf("Mean: %.4f V\n", mean);
        printf("RMS total: %.4f V\n", rms);
        printf("RMS AC: %.4f V\n", rms_ac);
        printf("DFT amplitude at 1 Hz: %.4f V\n",
               amplitude);

        printf("==================\n");

        sleep_ms(3000);
    }
}
