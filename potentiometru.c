
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"

#define ADC_PIN 26

int main() {
    stdio_init_all();

    // Initialize ADC
    adc_init();

    // Enable GP26 as ADC input
    adc_gpio_init(ADC_PIN);

    // Select ADC0
    adc_select_input(0);

    while (true) {
        // Read 12-bit ADC value (0-4095)
        uint16_t raw = adc_read();

        // Convert ADC value to voltage
        float voltage = raw * 3.3f / 4096.0f;

        printf("ADC: %u | Voltage: %.3f V\n",
               raw, voltage);

        sleep_ms(200);
    }
}
