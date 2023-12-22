#include "gpio.h" // Include the GPIO header file

/**
 * @brief Initialize GPIO pins used for encoder and servo drive signals.
 *
 * This function configures GPIO pins for encoder channels A, B, Z, and servo drive signals
 * SIGIN2 and VREF. The pins are configured as inputs or outputs based on their usage.
 */
extern "C" void init_gpio(gpio_num_t gpios[])
{
    // GPIO Inputs Configuration
    // ----------------------------------------------
    // Configure GPIO input pins for encoder channels A, B, Z, and servo drive signals
    gpio_config_t gpio_conf_input = {}; // Initialize the structure to zero

    // Specify the GPIO pins to be configured as inputs using a bitmask
    for (uint8_t i = 0; i < sizeof(gpios) / sizeof(gpio_num_t); i++)
    {
        gpio_conf_input.pin_bit_mask |= (1ULL << gpios[i]);
    }

    gpio_conf_input.mode = GPIO_MODE_INPUT;               // Set pins as input
    gpio_conf_input.pull_up_en = GPIO_PULLUP_ENABLE;      // Enable pull-up resistor
    gpio_conf_input.pull_down_en = GPIO_PULLDOWN_DISABLE; // Disable pull-down resistor
    gpio_conf_input.intr_type = GPIO_INTR_ANYEDGE;        // Disable interrupts

    gpio_config(&gpio_conf_input); // Apply configuration to input pins
}