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
     * @brief Initialize GPIO pins used for encoder and servo drive signals.
     *
     * This function configures the GPIO pins used dynamically
     */
    void init_gpio(gpio_num_t gpios[]);

#ifdef __cplusplus
}
#endif

#endif // GPIO_CONFIG_H
