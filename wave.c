
#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/adc.h"
#include "hardware/clocks.h"

#define PWM_PIN 16
#define ADC_PIN 26
#define PWM_FREQ 100000
#define SINE_FREQ 1.0f
#define SAMPLES 100

#define PI 3.14159265f

int main() {
    stdio_init_all();

    // Initialize PWM output
    gpio_set_function(PWM_PIN, GPIO_FUNC_PWM);

    uint slice = pwm_gpio_to_slice_num(PWM_PIN);

    uint32_t sys_clock = clock_get_hz(clk_sys);
    uint16_t wrap = sys_clock / PWM_FREQ - 1;

    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, 1.0f);
    pwm_config_set_wrap(&config, wrap);

    pwm_init(slice, &config, true);

    // Initialize ADC0
    adc_init();
    adc_gpio_init(ADC_PIN);
    adc_select_input(0);

    sleep_ms(2000);

    printf("Time_ms,Voltage_V\n");

    uint64_t start = time_us_64();

    while (true) {
        uint64_t elapsed = time_us_64() - start;
        float t = elapsed / 1000000.0f;

        // Generate 1 Hz sine wave
        float sine = (sinf(2.0f * PI * SINE_FREQ * t) + 1.0f) / 2.0f;

        // Convert sine into PWM duty cycle
        uint16_t duty = (uint16_t)(sine * (wrap + 1));
        pwm_set_gpio_level(PWM_PIN, duty);

        // Read filtered analog signal
        uint16_t raw = adc_read();
        float voltage = raw * 3.3f / 4096.0f;

        printf("%lu,%.3f\n",
               (unsigned long)(elapsed / 1000),
               voltage);

        sleep_ms(10);
    }
}
