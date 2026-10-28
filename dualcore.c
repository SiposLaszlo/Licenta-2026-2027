
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/mutex.h"
#include "pico/cyw43_arch.h"

#include "hardware/adc.h"

#include "lwip/apps/mqtt.h"
#include "lwip/ip_addr.h"

// WiFi settings
#define WIFI_SSID "DIGI-YyN6"
#define WIFI_PASSWORD "R7tkT2PCUg"

// MQTT settings
#define MQTT_BROKER_IP "192.168.1.128"
#define MQTT_PORT 1883
#define MQTT_TOPIC "nilm/pico2w/adc"

// ADC settings
#define ADC_PIN 26
#define N 100
#define SAMPLE_INTERVAL_US 10000

static mqtt_client_t *client;
static volatile bool mqtt_connected = false;

// Shared data between cores
static mutex_t data_mutex;
static float shared_voltage = 0.0f;
static uint32_t shared_sequence = 0;

// MQTT connection callback
static void mqtt_connection_callback(
    mqtt_client_t *client,
    void *arg,
    mqtt_connection_status_t status
) {
    if (status == MQTT_CONNECT_ACCEPTED) {
        mqtt_connected = true;
        printf("MQTT connected!\n");
    } else {
        mqtt_connected = false;
        printf("MQTT status: %d\n", status);
    }
}

// MQTT publish callback
static void mqtt_publish_callback(void *arg, err_t result) {
    if (result == ERR_OK) {
        printf("Message delivered to MQTT stack!\n");
    } else {
        printf("Publish error: %d\n", result);
    }
}

// CORE 1 - ADC acquisition
void core1_entry() {
    adc_init();
    adc_gpio_init(ADC_PIN);
    adc_select_input(0);

    while (true) {
        float sum = 0.0f;

        uint64_t next_sample = time_us_64();

        for (int i = 0; i < N; i++) {
            while (time_us_64() < next_sample) {
                tight_loop_contents();
            }

            uint16_t raw = adc_read();
            float voltage = raw * 3.3f / 4096.0f;

            sum += voltage;
            next_sample += SAMPLE_INTERVAL_US;
        }

        float mean = sum / N;

        // Protect shared data
        mutex_enter_blocking(&data_mutex);

        shared_voltage = mean;
        shared_sequence++;

        mutex_exit(&data_mutex);
    }
}

// CORE 0 - WiFi and MQTT
int main() {
    stdio_init_all();
    sleep_ms(3000);

    printf("\n=== TASK 8: DUAL CORE ===\n");

    mutex_init(&data_mutex);

    // Start Core 1
    multicore_launch_core1(core1_entry);

    printf("Core 1 started!\n");

    // Initialize WiFi
    if (cyw43_arch_init() != 0) {
        printf("WiFi initialization failed!\n");
        return 1;
    }

    cyw43_arch_enable_sta_mode();

    printf("Connecting to WiFi...\n");

    int wifi_result =
        cyw43_arch_wifi_connect_timeout_ms(
            WIFI_SSID,
            WIFI_PASSWORD,
            CYW43_AUTH_WPA2_MIXED_PSK,
            30000
        );

    if (wifi_result != 0) {
        printf("WiFi error: %d\n", wifi_result);
        return 1;
    }

    printf("WiFi connected!\n");

    ip_addr_t broker_ip;

    if (!ipaddr_aton(MQTT_BROKER_IP, &broker_ip)) {
        printf("Invalid broker IP!\n");
        return 1;
    }

    cyw43_arch_lwip_begin();
    client = mqtt_client_new();
    cyw43_arch_lwip_end();

    if (client == NULL) {
        printf("MQTT client creation failed!\n");
        return 1;
    }

    struct mqtt_connect_client_info_t info = {0};
    info.client_id = "pico2w-dualcore";
    info.keep_alive = 60;

    cyw43_arch_lwip_begin();

    err_t err = mqtt_client_connect(
        client,
        &broker_ip,
        MQTT_PORT,
        mqtt_connection_callback,
        NULL,
        &info
    );

    cyw43_arch_lwip_end();

    if (err != ERR_OK) {
        printf("MQTT connection error: %d\n", err);
        return 1;
    }

    printf("Waiting for MQTT connection...\n");

    for (int i = 0; i < 150 && !mqtt_connected; i++) {
        sleep_ms(100);
    }

    if (!mqtt_connected) {
        printf("MQTT connection timeout!\n");
        return 1;
    }

    uint32_t last_sequence = 0;

    while (true) {
        float voltage;
        uint32_t sequence;

        // Copy shared data safely
        mutex_enter_blocking(&data_mutex);

        voltage = shared_voltage;
        sequence = shared_sequence;

        mutex_exit(&data_mutex);

        if (sequence != last_sequence && mqtt_connected) {
            last_sequence = sequence;

            char json[128];

            snprintf(
                json,
                sizeof(json),
                "{\"device\":\"pico2w\","
                "\"core\":1,"
                "\"samples\":%d,"
                "\"mean_voltage\":%.4f,"
                "\"sequence\":%lu}",
                N,
                voltage,
                (unsigned long)sequence
            );

            printf("Sending: %s\n", json);

            cyw43_arch_lwip_begin();

            err_t result = mqtt_publish(
                client,
                MQTT_TOPIC,
                json,
                strlen(json),
                0,
                0,
                mqtt_publish_callback,
                NULL
            );

            cyw43_arch_lwip_end();

            if (result != ERR_OK) {
                printf("Publish request error: %d\n", result);
            }
        }

        sleep_ms(50);
    }
}
