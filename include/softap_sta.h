#ifndef WIFI_CONFIG_H
#define WIFI_CONFIG_H

#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_netif_net_stack.h"
#include "esp_http_server.h"
#include "http_server.h"

/* STA Configuration */
#define EXAMPLE_ESP_WIFI_STA_SSID CONFIG_ESP_WIFI_REMOTE_AP_SSID
#define EXAMPLE_ESP_WIFI_STA_PASSWD CONFIG_ESP_WIFI_REMOTE_AP_PASSWORD
#define EXAMPLE_ESP_MAXIMUM_RETRY CONFIG_ESP_MAXIMUM_STA_RETRY

#if CONFIG_ESP_WIFI_AUTH_OPEN
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_OPEN
#elif CONFIG_ESP_WIFI_AUTH_WEP
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WEP
#elif CONFIG_ESP_WIFI_AUTH_WPA_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA2_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA_WPA2_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA_WPA2_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA3_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA3_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA2_WPA3_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_WPA3_PSK
#elif CONFIG_ESP_WIFI_AUTH_WAPI_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WAPI_PSK
#endif

/* AP Configuration */
#define EXAMPLE_ESP_WIFI_AP_SSID CONFIG_ESP_WIFI_AP_SSID
#define EXAMPLE_ESP_WIFI_AP_PASSWD CONFIG_ESP_WIFI_AP_PASSWORD
#define EXAMPLE_ESP_WIFI_CHANNEL CONFIG_ESP_WIFI_AP_CHANNEL
#define EXAMPLE_MAX_STA_CONN CONFIG_ESP_MAX_STA_CONN_AP

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief File path for the HTML index page.
 */
#define HTML_INDEX_PATH "index.html"

/**
 * @brief File path for the HTML file when the server is off.
 */
#define OFF_HTML_FILE_PATH "index.html"

    /**
     * @brief Queue handle for HTTP requests.
     */
    extern QueueHandle_t xQueueHttp;

    static const char *TAG_AP = "WiFi SoftAP";
    static const char *TAG_STA = "WiFi Sta";

    static int s_retry_num = 0;

    /* FreeRTOS event group to signal when we are connected/disconnected */
    static EventGroupHandle_t s_wifi_event_group;

    void wifi_event_handler(void *arg, esp_event_base_t event_base,
                            int32_t event_id, void *event_data);

    esp_netif_t *wifi_init_softap(void);
    esp_netif_t *wifi_init_sta(void);
    /**
     * @brief HTTP server request handler for sending the web page.
     */
    esp_err_t send_web_page(httpd_req_t *req);

    /**
     * @brief HTTP server request handler for GET requests.
     */
    esp_err_t get_req_handler(httpd_req_t *req);

    /**
     * @brief Function to set the SPIFFS file system and mount it.
     */
    esp_err_t SPIFFS_Mount(char *path, char *label, int max_files);

    /**
     * @brief Function to initialize mDNS.
     */
    void initialise_mdns(void);

#ifdef __cplusplus
}
#endif

#endif // WIFI_CONFIG_H
