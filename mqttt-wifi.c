
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/apps/mqtt.h"
#include "lwip/ip_addr.h"

#define WIFI_SSID "DIGI-YyN6"
#define WIFI_PASSWORD "R7tkT2PCUg"

#define MQTT_BROKER_IP "192.168.1.128"
#define MQTT_PORT 1883
#define MQTT_TOPIC "nilm/pico2w/data"

static mqtt_client_t *client;
static volatile bool connected = false;

static void publish_callback(void *arg, err_t result) {
    if (result == ERR_OK) {
        printf("Message sent successfully!\n");
    } else {
        printf("Publish error: %d\n", result);
    }
}

static void connection_callback(
    mqtt_client_t *client,
    void *arg,
    mqtt_connection_status_t status
) {
    if (status == MQTT_CONNECT_ACCEPTED) {
        connected = true;
        printf("MQTT connected!\n");
    } else {
        connected = false;
        printf("MQTT status: %d\n", status);
    }
}

int main() {
    stdio_init_all();
    sleep_ms(3000);

    printf("\n=== TASK 7: WiFi + MQTT ===\n");

    if (cyw43_arch_init() != 0) {
        printf("WiFi initialization failed!\n");
        return 1;
    }

    cyw43_arch_enable_sta_mode();

    printf("Connecting to WiFi...\n");

    int result = cyw43_arch_wifi_connect_timeout_ms(
        WIFI_SSID,
        WIFI_PASSWORD,
        CYW43_AUTH_WPA2_AES_PSK,
        30000
    );

    if (result != 0) {
        printf("WiFi connection failed: %d\n", result);
        cyw43_arch_deinit();
        return 1;
    }

    printf("WiFi connected!\n");

    ip_addr_t broker_ip;
    if (!ipaddr_aton(MQTT_BROKER_IP, &broker_ip)) {
        printf("Invalid MQTT broker IP!\n");
        cyw43_arch_deinit();
        return 1;
    }

    cyw43_arch_lwip_begin();
    client = mqtt_client_new();
    cyw43_arch_lwip_end();

    if (client == NULL) {
        printf("MQTT client creation failed!\n");
        cyw43_arch_deinit();
        return 1;
    }

    struct mqtt_connect_client_info_t info = {0};
    info.client_id = "pico2w-nilm";
    info.keep_alive = 60;

    printf("Connecting to MQTT broker...\n");

    cyw43_arch_lwip_begin();

    err_t err = mqtt_client_connect(
        client,
        &broker_ip,
        MQTT_PORT,
        connection_callback,
        NULL,
        &info
    );

    cyw43_arch_lwip_end();

    if (err != ERR_OK) {
        printf("MQTT connection error: %d\n", err);
        cyw43_arch_deinit();
        return 1;
    }

    // Wait for MQTT connection
    for (int i = 0; i < 150 && !connected; i++) {
        sleep_ms(100);
    }

    if (!connected) {
        printf("MQTT connection timeout!\n");
        cyw43_arch_deinit();
        return 1;
    }

    int counter = 0;

    while (true) {
        if (connected) {
            counter++;

            char message[128];

            snprintf(
                message,
                sizeof(message),
                "{\"device\":\"pico2w\","
                "\"counter\":%d,"
                "\"uptime_ms\":%lu}",
                counter,
                (unsigned long)to_ms_since_boot(
                    get_absolute_time()
                )
            );

            printf("Sending: %s\n", message);

            cyw43_arch_lwip_begin();

            err_t pub_result = mqtt_publish(
                client,
                MQTT_TOPIC,
                message,
                strlen(message),
                0,
                0,
                publish_callback,
                NULL
            );

            cyw43_arch_lwip_end();

            if (pub_result != ERR_OK) {
                printf("Publish request failed: %d\n",
                       pub_result);
            }
        }

        sleep_ms(2000);
    }
}
