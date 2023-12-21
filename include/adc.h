#ifndef ADC_H
#define ADC_H

#include "driver/adc.h"
#include "esp_adc_cal.h"

/**
 * @file adc.h
 * @brief Header file for ADC (Analog-to-Digital Converter) functions.
 */

#define ADC1_CHANNEL ADC1_CHANNEL_3     /**< ADC channel to be used. */
#define ADC_ATTENUATION ADC_ATTEN_DB_11 /**< ADC attenuation setting. */

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief Structure to hold characteristics of ADC calibration for ADC1.
     */
    static esp_adc_cal_characteristics_t adc1_chars;

    /**
     * @brief Initializes the ADC.
     */
    void adc_init();

    /**
     * @brief Reads the raw ADC value.
     * @return The raw ADC value.
     */
    int adc_get_raw();

    /**
     * @brief Reads the raw ADC value.
     * @return The raw ADC value.
     */
    int adc_get_raw_throttle();

    /**
     * @brief Reads the raw ADC value.
     * @return The raw ADC value.
     */
    int adc_get_raw_brake();

#ifdef __cplusplus
}
#endif

#endif // ADC_H
