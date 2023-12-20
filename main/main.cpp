
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "esp_log.h"

#include "gpio.h"
#include "adc.h"
#include "soft_access_point.h"

// #include "Arduino.h"

static const char *TAG = "example";

char variable_id_g[64] = "";
char str_value_g[64] = "";
int long_value_g = 0;

extern "C" void variable_id_web_sever_task(void *pvParameters)
{
    char variable_id_last[64] = "";

    while (1)
    {
        vTaskDelay(2000); // Adjust the delay based on your requirements
    }
}

extern "C" void http_server_task_1(void *pvParameters);

extern "C" void app_main()
{

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