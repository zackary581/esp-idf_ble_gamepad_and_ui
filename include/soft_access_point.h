/**
 * @file soft_access_point.h
 * @brief Header file for WiFi SoftAP functionality.
 */

#ifndef WIFI_SOFTAP_H
#define WIFI_SOFTAP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <esp_http_server.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/semphr.h"
#include "esp_mac.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_vfs.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_netif.h"
#include "driver/gpio.h"
#include "mdns.h"
#include "esp_spiffs.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include <lwip/sockets.h>
#include <lwip/sys.h>
#include <lwip/api.h>
#include <lwip/netdb.h>
#include "lwip/dns.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "http_server.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief WiFi configuration macros that can be set via the project configuration menu.
 */
#define EXAMPLE_ESP_WIFI_SSID CONFIG_ESP_WIFI_SSID
#define EXAMPLE_ESP_WIFI_PASS CONFIG_ESP_WIFI_PASSWORD
#define EXAMPLE_ESP_WIFI_CHANNEL CONFIG_ESP_WIFI_CHANNEL
#define EXAMPLE_MAX_STA_CONN CONFIG_ESP_MAX_STA_CONN

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

   /**
    * @brief Function to initialize WiFi SoftAP.
    */
   void wifi_init_softap(void);

   /**
    * @brief WiFi event handler function.
    */
   static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);

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

   /**
    * @brief Function to set a static IP address.
    */
   void set_static_ip();

#ifdef __cplusplus
}
#endif

#endif /* WIFI_SOFTAP_H */
