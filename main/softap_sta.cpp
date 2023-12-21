

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include "freertos/semphr.h"

#include "esp_system.h"
#include "esp_mac.h"
#include "esp_event.h"
#include "esp_vfs.h"
#include "esp_spiffs.h"
#include "esp_err.h"
#include "esp_log.h"

#include "mdns.h"

#include "lwip/dns.h"
#include "lwip/inet.h"
#include "lwip/netdb.h"
#include "lwip/sockets.h"
#if IP_NAPT
#include "lwip/lwip_napt.h"
#endif
#include "lwip/err.h"
#include "lwip/sys.h"

#include "softap_sta.h"
#include "http_server.h"

/* The examples use WiFi configuration that you can set via project configuration menu.

   If you'd rather not, just change the below entries to strings with
   the config you want - ie #define EXAMPLE_ESP_WIFI_STA_SSID "mywifissid"
*/

/* FreeRTOS event group to signal when we are connected/disconnected */
// static EventGroupHandle_t s_wifi_event_group;

// Static tag for logging
static const char *TAG = "MAIN";

// Queue handle for HTTP requests
QueueHandle_t xQueueHttp;

/**
 * @brief WiFi event handler function.
 *
 * This function handles WiFi events, such as a station connecting or disconnecting from the SoftAP.
 * It logs information about the events using ESP_LOGI.
 *
 * @param[in] arg: An argument passed to the handler (not used in this function).
 * @param[in] event_base: The event base associated with the event.
 * @param[in] event_id: The ID of the WiFi event.
 * @param[in] event_data: The data associated with the WiFi event.
 */
extern "C" void wifi_event_handler(void *arg, esp_event_base_t event_base,
                                   int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STACONNECTED)
    {
        wifi_event_ap_staconnected_t *event = (wifi_event_ap_staconnected_t *)event_data;
        ESP_LOGI(TAG_AP, "Station " MACSTR " joined, AID=%d",
                 MAC2STR(event->mac), event->aid);
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STADISCONNECTED)
    {
        wifi_event_ap_stadisconnected_t *event = (wifi_event_ap_stadisconnected_t *)event_data;
        ESP_LOGI(TAG_AP, "Station " MACSTR " left, AID=%d",
                 MAC2STR(event->mac), event->aid);
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
        ESP_LOGI(TAG_STA, "Station started");
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG_STA, "Got IP:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

/* Initialize soft AP */
extern "C" esp_netif_t *wifi_init_softap(void)
{
    esp_netif_t *esp_netif_ap = esp_netif_create_default_wifi_ap();

    wifi_config_t wifi_ap_config = {
        .ap = {
            .ssid = EXAMPLE_ESP_WIFI_AP_SSID,
            .password = EXAMPLE_ESP_WIFI_AP_PASSWD,
            .ssid_len = strlen(EXAMPLE_ESP_WIFI_AP_SSID),
            .channel = EXAMPLE_ESP_WIFI_CHANNEL,
            .authmode = WIFI_AUTH_WPA2_PSK,
            .max_connection = EXAMPLE_MAX_STA_CONN,
            .pmf_cfg = {
                .required = false,
            },
        },
    };

    if (strlen(EXAMPLE_ESP_WIFI_AP_PASSWD) == 0)
    {
        wifi_ap_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_ap_config));

    ESP_LOGI(TAG_AP, "wifi_init_softap finished. SSID:%s password:%s channel:%d",
             EXAMPLE_ESP_WIFI_AP_SSID, EXAMPLE_ESP_WIFI_AP_PASSWD, EXAMPLE_ESP_WIFI_CHANNEL);

    return esp_netif_ap;
}

/* Initialize wifi station */
extern "C" esp_netif_t *wifi_init_sta(void)
{
    esp_netif_t *esp_netif_sta = esp_netif_create_default_wifi_sta();

    wifi_config_t wifi_sta_config = {
        .sta = {
            .ssid = EXAMPLE_ESP_WIFI_STA_SSID,
            .password = EXAMPLE_ESP_WIFI_STA_PASSWD,
            .scan_method = WIFI_ALL_CHANNEL_SCAN,
            /* Authmode threshold resets to WPA2 as default if password matches WPA2 standards (pasword len => 8).
             * If you want to connect the device to deprecated WEP/WPA networks, Please set the threshold value
             * to WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK and set the password with length and format matching to
             * WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK standards.
             */
            .threshold = {
                .authmode = ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD,
            },
            .sae_pwe_h2e = WPA3_SAE_PWE_BOTH,
            .failure_retry_cnt = EXAMPLE_ESP_MAXIMUM_RETRY,
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_sta_config));

    ESP_LOGI(TAG_STA, "wifi_init_sta finished.");

    return esp_netif_sta;
}

/**
 * @brief Function to initialize mDNS.
 *
 * This function initializes the mDNS (Multicast DNS) service. It sets up the mDNS system and configures
 * the hostname for the ESP32. mDNS allows devices on a network to discover and connect to each other
 * using human-readable hostnames without the need for a central DNS server.
 *
 * The function uses the ESP_ERROR_CHECK macro to handle errors during mDNS initialization. It then sets the
 * mDNS hostname to the value defined in the CONFIG_MDNS_HOSTNAME configuration. This hostname is essential
 * for advertising services over the network.
 *
 * Additionally, there is commented-out code to set a default mDNS instance name ("ESP32 with mDNS"). You can
 * uncomment and modify this code if a specific instance name is desired.
 *
 * The function logs relevant information using ESP_LOGI during the initialization process.
 */
