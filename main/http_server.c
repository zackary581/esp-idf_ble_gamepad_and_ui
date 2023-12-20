#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <math.h>
#include <sys/stat.h>
#include <mbedtls/base64.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "esp_err.h"
#include "nvs.h"
#include "esp_http_server.h"
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
#include "driver/mcpwm_prelude.h"
#else
#include "driver/mcpwm.h"
#endif

#include "http_server.h"

static const char *TAG = "HTTP";

extern QueueHandle_t xQueueHttp;
extern char variable_id_g[64];
extern char str_value_g[64];
extern int long_value_g;

////Config config;

/**
 * @brief Finds the value associated with a key in a parameter string.
 * @param key The key to search for.
 * @param parameter The parameter string.
 * @param value The output value.
 * @return The length of the found value.
 */
int find_key_value(char *key, char *parameter, char *value)
{
    // char * addr1;
    // Find the first occurrence of the key in the parameter string.
    char *addr1 = strstr(parameter, key);
    if (addr1 == NULL)
        return 0; // If the key is not found, return 0 indicating no value found.

    ESP_LOGD(TAG, "addr1=%s", addr1); // Debug log: Print the address where the key is found.

    // Advance addr2 to the position right after the found key.
    char *addr2 = addr1 + strlen(key);
    ESP_LOGD(TAG, "addr2=[%s]", addr2); // Debug log: Print the address pointing to the value.

    // Find the next '&' character after the key's value.
    char *addr3 = strstr(addr2, "&");
    ESP_LOGD(TAG, "addr3=%p", addr3); // Debug log: Print the address where '&' is found.

    if (addr3 == NULL)
    {
        // If no '&' is found, copy the value from addr2 to the end.
        strcpy(value, addr2);
    }
    else
    {
        // Calculate the length of the value between addr2 and addr3.
        int length = addr3 - addr2;
        ESP_LOGD(TAG, "addr2=%p addr3=%p length=%d", addr2, addr3, length); // Debug log: Print the addresses and length.

        // Copy the value from addr2 to value, up to the calculated length.
        strncpy(value, addr2, length);
        value[length] = 0; // Null-terminate the value string.
    }

    ESP_LOGI(TAG, "key=[%s] value=[%s]", key, value); // Info log: Print the key and the found value.
    return strlen(value);                             // Return the length of the found value.
}

/**
 * @brief Sends text data as an HTML response.
 * @param req HTTP request structure.
 * @param filename Name of the HTML file.
 * @return ESP_OK if successful, otherwise ESP_FAIL.
 */
esp_err_t Text2Html(httpd_req_t *req, char *filename)
{
    ESP_LOGI(TAG, "Reading %s", filename); // Log: Indicate that the HTML file is being read.
    FILE *fhtml = fopen(filename, "r");    // Open the HTML file in read-only mode.
    if (fhtml == NULL)
    {
        ESP_LOGE(TAG, "fopen fail. [%s]", filename); // Log: Indicate failure if file opening fails.
        return ESP_FAIL;                             // Return failure status.
    }
    else
    {
        char line[128]; // Buffer to store each line of the HTML file.

        // Read each line from the HTML file.
        while (fgets(line, sizeof(line), fhtml) != NULL)
        {
            size_t linelen = strlen(line); // Get the length of the current line.

            // Remove end-of-line characters (CR or LF) from the line.
            for (int i = linelen; i > 0; i--)
            {
                if (line[i - 1] == 0x0a)
                {
                    line[i - 1] = 0;
                }
                else if (line[i - 1] == 0x0d)
                {
                    line[i - 1] = 0;
                }
                else
                {
                    break;
                }
            }

            ESP_LOGD(TAG, "line=[%s]", line); // Debug log: Print the current line.

            if (strlen(line) == 0)
                continue; // Skip empty lines.

            // Send the current line as an HTML response chunk.
            esp_err_t ret = httpd_resp_sendstr_chunk(req, line);
            if (ret != ESP_OK)
            {
                ESP_LOGE(TAG, "httpd_resp_sendstr_chunk fail %d", ret); // Log: Indicate failure if sending chunk fails.
            }
        }

        fclose(fhtml); // Close the HTML file.
    }

    return ESP_OK; // Return success status.
}

/**
 * @brief Sends image data as an HTML response.
 * @param req HTTP request structure.
 * @param filename Name of the image file.
 * @param type Type of the image (e.g., "jpeg", "jpg", "png").
 * @return ESP_OK if successful, otherwise ESP_FAIL.
 */
