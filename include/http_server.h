#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include "esp_http_server.h"
// #include "config.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @file http_server.h
     * @brief Header file for HTTP server functions.
     */

    extern char variable_id_g[64]; /**< Global variable for storing a variable ID. */
    extern char str_value_g[64];   /**< Global variable for storing a string value. */
    extern int long_value_g;       /**< Global variable for storing a long value. */

    /**
     * @brief Structure to represent URL parameters.
     */
    typedef struct
    {
        char str_value[64];   /**< String value from the URL. */
        long long_value;      /**< Long value from the URL. */
        char variable_id[64]; /**< Variable ID from the URL. */
    } URL_t;

    extern QueueHandle_t xQueueHttp; /**< Queue handle for communication between tasks. */

    /**
     * @brief Finds the value associated with a key in a parameter string.
     * @param key The key to search for.
     * @param parameter The parameter string.
     * @param value The output value.
     * @return The length of the found value.
     */
    int find_key_value(char *key, char *parameter, char *value);

    /**
     * @brief Sends text data as an HTML response.
     * @param req HTTP request structure.
     * @param filename Name of the HTML file.
     * @return ESP_OK if successful, otherwise ESP_FAIL.
     */
    esp_err_t Text2Html(httpd_req_t *req, char *filename);

    /**
     * @brief Sends image data as an HTML response.
     * @param req HTTP request structure.
     * @param filename Name of the image file.
     * @param type Type of the image (e.g., "jpeg", "jpg", "png").
     * @return ESP_OK if successful, otherwise ESP_FAIL.
     */
    esp_err_t Image2Html(httpd_req_t *req, char *filename, char *type);

    /**
     * @brief Starts the HTTP server.
     * @param base_path The base path for serving files.
     * @param port The port number for the server.
     * @return ESP_OK if successful, otherwise ESP_FAIL.
     */
    esp_err_t start_server(const char *base_path, int port);

    /**
     * @brief HTTP server task to handle requests and responses.
     * @param pvParameters Task parameters.
     */
    void http_server_task(void *pvParameters);

    /**
     * @brief Additional HTTP server task for handling specific functionality.
     * @param pvParameters Task parameters.
     */
    void http_server_task_1(void *pvParameters);

#ifdef __cplusplus
}
#endif

#endif /* HTTP_SERVER_H */