void initialise_mdns(void)
{
    // Initialize mDNS service
    ESP_ERROR_CHECK(mdns_init());

    // Set mDNS hostname (required for advertising services)
    ESP_ERROR_CHECK(mdns_hostname_set(CONFIG_MDNS_HOSTNAME));
    ESP_LOGI(TAG, "mdns hostname set to: [%s]", CONFIG_MDNS_HOSTNAME);

    // Commented-out: Set default mDNS instance name (uncomment and modify as needed)
    /*
    ESP_ERROR_CHECK(mdns_instance_name_set("ESP32 with mDNS"));
    */

    // Log completion of mDNS initialization
    ESP_LOGI(TAG, "mDNS initialization complete.");
}

/**
 * @brief Function to display the contents of a directory in SPIFFS.
 *
 * This function takes a path as a parameter and opens the corresponding directory in the SPIFFS (SPI Flash File System).
 * It then iterates through the directory entries, printing information about each entry using ESP_LOGI. The printed
 * information includes the name of the file or directory, the inode number, and the type of the entry.
 *
 * @param path The path of the directory in SPIFFS whose contents are to be displayed.
 *
 * The function uses the opendir function to open the specified directory. If the directory cannot be opened, it asserts
 * to indicate a critical error. The function then enters a loop that iterates through each entry in the directory using
 * the readdir function. For each entry, it prints information using ESP_LOGI.
 *
 * The loop continues until there are no more entries in the directory. Finally, the closedir function is called to
 * close the directory stream.
 *
 * Note: This function assumes that the SPIFFS has been mounted and configured properly before calling it.
 *
 * Example Usage:
 *     SPIFFS_Directory("/my_directory");
 *
 * This example would display information about each file and subdirectory in the "/my_directory" directory in SPIFFS.
 */
void SPIFFS_Directory(char *path)
{
    // Open the directory in SPIFFS
    DIR *dir = opendir(path);
    assert(dir != NULL); // Assert if the directory cannot be opened (critical error)

    // Iterate through each entry in the directory
    while (true)
    {
        // Read the next directory entry
        struct dirent *pe = readdir(dir);

        // Break the loop if there are no more entries
        if (!pe)
            break;

        // Print information about the current entry using ESP_LOGI
        ESP_LOGI(TAG, "d_name=%s/%s d_ino=%d d_type=%x", path, pe->d_name, pe->d_ino, pe->d_type);
    }

    // Close the directory stream
    closedir(dir);
}

/**
 * @brief Function to set the SPIFFS file system and mount it.
 *
 * This function initializes and mounts the SPIFFS (SPI Flash File System) using the specified configuration parameters.
 *
 * @param path The base file path prefix associated with the filesystem.
 * @param label Optional label of the SPIFFS partition to use. If set to NULL, the first partition with subtype=spiffs will be used.
 * @param max_files Maximum number of files that can be open at the same time.
 * @return esp_err_t Returns ESP_OK on success, or an error code if the operation fails.
 *
 * The function takes the base file path, partition label, and maximum number of files as parameters to configure the SPIFFS.
 * It uses the esp_vfs_spiffs_conf_t structure to store the configuration and calls esp_vfs_spiffs_register to initialize and mount the SPIFFS.
 *
 * If the operation fails, it logs an error message with the specific failure details using ESP_LOGE and returns the corresponding error code.
 * If successful, it retrieves and logs information about the SPIFFS partition, including total and used space.
 * Finally, the function calls SPIFFS_Directory to display the contents of the mounted directory.
 *
 * Example Usage:
 *     ESP_ERROR_CHECK(SPIFFS_Mount("/spiffs", "data", 10));
 *
 * This example initializes and mounts SPIFFS with a base path of "/spiffs," using the "data" partition label, and supporting a maximum
 * of 10 open files at the same time.
 */
esp_err_t SPIFFS_Mount(char *path, char *label, int max_files)
{
    // Configuration for esp_vfs_spiffs_register
    esp_vfs_spiffs_conf_t conf = {
        .base_path = path,
        .partition_label = label,
        .max_files = (size_t)max_files,
        .format_if_mount_failed = true};

    // Use settings defined above to initialize and mount SPIFFS filesystem.
    esp_err_t ret = esp_vfs_spiffs_register(&conf);

    if (ret != ESP_OK)
    {
        // Handle mount or format failure
        if (ret == ESP_FAIL)
        {
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
        }
        else if (ret == ESP_ERR_NOT_FOUND)
        {
            ESP_LOGE(TAG, "Failed to find SPIFFS partition");
        }
        else
        {
            ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return ret;
    }

    // Get and log SPIFFS partition information
    size_t total = 0, used = 0;
    ret = esp_spiffs_info(conf.partition_label, &total, &used);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
    }
    else
    {
        ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
    }

    // Log successful mount and display the contents of the directory
    if (ret == ESP_OK)
    {
        ESP_LOGI(TAG, "Mount %s to %s success", path, label);
        SPIFFS_Directory(path);
    }

    return ret;
}

/**
 * @brief Task to handle HTTP server operations.
 *
 * This function represents a FreeRTOS task that handles HTTP server operations. The actual implementation of the HTTP server
 * operations is expected to be defined in the http_server_task function.
 *
 * @param pvParameters Task parameters (unused in this example).
 *
 * The function serves as a placeholder for the implementation of HTTP server-related functionality within a FreeRTOS task.
 * It is expected that the http_server_task function will include the logic for HTTP server operations.
 *
 * Example Usage:
 *     xTaskCreate(&http_server_task, "http_server", 4096, NULL, 5, NULL);
 *
 * This example creates a FreeRTOS task named "http_server" with a stack size of 4096 bytes, no task parameters, a priority of 5,
 * and no task handle.
 */
void http_server_task(void *pvParameters);