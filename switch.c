
#include <stdio.h>
#include "pico/stdlib.h"

#define SWITCH_PIN 2
#define DEBOUNCE_MS 50

int main() {
    stdio_init_all();

    gpio_init(SWITCH_PIN);
    gpio_set_dir(SWITCH_PIN, GPIO_IN);
    gpio_pull_up(SWITCH_PIN);

    bool last_reading = gpio_get(SWITCH_PIN);
    bool stable_state = last_reading;

    uint32_t last_change = to_ms_since_boot(get_absolute_time());
    int count = 0;

    while (true) {
        bool reading = gpio_get(SWITCH_PIN);
        uint32_t now = to_ms_since_boot(get_absolute_time());

        if (reading != last_reading) {
            last_change = now;
        }

        if ((uint32_t)(now - last_change) >= DEBOUNCE_MS) {
            if (reading != stable_state) {
                stable_state = reading;

                if (stable_state == 0) {
                    count++;
                    printf("Activation: %d | Timestamp: %lu ms\n",
                           count, (unsigned long)now);
                }
            }
        }

        last_reading = reading;
        sleep_ms(5);
    }
}
