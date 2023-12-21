
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "esp_log.h"

#include "gpio.h"
#include "adc.h"
#include "soft_access_point.h"
#include "BleGamepad.h"

// #include "Arduino.h"

#define numOfButtons 14
#define numOfHatSwitches 0
#define enableX false
#define enableY false
#define enableZ false
#define enableRX false
#define enableRY false
#define enableRZ false
#define enableSlider1 false
#define enableSlider2 false
#define enableRudder false
#define enableThrottle true
#define enableAccelerator false
#define enableBrake true
#define enableSteering false

BleGamepad bleGamepad("BLE Driving Controller", "test", 100);

static const char *TAG = "example";

char variable_id_g[64] = "";
char str_value_g[64] = "";
int long_value_g = 0;

char esp32_chip_series[64] = "ESP32_S3";

typedef struct
{
    gpio_num_t *data;
    size_t size;
} DynamicArray;

DynamicArray gpios;

// Function to initialize a dynamic array
void initializeDynamicArray(DynamicArray *array, size_t initialSize)
{
    array->data = (gpio_num_t *)malloc(initialSize * sizeof(gpio_num_t));
    array->size = initialSize;
}

// Function to add a value to the dynamic array
void addToDynamicArray(DynamicArray *array, gpio_num_t value)
{
    array->data[array->size++] = value;
}

// Function to resize the dynamic array
void resizeDynamicArray(DynamicArray *array, size_t newSize)
{
    array->data = (gpio_num_t *)realloc(array->data, newSize * sizeof(gpio_num_t));
    array->size = newSize;
}

extern "C" void variable_id_web_sever_task(void *pvParameters)
{
    char variable_id_last[64] = "";
    const char delimiter[] = ",";

    while (1)
    {
        if (strcmp(variable_id_last, variable_id_g) != 0)
        {
            if (strcmp(variable_id_g, "apply") == 0)
            {
                // Initialize strtok with the input string and delimiter
                char *token = strtok(str_value_g, delimiter);

                DynamicArray tempArray;
                initializeDynamicArray(&tempArray, 48); // Start with an initial size

                // Loop through the tokens
                while (token != NULL)
                {
                    // Convert the token to an integer using atoi
                    uint8_t temp_int = atoi(token);

                    // Add values to tempArray
                    gpio_num_t temp_gpio = (gpio_num_t)temp_int;
                    addToDynamicArray(&tempArray, temp_gpio);

                    // Get the next token
                    token = strtok(NULL, delimiter);
                }

                // Resize tempArray based on the actual number of elements
                resizeDynamicArray(&tempArray, tempArray.size);

                // Now, assign the tempArray to gpios array
                gpio_num_t gpios[tempArray.size];
                memcpy(gpios, tempArray.data, tempArray.size);

                // Free the memory allocated for tempArray
                free(tempArray.data);

                init_gpio(gpios);
            }
            else if (strcmp(variable_id_g, "esp32_chip_series") == 0)
            {
                strcpy(esp32_chip_series, str_value_g);
                // update_chip_series();
                printf("esp32_chip_series: %s\n", esp32_chip_series);
            }
            else
            {
                // Handle the default case or log an error
                ESP_LOGE(TAG, "Unknown variable_id: %s", variable_id_g);
                // break;
            }
            strcpy(variable_id_last, variable_id_g);
        }
        vTaskDelay(2000); // Adjust the delay based on your requirements
    }
}

extern "C" void http_server_task_1(void *pvParameters);

