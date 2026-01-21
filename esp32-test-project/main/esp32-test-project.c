#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

// GPIO configuration
#define A3144_SENSOR_PIN    GPIO_NUM_4  // Pin connected to A3144 OUT
#define LED_PIN             GPIO_NUM_2  // Built-in LED

static const char *TAG = "A3144-Example";

void app_main(void)
{
    // Configure GPIO pins
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << A3144_SENSOR_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,  // A3144 has open-drain output
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);

    // Configure LED as output
    gpio_config_t led_conf = {
        .pin_bit_mask = (1ULL << LED_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&led_conf);

    ESP_LOGI(TAG, "A3144 Hall Effect Sensor Example");
    ESP_LOGI(TAG, "================================");
    ESP_LOGI(TAG, "Sensor connected to GPIO %d", A3144_SENSOR_PIN);
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "The A3144 sensor output is active LOW:");
    ESP_LOGI(TAG, "  - HIGH (1) = No magnetic field detected");
    ESP_LOGI(TAG, "  - LOW (0)  = Magnetic field detected");
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "Use the sensor controls in Wokwi to simulate magnetic field");
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "Starting polling loop...");

    int prev_state = -1;
    int detection_count = 0;
    int loop_count = 0;

    while (1) {
        // Read sensor state
        int sensor_state = gpio_get_level(A3144_SENSOR_PIN);

        // Print current state every 10 iterations (every second)
        if (loop_count % 10 == 0) {
            ESP_LOGI(TAG, "Polling... Sensor state: %d", sensor_state);
        }
        loop_count++;

        // Detect state changes
        if (sensor_state != prev_state) {
            if (sensor_state == 0) {
                // LOW = Magnetic field detected
                detection_count++;
                ESP_LOGI(TAG, ">>> MAGNETIC FIELD DETECTED! (Count: %d) <<<", detection_count);
                gpio_set_level(LED_PIN, 1);  // Turn LED on
            } else {
                // HIGH = No magnetic field
                ESP_LOGI(TAG, ">>> No magnetic field <<<");
                gpio_set_level(LED_PIN, 0);  // Turn LED off
            }
            prev_state = sensor_state;
        }

        // Small delay to prevent busy-waiting (100ms = 10 polls per second)
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