esp_err_t Image2Html(httpd_req_t *req, char *filename, char *type)
{
    FILE *fhtml = fopen(filename, "r"); // Open the image file in read-only mode.
    if (fhtml == NULL)
    {
        ESP_LOGE(TAG, "fopen fail. [%s]", filename); // Log: Indicate failure if file opening fails.
        return ESP_FAIL;                             // Return failure status.
    }
    else
    {
        char buffer[64]; // Buffer to store chunks of image data.

        // Determine the image type and send the corresponding HTML header.
        if (strcmp(type, "jpeg") == 0)
        {
            httpd_resp_sendstr_chunk(req, "<img src=\"data:image/jpeg;base64,");
        }
        else if (strcmp(type, "jpg") == 0)
        {
            httpd_resp_sendstr_chunk(req, "<img src=\"data:image/jpeg;base64,");
        }
        else if (strcmp(type, "png") == 0)
        {
            httpd_resp_sendstr_chunk(req, "<img src=\"data:image/png;base64,");
        }
        else
        {
            ESP_LOGW(TAG, "file type fail. [%s]", type); // Log: Warn about unsupported image type.
            httpd_resp_sendstr_chunk(req, "<img src=\"data:image/png;base64,");
        }

        while (1)
        {
            size_t bufferSize = fread(buffer, 1, sizeof(buffer), fhtml); // Read a chunk of image data.
            ESP_LOGD(TAG, "bufferSize=%d", bufferSize);                  // Debug log: Print the size of the read buffer.

            if (bufferSize > 0)
            {
                // Send the chunk of image data to the HTTP response.
                httpd_resp_send_chunk(req, buffer, bufferSize);
            }
            else
            {
                break; // Break out of the loop if no more data is available in the file.
            }
        }

        fclose(fhtml);                        // Close the image file.
        httpd_resp_sendstr_chunk(req, "\">"); // Send the closing HTML tag for the image.
    }
    return ESP_OK; // Return success status.
}

/**
 * @brief HTTP GET handler for the root URI.
 * @param req HTTP request structure.
 * @return ESP_OK if successful, otherwise ESP_FAIL.
 */
static esp_err_t root_get_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "root_get_handler req->uri=[%s]", req->uri);

    /* Send index.html */
    Text2Html(req, "/html/index.html");

    /* Send Image */
    Image2Html(req, "/html/ESP-LOGO.txt", "png");

    /* Send empty chunk to signal HTTP response completion */
    httpd_resp_sendstr_chunk(req, NULL);

    return ESP_OK;
}

/**
 * @brief HTTP POST handler for the root URI.
 * @param req HTTP request structure.
 * @return ESP_OK if successful, otherwise ESP_FAIL.
 */
static esp_err_t root_post_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "root_post_handler req->uri=[%s]", req->uri);           // Log: Print the URI of the incoming POST request.
    URL_t urlBuf;                                                         // Define a structure to store URL information.
    find_key_value("value=", (char *)req->uri, urlBuf.str_value);         // Extract the 'value' parameter from the URI.
    find_key_value("variable_id=", (char *)req->uri, urlBuf.variable_id); // Extract the 'variable_id' parameter from the URI.

    ESP_LOGD(TAG, "urlBuf.str_value=[%s]", urlBuf.str_value);     // Debug log: Print the extracted 'value' parameter.
    urlBuf.long_value = strtol(urlBuf.str_value, NULL, 10);       // Convert the 'value' parameter to a long integer.
    ESP_LOGD(TAG, "urlBuf.long_value=%ld", urlBuf.long_value);    // Debug log: Print the converted long value.
    ESP_LOGD(TAG, "urlBuf.variable_id=[%s]", urlBuf.variable_id); // Debug log: Print the extracted 'variable_id' parameter.

    // Send the URL structure to the http_server_task through a queue.
    if (xQueueSend(xQueueHttp, &urlBuf, portMAX_DELAY) != pdPASS)
    {
        ESP_LOGE(TAG, "xQueueSend Fail"); // Log: Indicate failure if sending to the queue fails.
    }

    /* Redirect onto root to see the updated file list */
    httpd_resp_set_status(req, "303 See Other"); // Set HTTP response status to redirect.
    httpd_resp_set_hdr(req, "Location", "/");    // Set the 'Location' header for redirection to the root URI.
#ifdef CONFIG_EXAMPLE_HTTPD_CONN_CLOSE_HEADER
    httpd_resp_set_hdr(req, "Connection", "close"); // Set the 'Connection' header for connection closure.
#endif
    httpd_resp_sendstr(req, "post successfully"); // Send a success message as the response.
    return ESP_OK;                                // Return success status.
}

/**
 * @brief HTTP GET handler for the favicon.
 * @param req HTTP request structure.
 * @return ESP_OK if successful, otherwise ESP_FAIL.
 */
