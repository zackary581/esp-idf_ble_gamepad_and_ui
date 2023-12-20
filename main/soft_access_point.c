/*Reference: ESP-IDF general wifi examples and https://github.com/nopnop2002/esp-idf-pwm-slider/tree/main*/

/**
 * @file soft_access_point.c
 * @brief Implementation file for WiFi SoftAP functionality.
 */
#include "soft_access_point.h"

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
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_vfs.h"
#include "esp_spiffs.h"
#include "nvs_flash.h"
#include "esp_err.h"
#include "esp_log.h"
#include "mdns.h"
#include "lwip/dns.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "http_server.h"

// Your WiFi configuration macros that can be set via the project configuration menu
#define EXAMPLE_ESP_WIFI_SSID CONFIG_ESP_WIFI_SSID
#define EXAMPLE_ESP_WIFI_PASS CONFIG_ESP_WIFI_PASSWORD
#define EXAMPLE_ESP_WIFI_CHANNEL CONFIG_ESP_WIFI_CHANNEL
#define EXAMPLE_MAX_STA_CONN CONFIG_ESP_MAX_STA_CONN

// Static tag for logging
static const char *TAG = "MAIN";

// FreeRTOS event group to signal when we are connected
static EventGroupHandle_t s_wifi_event_group;

// The event group allows multiple bits for each event, but we only care about two events:
// - we are connected to the AP with an IP
// - we failed to connect after the maximum amount of retries
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1

// Variable to track the number of retry attempts
static int s_retry_num = 0;

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
void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    // Check if the WiFi event is a station connecting to the SoftAP
    if (event_id == WIFI_EVENT_AP_STACONNECTED)
    {
        // Cast the event data to the structure for connected stations
        wifi_event_ap_staconnected_t *event = (wifi_event_ap_staconnected_t *)event_data;

        // Log information about the connected station
        ESP_LOGI(TAG, "Station " MACSTR " joined, AID=%d", MAC2STR(event->mac), event->aid);
    }
    // Check if the WiFi event is a station disconnecting from the SoftAP
    else if (event_id == WIFI_EVENT_AP_STADISCONNECTED)
    {
        // Cast the event data to the structure for disconnected stations
        wifi_event_ap_stadisconnected_t *event = (wifi_event_ap_stadisconnected_t *)event_data;

        // Log information about the disconnected station
        ESP_LOGI(TAG, "Station " MACSTR " left, AID=%d", MAC2STR(event->mac), event->aid);
    }
    // Add more conditions to handle other WiFi events if needed
}

/**
 * @brief WiFi initialization function for the station mode.
 *
 * This function initializes the WiFi stack in station (STA) mode. It creates an event group,
 * sets up network interfaces, handles static IP configuration (if enabled), configures WiFi
 * parameters, registers event handlers, and waits for a successful connection to the access point (AP).
 *
 * The function blocks until the station successfully connects to the AP and obtains an IP address.
 * It logs relevant information using ESP_LOGI during the initialization process.
 */