extern "C" void button_task(void *arg)
{
    bool pressed[sizeof(gpios)];

    while (1)
    {
        for (uint8_t i = 0; i < sizeof(gpios); i++)
        {
            // PADDLE_SHIFTER_SWITCH_1
            if (gpio_get_level(gpios[i]) == 0 && pressed[i] == false)
            {
                vTaskDelay(pdMS_TO_TICKS(1));
                if (gpio_get_level(gpios[i]) == 0)
                {
                    printf("GPIO: %d High\n", gpios[i]);
                    bleGamepad.press(1);
                    bleGamepad.sendReport();
                    pressed[i] = true;
                    vTaskDelay(pdMS_TO_TICKS(50)); // Adjust the delay according to your needs
                }
            }
            else if (gpio_get_level(gpios[i]) == 1 && pressed[i] == true)
            {
                vTaskDelay(pdMS_TO_TICKS(1));
                bleGamepad.release(1);
                bleGamepad.sendReport();
                pressed[i] = false;
                vTaskDelay(pdMS_TO_TICKS(50));
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

extern "C" void button_matrix_task(void *arg)
{
    bool pressed[sizeof(gpios)];

    while (1)
    {
        for (uint8_t i = 0; i < sizeof(gpios) - 1; i++)
        {
            // PADDLE_SHIFTER_SWITCH_1
            if (gpio_get_level(gpios[i]) == 0 && gpio_get_level(gpios[i + 1]) == 0 && pressed[i] == false)
            {
                vTaskDelay(pdMS_TO_TICKS(1));
                if (gpio_get_level(gpios[i]) == 0)
                {
                    printf("GPIO: %d and GPIO: %d High\n", gpios[i], gpios[i + 1]);
                    bleGamepad.press(1);
                    bleGamepad.sendReport();
                    pressed[i] = true;
                    vTaskDelay(pdMS_TO_TICKS(50)); // Adjust the delay according to your needs
                }
            }
            else if (gpio_get_level(gpios[i]) == 1 && gpio_get_level(gpios[i + 1]) == 1 && pressed[i] == true)
            {
                vTaskDelay(pdMS_TO_TICKS(1));
                bleGamepad.release(1);
                bleGamepad.sendReport();
                pressed[i] = false;
                vTaskDelay(pdMS_TO_TICKS(50));
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

extern "C" void adc_task(void *pvParameters)
{
    while (1)
    {
        // printf("ADC Raw Value: %d\n", adc_get_raw());
        if (bleGamepad.isConnected())
        {
            // printf("Connected\n");
            bleGamepad.setThrottle(adc_get_raw_throttle());
            bleGamepad.sendReport();

            bleGamepad.setBrake(adc_get_raw_brake());
            bleGamepad.sendReport();
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

/*
extern "C" void gpio_update(gpio_num_t gpios[])
{
    init_gpio();
}
*/
extern "C" void app_main(void)
{
    printf("Starting BLE work!");

    // Setup controller with 10 buttons, accelerator, brake and steering
    BleGamepadConfiguration bleGamepadConfig;
    bleGamepadConfig.setAutoReport(false);
    bleGamepadConfig.setControllerType(CONTROLLER_TYPE_JOYSTICK); // CONTROLLER_TYPE_JOYSTICK, CONTROLLER_TYPE_GAMEPAD (DEFAULT), CONTROLLER_TYPE_MULTI_AXIS
    bleGamepadConfig.setButtonCount(numOfButtons);
    bleGamepadConfig.setWhichAxes(enableX, enableY, enableZ, enableRX, enableRY, enableRZ, enableSlider1, enableSlider2);      // Can also be done per-axis individually. All are true by default
    bleGamepadConfig.setWhichSimulationControls(enableRudder, enableThrottle, enableAccelerator, enableBrake, enableSteering); // Can also be done per-control individually. All are false by default
    bleGamepadConfig.setHatSwitchCount(numOfHatSwitches);                                                                      // 1 by default
    // Some non-Windows operating systems and web based gamepad testers don't like min axis set below 0, so 0 is set by default
    bleGamepadConfig.setAxesMin(0x0000); // -32767 --> int16_t - 16 bit signed integer - Can be in decimal or hexadecimal
    bleGamepadConfig.setAxesMax(0x7FFF); // 32767 --> int16_t - 16 bit signed integer - Can be in decimal or hexadecimal
    bleGamepadConfig.setSimulationMin(0x0000);
    bleGamepadConfig.setSimulationMax(0x0FFF);

    bleGamepad.begin(&bleGamepadConfig);
    // changing bleGamepadConfig after the begin function has no effect, unless you call the begin function again

    // Set accelerator and brake to min
    // bleGamepad.setAccelerator(-32767);
    // bleGamepad.setBrake(-32767);
    // bleGamepad.setAccelerator(0);
    bleGamepad.setThrottle(0);
    bleGamepad.setBrake(0);

    // Set steering to center
    // bleGamepad.setSteering(0);

    adc_init();
    initializeDynamicArray
        init_gpio(gpios->data);

    xTaskCreate(&adc_task, "adc_task", 4096, NULL, 5, NULL);
    xTaskCreate(&button_task, "button_task", 4096, NULL, 5, NULL);

    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "ESP_WIFI_MODE_AP");
    wifi_init_softap();

    // Initialize mDNS
    initialise_mdns();

    // Initialize SPIFFS
    ESP_LOGI(TAG, "Initializing SPIFFS");
    if (SPIFFS_Mount("/html", "storage", 6) != ESP_OK)
    {
        ESP_LOGE(TAG, "SPIFFS mount failed");
        while (1)
        {
            vTaskDelay(1);
        }
    }

    // Create Queue
    xQueueHttp = xQueueCreate(10, sizeof(URL_t));
    configASSERT(xQueueHttp);

    // Get the local IP address
    esp_netif_ip_info_t ip_info;
    ESP_ERROR_CHECK(esp_netif_get_ip_info(esp_netif_get_handle_from_ifkey("WIFI_AP_DEF"), &ip_info));

    char cparam0[64];
    sprintf(cparam0, IPSTR, IP2STR(&ip_info.ip));
    printf("Test 1 Main");
    xTaskCreate(http_server_task_1, "HTTP", 1024 * 6, (void *)cparam0, 2, NULL);
    // xTaskCreate(printVariablesTask, "PrintVariablesTask", 4096, NULL, 1, NULL);
    xTaskCreate(variable_id_web_sever_task, "variable_id_web_sever_task", 4096, NULL, 1, NULL);

    // Wait for the task to start, because cparam0 is discarded.
    vTaskDelay(10);
}