static esp_err_t favicon_get_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "favicon_get_handler req->uri=[%s]", req->uri); // Log: Print the URI of the incoming GET request.

    return ESP_OK; // Return success status.
}

/**
 * @brief Function to start the web server.
 * @param base_path The base path for serving files.
 * @param port The port number for the server.
 * @return ESP_OK if successful, otherwise ESP_FAIL.
 */
esp_err_t start_server(const char *base_path, int port)
{
    httpd_handle_t server = NULL;                   // Declare a handle for the HTTP server.
    httpd_config_t config = HTTPD_DEFAULT_CONFIG(); // Initialize a default configuration for the HTTP server.
    config.server_port = port;                      // Set the server port in the configuration.

    /* Use the URI wildcard matching function to
     * allow the same handler to respond to multiple different
     * target URIs that match the wildcard scheme */
    config.uri_match_fn = httpd_uri_match_wildcard;

    ESP_LOGI(TAG, "Starting HTTP Server on port: '%d'", config.server_port); // Log: Print the server port being used.

    // Attempt to start the HTTP server with the specified configuration.
    if (httpd_start(&server, &config) != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to start file server!"); // Log: Indicate failure to start the server.
        return ESP_FAIL;                               // Return failure status.
    }

    /* URI handler for GET requests */
    httpd_uri_t _root_get_handler = {
        .uri = "/",                  // URI path for the root handler.
        .method = HTTP_GET,          // Handler for GET requests.
        .handler = root_get_handler, // Function pointer to the handler for GET requests.
        //.user_ctx  = server_data // Optional user context for the handler.
    };
    httpd_register_uri_handler(server, &_root_get_handler); // Register the handler for GET requests.

    /* URI handler for POST requests */
    httpd_uri_t _root_post_handler = {
        .uri = "/post",               // URI path for the post handler.
        .method = HTTP_POST,          // Handler for POST requests.
        .handler = root_post_handler, // Function pointer to the handler for POST requests.
        //.user_ctx  = server_data // Optional user context for the handler.
    };
    httpd_register_uri_handler(server, &_root_post_handler); // Register the handler for POST requests.

    /* URI handler for favicon.ico */
    httpd_uri_t _favicon_get_handler = {
        .uri = "/favicon.ico",          // URI path for the favicon handler.
        .method = HTTP_GET,             // Handler for GET requests.
        .handler = favicon_get_handler, // Function pointer to the handler for GET requests.
        //.user_ctx  = server_data // Optional user context for the handler.
    };
    httpd_register_uri_handler(server, &_favicon_get_handler); // Register the handler for GET requests of the favicon.

    return ESP_OK; // Return success status.
}

/**
 * @brief HTTP server task for handling specific functionality.
 * @param pvParameters Task parameters.
 */
void http_server_task_1(void *pvParameters)
{
    char *task_parameter = (char *)pvParameters;              // Extract the task parameter (server address) from the input.
    ESP_LOGI(TAG, "Start task_parameter=%s", task_parameter); // Log: Indicate the start of the task with the specified parameter.
    char url[64];
    sprintf(url, "http://%s:%d", task_parameter, CONFIG_WEB_PORT); // Create a URL string using the task parameter and server port.

    // Start Server
    ESP_LOGI(TAG, "Starting server on %s", url);               // Log: Indicate the start of the server on the specified URL.
    ESP_ERROR_CHECK(start_server("/spiffs", CONFIG_WEB_PORT)); // Start the server with the specified base path and port.

    URL_t urlBuf; // Declare a structure to store URL information.
    while (1)
    {
        printf("http_server_task_1 loop"); // Print a message indicating the start of the task loop.
        // Waiting for post
        if (xQueueReceive(xQueueHttp, &urlBuf, portMAX_DELAY) == pdTRUE) // Check if there is a message in the HTTP queue.
        {
            printf("http_server_task_1 xQueueReceive if");                                                                        // Print a message indicating the execution of the conditional block.
            ESP_LOGI(TAG, "str_value=%s long_value=%ld variable_id=%s", urlBuf.str_value, urlBuf.long_value, urlBuf.variable_id); // Log: Print received URL information.

            // Copy received URL information to global variables
            strcpy(variable_id_g, urlBuf.variable_id);
            strcpy(str_value_g, urlBuf.str_value);
            long_value_g = urlBuf.long_value;
        }
        vTaskDelay(2000); // Delay the task for 2000 milliseconds.
    }

    // Never reach here
    ESP_LOGI(TAG, "finish"); // Log: Indicate that the task has finished.
    vTaskDelete(NULL);       // Delete the task (should never reach this point).
}
