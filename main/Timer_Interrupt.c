#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/timer.h"
#include "esp_log.h"

#define BLINK_GPIO GPIO_NUM_2
#define TIMER_BASE_CLK 80000000 // clock frequecy 80 MHz which is APB_CLK
#define TIMER_DIVIDER 80  // Hardware timer clock divider
#define TIMER_SCALE (TIMER_BASE_CLK / TIMER_DIVIDER)  // convert counter value to seconds which is (80000000/8 = 100000)
#define TIMER_INTERVAL_SEC    (0.07) // Sample test interval for the timer



void IRAM_ATTR timer_group0_isr(void *para) {
    int timer_idx = (int) para;

    // Clear the interrupt
    if (timer_idx == TIMER_0) {
        timer_group_clr_intr_status_in_isr(TIMER_GROUP_0, TIMER_0);
    } else if (timer_idx == TIMER_1) {
        timer_group_clr_intr_status_in_isr(TIMER_GROUP_0, TIMER_1);
    }

    // Toggle the LED
    static bool led_on = false;
    gpio_set_level(BLINK_GPIO, led_on);
    led_on = !led_on;  // Toggle the LED state

    // Re-enable the alarm
    if (timer_idx == TIMER_0) {
        timer_group_enable_alarm_in_isr(TIMER_GROUP_0, TIMER_0);
    } else if (timer_idx == TIMER_1) {
        timer_group_enable_alarm_in_isr(TIMER_GROUP_0, TIMER_1);
    }
    
}

static void example_tg0_timer_init(int timer_idx, bool auto_reload, double timer_interval_sec) {
    timer_config_t config = {
        .divider = TIMER_DIVIDER,
        .counter_dir = TIMER_COUNT_UP,
        .counter_en = TIMER_PAUSE,
        .alarm_en = TIMER_ALARM_EN,
        .auto_reload = auto_reload,
    };

    // Initialize the timer
    timer_init(TIMER_GROUP_0, timer_idx, &config);
    
    // Set the counter value to 0
    timer_set_counter_value(TIMER_GROUP_0, timer_idx, 0x00000000ULL);

    // Set the alarm value to the timer interval in seconds
    timer_set_alarm_value(TIMER_GROUP_0, timer_idx, timer_interval_sec * TIMER_SCALE);

    // Enable the interrupt for the timer
    timer_enable_intr(TIMER_GROUP_0, timer_idx);

    // Register the ISR handler
    timer_isr_register(TIMER_GROUP_0, timer_idx, timer_group0_isr, (void *) timer_idx, ESP_INTR_FLAG_IRAM, NULL);

    // Start the timer
    timer_start(TIMER_GROUP_0, timer_idx);
}

void app_main(void) {

    // Configure the GPIO pin
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);

    // Initialize the timer
    example_tg0_timer_init(TIMER_0, true, TIMER_INTERVAL_SEC);  // Timer 0, auto-reload

}
