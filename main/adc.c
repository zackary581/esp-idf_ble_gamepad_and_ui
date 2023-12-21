#include "adc.h"

/**
 * @brief Initializes the ADC.
 * @details This function characterizes ADC1 and configures the ADC channel and width.
 */
void adc_init()
{
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTENUATION, ADC_WIDTH_BIT_DEFAULT, 0, &adc1_chars);
    // Characterize ADC1 using specified parameters and store the characteristics in adc1_chars.

    adc1_config_width(ADC_WIDTH_BIT_DEFAULT);
    // Configure ADC1 to use the default bit width.

    adc1_config_channel_atten(ADC1_CHANNEL, ADC_ATTENUATION);
    // Configure ADC1 channel to use the specified attenuation level.
}

/**
 * @brief Reads the raw ADC value.
 * @return The raw ADC value.
 */
int adc_get_raw_throttle()
{
    return adc1_get_raw(ADC1_CHANNEL); // adc channel 3 gpio 4
}

/**
 * @brief Reads the raw ADC value.
 * @return The raw ADC value.
 */
int adc_get_raw_brake()
{
    return adc1_get_raw(ADC1_CHANNEL_5); // Gpio 6
}
