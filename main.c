#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

// -------------------- Configuration --------------------
#define LED_PIN             GPIO_NUM_2

#define BLINK_HALF_MS       250        // 250ms ON, 250ms OFF -> 2 Hz blink (500ms period)
#define PRINT_INTERVAL_MS   10000      // Print every 10 seconds

// Theme message (edit if you want a different theme)
#define THEME_NAME          "Audio System Health Monitor"

// -------------------- Tasks --------------------
static void blink_task(void *pvParameters)
{
    (void)pvParameters;

    int led_on = 0;

    // Hard-bonus timing log variables
    uint32_t last_toggle_ms = (uint32_t)(xTaskGetTickCount() * portTICK_PERIOD_MS);

    while (1) {
        led_on = !led_on;

        // Drive LED according to led_on flag
        gpio_set_level(LED_PIN, led_on ? 1 : 0);

        // Hard bonus: log blink period details (dt between toggles)
        uint32_t now_ms = (uint32_t)(xTaskGetTickCount() * portTICK_PERIOD_MS);
        printf("[blink_task] LED=%s | dt=%lu ms\n",
               led_on ? "ON" : "OFF",
               (unsigned long)(now_ms - last_toggle_ms));
        last_toggle_ms = now_ms;

        // Yield to scheduler (no busy-wait)
        vTaskDelay(pdMS_TO_TICKS(BLINK_HALF_MS));
    }
}

static void print_task(void *pvParameters)
{
    (void)pvParameters;

    // Hard-bonus timing log variables
    uint32_t last_print_ms = (uint32_t)(xTaskGetTickCount() * portTICK_PERIOD_MS);

    while (1) {
        uint32_t now_ms = (uint32_t)(xTaskGetTickCount() * portTICK_PERIOD_MS);

        // Theme-related heartbeat message
        printf("[print_task] %s | heartbeat OK | uptime=%lu ms | dt=%lu ms\n",
               THEME_NAME,
               (unsigned long)now_ms,
               (unsigned long)(now_ms - last_print_ms));

        last_print_ms = now_ms;

        // Yield to scheduler (no busy-wait)
        vTaskDelay(pdMS_TO_TICKS(PRINT_INTERVAL_MS));
    }
}

// -------------------- Entry Point --------------------
void app_main(void)
{
    // GPIO Configuration (LED setup)
    gpio_reset_pin(LED_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(LED_PIN, 0);

    printf("RTS Module 1 started: %s\n", THEME_NAME);
    printf("LED pin: GPIO%d | Blink: ~2 Hz | Print: every %d ms\n", 2, PRINT_INTERVAL_MS);

    // Create FreeRTOS tasks
    xTaskCreate(blink_task, "blink_task", 2048, NULL, 1, NULL);
    xTaskCreate(print_task, "print_task", 2048, NULL, 1, NULL);
}
