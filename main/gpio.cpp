#include "gpio.h" // Include the GPIO header file

/**
 * @brief Initialize GPIO pins used for encoder and servo drive signals.
 *
 * This function configures GPIO pins for encoder channels A, B, Z, and servo drive signals
 * SIGIN2 and VREF. The pins are configured as inputs or outputs based on their usage.
 */
extern "C" void init_gpio(Gpio *gpio_struct)
{
    // GPIO Inputs Configuration
    // ----------------------------------------------
    // Configure GPIO input pins for encoder channels A, B, Z, and servo drive signals
    gpio_config_t gpio_conf_input; // Configuration structure for input pins

    // Specify the GPIO pins to be configured as inputs using a bitmask
    gpio_conf_input.pin_bit_mask = (1ULL << gpio_struct->encoder_pin_a) | (1ULL << gpio_struct->encoder_pin_b) | (1ULL << gpio_struct->encoder_pin_z);

    gpio_conf_input.mode = GPIO_MODE_INPUT;               // Set pins as input
    gpio_conf_input.pull_up_en = GPIO_PULLUP_ENABLE;      // Enable pull-up resistor
    gpio_conf_input.pull_down_en = GPIO_PULLDOWN_DISABLE; // Disable pull-down resistor
    gpio_conf_input.intr_type = GPIO_INTR_ANYEDGE;        // Disable interrupts

    gpio_config(&gpio_conf_input); // Apply configuration to input pins

    // GPIOs for the ouputs are initialized in the pwm.c file/pwm code
    /*
        // GPIO Outputs Configuration
        // ----------------------------------------------
        // Configure GPIO output pins for servo drive signals SIGIN2 and VREF
        gpio_config_t gpio_conf_output; // Configuration structure for output pins

        // Specify the GPIO pins to be configured as outputs using a bitmask
        gpio_conf_output.pin_bit_mask = (1ULL << gpio_struct->servo_pin_sign2) | (1ULL << gpio_struct->servo_pin_vref);

        gpio_conf_output.mode = GPIO_MODE_OUTPUT;              // Set pins as output
        gpio_conf_output.pull_up_en = GPIO_PULLUP_ENABLE;      // Disable pull-up resistor
        gpio_conf_output.pull_down_en = GPIO_PULLDOWN_DISABLE; // Disable pull-down resistor
        gpio_conf_output.intr_type = GPIO_INTR_DISABLE;        // Disable interrupts

        gpio_config(&gpio_conf_output); // Apply configuration to output pins
    */

    // GPIO Outputs Configuration
    // ----------------------------------------------
    // Configure GPIO output pins for servo drive signals SIGIN2 and VREF
    gpio_config_t gpio_conf_output; // Configuration structure for output pins

    // Specify the GPIO pins to be configured as outputs using a bitmask
    gpio_conf_output.pin_bit_mask = (1ULL << DIRECTION_PIN);

    gpio_conf_output.mode = GPIO_MODE_OUTPUT;              // Set pins as output
    gpio_conf_output.pull_up_en = GPIO_PULLUP_DISABLE;     // Disable pull-up resistor
    gpio_conf_output.pull_down_en = GPIO_PULLDOWN_DISABLE; // Disable pull-down resistor
    gpio_conf_output.intr_type = GPIO_INTR_DISABLE;        // Disable interrupts

    gpio_config(&gpio_conf_output); // Apply configuration to output pins
}

/*
// Getter functions for GPIO pins
gpio_num_t set_ENCODER_PIN_A()
{
    return gpio_struct.encoder_pin_a;
}

gpio_num_t get_ENCODER_PIN_B()
{
    return encoder_pin_b;
}

gpio_num_t get_ENCODER_PIN_Z()
{
    return encoder_pin_z;
}

gpio_num_t get_SERVO_PIN_SIGIN2()
{
    return servo_pin_sign2;
}

gpio_num_t get_SERVO_PIN_VREF()
{
    return servo_pin_vref;
}

// Setter functions for gpio pins
// set functions for GPIO pins
gpio_num_t set_ENCODER_PIN_A(gpio_num_t gpio_pin_num)
{
    encoder_pin_a = gpio_pin_num;
}

gpio_num_t set_ENCODER_PIN_B(gpio_num_t gpio_pin_num)
{
    encoder_pin_b = gpio_pin_num;
}

gpio_num_t get_ENCODER_PIN_Z(gpio_num_t gpio_pin_num)
{
    encoder_pin_z = gpio_pin_num;
}

gpio_num_t get_SERVO_PIN_SIGIN2(gpio_num_t gpio_pin_num)
{
    servo_pin_sign2 = gpio_pin_num;
}

gpio_num_t get_SERVO_PIN_VREF(gpio_num_t gpio_pin_num)
{
    servo_pin_vref = gpio_pin_num;
}
*/