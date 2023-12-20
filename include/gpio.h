/**
 * @file gpio.h
 * @brief This file contains GPIO pin configurations and initialization function.
 */

#ifndef GPIO_CONFIG_H
#define GPIO_CONFIG_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "driver/gpio.h" // Include the GPIO driver library

    /**
     * @defgroup ENCODER_PINS Encoder GPIO Pins
     * @{
     */
    ////#define ENCODER_PIN_A GPIO_NUM_17 /**< GPIO pin for encoder channel A */
    ////#define ENCODER_PIN_B GPIO_NUM_18 /**< GPIO pin for encoder channel B */
    ////#define ENCODER_PIN_Z GPIO_NUM_8  /**< GPIO pin for encoder channel Z */
    /** @} */

#define MOMENTARY_PUSH_BUTTON_1 GPIO_NUM_12 /**< GPIO pin for servo drive signal SIGIN2 */
#define PADDLE_SHIFTER_SWITCH_1 GPIO_NUM_5  /**< GPIO pin for servo drive signal VREF */
#define PADDLE_SHIFTER_SWITCH_2 GPIO_NUM_8

#define DIRECTION_PIN GPIO_NUM_16

    /**
     * @defgroup SERVO_PINS Servo Drive GPIO Pins
     * @{
     */
    ////#define SERVO_PIN_SIGIN2 GPIO_NUM_15 /**< GPIO pin for servo drive signal SIGIN2 */
    ////#define SERVO_PIN_VREF GPIO_NUM_7    /**< GPIO pin for servo drive signal VREF */

    typedef struct Gpio
    {
        // gpio_num_t encoder_pin_a = GPIO_NUM_17;
        // gpio_num_t encoder_pin_b = GPIO_NUM_18;
        // gpio_num_t encoder_pin_z = GPIO_NUM_8;
        // gpio_num_t servo_pin_sign2 = GPIO_NUM_15;
        // gpio_num_t servo_pin_vref = GPIO_NUM_7;
        gpio_num_t encoder_pin_a;
        gpio_num_t encoder_pin_b;
        gpio_num_t encoder_pin_z;
        gpio_num_t servo_pin_sign2;
        gpio_num_t servo_pin_vref;
    } Gpio;

    /** @} */

    /**
     * @brief Initialize GPIO pins used for encoder and servo drive signals.
     *
     * This function configures the GPIO pins used for encoder channels A, B, and Z,
     * as well as the pins for servo drive signals.
     */
    void init_gpio(Gpio *gpio_struct);
/*
    // Getter functions for GPIO pins
    gpio_num_t get_encoder_pin_a();
    gpio_num_t get_encoder_pin_b();
    gpio_num_t get_encoder_pin_z();
    gpio_num_t get_servo_pin_sigin2();
    gpio_num_t get_servo_pin_vref();

    // Setter functions for GPIO pins
    gpio_num_t set_encoder_pin_a(gpio_num_t gpio_pin_num);
    gpio_num_t set_encoder_pin_b(gpio_num_t gpio_pin_num);
    gpio_num_t set_encoder_pin_z(gpio_num_t gpio_pin_num);
    gpio_num_t set_servo_pin_sigin2(gpio_num_t gpio_pin_num);
    gpio_num_t set_servo_pin_vref(gpio_num_t gpio_pin_num);
*/
#ifdef __cplusplus
}
#endif

#endif // GPIO_CONFIG_H