void wifi_init_sta()
{
    // Create a FreeRTOS event group to signal WiFi events
    s_wifi_event_group = xEventGroupCreate();

    // Log ESP-IDF version information
    ESP_LOGI(TAG, "ESP-IDF Ver%d.%d", ESP_IDF_VERSION_MAJOR, ESP_IDF_VERSION_MINOR);
    ESP_LOGI(TAG, "ESP_IDF_VERSION %d", ESP_IDF_VERSION);

    // Initialize ESP-IDF networking interface
    ESP_LOGI(TAG, "ESP-IDF esp_netif");
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Create a default WiFi station interface
    esp_netif_t *netif = esp_netif_create_default_wifi_sta();
    assert(netif);

    // Handle static IP configuration if enabled
#if CONFIG_STATIC_IP
    ESP_LOGI(TAG, "CONFIG_STATIC_IP_ADDRESS=[%s]", CONFIG_STATIC_IP_ADDRESS);
    ESP_LOGI(TAG, "CONFIG_STATIC_GW_ADDRESS=[%s]", CONFIG_STATIC_GW_ADDRESS);
    ESP_LOGI(TAG, "CONFIG_STATIC_NM_ADDRESS=[%s]", CONFIG_STATIC_NM_ADDRESS);

    // Stop DHCP client and configure static IP address
    ESP_ERROR_CHECK(esp_netif_dhcpc_stop(netif));
    ESP_LOGI(TAG, "Stop DHCP Services");

    esp_netif_ip_info_t ip_info;
    memset(&ip_info, 0, sizeof(esp_netif_ip_info_t));
    ip_info.ip.addr = ipaddr_addr(CONFIG_STATIC_IP_ADDRESS);
    ip_info.netmask.addr = ipaddr_addr(CONFIG_STATIC_NM_ADDRESS);
    ip_info.gw.addr = ipaddr_addr(CONFIG_STATIC_GW_ADDRESS);
    esp_netif_set_ip_info(netif, &ip_info);

    // Configure DNS servers manually (Google's public DNS servers)
    ip_addr_t d;
    d.type = IPADDR_TYPE_V4;
    d.u_addr.ip4.addr = 0x08080808; // 8.8.8.8 dns
    dns_setserver(0, &d);
    d.u_addr.ip4.addr = 0x08080404; // 8.8.4.4 dns
    dns_setserver(1, &d);
#endif // CONFIG_STATIC_IP

    // Initialize WiFi stack with default configuration
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // Register WiFi and IP event handlers
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL));

    // Configure WiFi parameters for station mode
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = CONFIG_ESP_WIFI_SSID,
            .password = CONFIG_ESP_WIFI_PASSWORD},
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    // Log completion of WiFi station initialization
    ESP_LOGI(TAG, "wifi_init_sta finished.");
    ESP_LOGI(TAG, "Connecting to AP SSID:%s password:%s",
             CONFIG_ESP_WIFI_SSID, CONFIG_ESP_WIFI_PASSWORD);

    // Wait for IP_EVENT_STA_GOT_IP event to indicate a successful connection
    while (1)
    {
        // Wait indefinitely for WIFI_CONNECTED_BIT to be set within the event group
        EventBits_t uxBits = xEventGroupWaitBits(s_wifi_event_group,
                                                 WIFI_CONNECTED_BIT, /* Bits to wait for within the event group */
                                                 pdTRUE,             /* WIFI_CONNECTED_BIT should be cleared before returning */
                                                 pdFALSE,            /* Don't wait for both bits, either bit will do */
                                                 portMAX_DELAY);     /* Wait indefinitely */

        // Check if WIFI_CONNECTED_BIT is set
        if ((uxBits & WIFI_CONNECTED_BIT) == WIFI_CONNECTED_BIT)
        {
            ESP_LOGI(TAG, "WIFI_CONNECTED_BIT");
            break;
        }
    }

    // Log successful connection and obtained IP address
    ESP_LOGI(TAG, "Got IP Address.");
}

/**
 * @brief WiFi initialization function for the SoftAP mode.
 *
 * This function initializes the WiFi stack in SoftAP (Access Point) mode. It sets up the network interface,
 * initializes the WiFi stack with default configuration, registers WiFi event handlers, and starts the SoftAP.
 *
 * The SoftAP configuration is provided through the `wifi_config_t` structure, where parameters such as SSID,
 * password, authentication mode, and channel are specified. If no password is provided, the access point is set
 * to an open network. The function logs relevant information using ESP_LOGI during the initialization process.
 */
void wifi_init_softap(void)
{
    // Initialize ESP-IDF networking interface
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Create a default WiFi SoftAP interface
    esp_netif_create_default_wifi_ap();

    // Initialize WiFi stack with default configuration
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // Register WiFi event handlers
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        NULL));

    // Configure SoftAP parameters using wifi_config_t structure
    wifi_config_t wifi_config = {
        .ap = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .ssid_len = strlen(EXAMPLE_ESP_WIFI_SSID),
            .channel = EXAMPLE_ESP_WIFI_CHANNEL,
            .password = EXAMPLE_ESP_WIFI_PASS,
            .max_connection = EXAMPLE_MAX_STA_CONN,
#ifdef CONFIG_ESP_WIFI_SOFTAP_SAE_SUPPORT
            .authmode = WIFI_AUTH_WPA3_PSK,
            .sae_pwe_h2e = WPA3_SAE_PWE_BOTH,
#else /* CONFIG_ESP_WIFI_SOFTAP_SAE_SUPPORT */
            .authmode = WIFI_AUTH_WPA2_PSK,
#endif
            .pmf_cfg = {
                .required = true,
            },
        },
    };

    // Set SoftAP to an open network if no password is provided
    if (strlen(EXAMPLE_ESP_WIFI_PASS) == 0)
    {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    // Set WiFi mode to SoftAP and configure SoftAP parameters
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));

    // Start the WiFi SoftAP
    ESP_ERROR_CHECK(esp_wifi_start());

    // Log completion of WiFi SoftAP initialization
    ESP_LOGI(TAG, "wifi_init_softap finished. SSID:%s password:%s channel:%d",
             EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS, EXAMPLE_ESP_WIFI_CHANNEL);
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