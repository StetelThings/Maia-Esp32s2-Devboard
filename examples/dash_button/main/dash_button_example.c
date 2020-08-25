/* 
    Dash button example for Maia DevBoard.
    Based on ESP HTTP Client Example.

*/

#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "protocol_examples_common.h"
#include "esp_tls.h"
#include "driver/gpio.h"

#include "esp_http_client.h"


#define BOOT_BUTTON_PIN                 0
#define BOOT_BUTTON_PRESSED_LVL         0

#define PWR_OFF_PIN  	               12
#define PWR_OFF_LVL     	            1

#define RGB_LED_R_PIN                   8
#define RGB_LED_G_PIN                   9

#define MAX_HTTP_RECV_BUFFER          512
#define MAX_HTTP_OUTPUT_BUFFER       2048



#define HOST_NAME       "httpbin.org"




static const char *TAG = "HTTP_CLIENT";


static void power_off() {
    ESP_LOGI(TAG, "Finish DASH_BUTTON_EXAMPLE");

    ESP_LOGI("Power","Powering off...");
    ESP_LOGW("Power", "To turn the board back on, press USER button");
    fflush(stdout);
    vTaskDelay(3000/portTICK_PERIOD_MS);
    gpio_set_level(PWR_OFF_PIN, PWR_OFF_LVL);
    vTaskDelay(2000/portTICK_PERIOD_MS);
    ESP_LOGE("Power","Powering off FAILED");
    
    while (1)
    {
        ESP_LOGW("Power", "Board is still powered on");
        vTaskDelay(2000/portTICK_PERIOD_MS);
    }
}


esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
    static char *output_buffer;  // Buffer to store response of http request from event handler
    static int output_len;       // Stores number of bytes read
    switch(evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGD(TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGD(TAG, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
            break;
        case HTTP_EVENT_ON_DATA:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            /*
             *  Check for chunked encoding is added as the URL for chunked encoding used in this example returns binary data.
             *  However, event handler can also be used in case chunked encoding is used.
             */
            if (!esp_http_client_is_chunked_response(evt->client)) {
                // If user_data buffer is configured, copy the response into the buffer
                if (evt->user_data) {
                    memcpy(evt->user_data + output_len, evt->data, evt->data_len);
                } else {
                    if (output_buffer == NULL) {
                        output_buffer = (char *) malloc(esp_http_client_get_content_length(evt->client));
                        output_len = 0;
                        if (output_buffer == NULL) {
                            ESP_LOGE(TAG, "Failed to allocate memory for output buffer");
                            return ESP_FAIL;
                        }
                    }
                    memcpy(output_buffer + output_len, evt->data, evt->data_len);
                }
                output_len += evt->data_len;
            }

            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
            if (output_buffer != NULL) {
                // Response is accumulated in output_buffer. Uncomment the below line to print the accumulated response
                // ESP_LOG_BUFFER_HEX(TAG, output_buffer, output_len);
                free(output_buffer);
                output_buffer = NULL;
            }
            output_len = 0;
            break;
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
            int mbedtls_err = 0;
            esp_err_t err = esp_tls_get_and_clear_last_error(evt->data, &mbedtls_err, NULL);
            if (err != 0) {
                if (output_buffer != NULL) {
                    free(output_buffer);
                    output_buffer = NULL;
                }
                output_len = 0;
                ESP_LOGI(TAG, "Last esp error code: 0x%x", err);
                ESP_LOGI(TAG, "Last mbedtls failure: 0x%x", mbedtls_err);
            }

            power_off();

            break;
    }
    return ESP_OK;
}

static void http_rest_with_url(void)
{
    char local_response_buffer[MAX_HTTP_OUTPUT_BUFFER] = {0};
    /**
     * NOTE: All the configuration parameters for http_client must be spefied either in URL or as host and path parameters.
     * If host and path parameters are not set, query parameter will be ignored. In such cases,
     * query parameter should be specified in URL.
     *
     * If URL as well as host and path parameters are specified, values of host and path will be considered.
     */
    esp_http_client_config_t config = {
        .host = HOST_NAME,
        .path = "/get",
        .query = "esp",
        .event_handler = _http_event_handler,
        .user_data = local_response_buffer,        // Pass address of local buffer to get response
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);


    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "HTTP GET Status = %d, content_length = %d",
                esp_http_client_get_status_code(client),
                esp_http_client_get_content_length(client));

        if (esp_http_client_get_status_code(client) == 200) {
            ESP_LOGI("Maia Board", "Green LED ON!");
            vTaskDelay(1000/portTICK_PERIOD_MS);
            gpio_set_level(RGB_LED_G_PIN, 1);
        }
        else {
            ESP_LOGI("Maia Board", "Red LED ON...");
            vTaskDelay(1000/portTICK_PERIOD_MS);
            gpio_set_level(RGB_LED_R_PIN, 1);
        }

    } else {
        ESP_LOGE(TAG, "HTTP GET request failed: %s", esp_err_to_name(err));
        ESP_LOGE("Maia Board", "Red LED ON...");
        vTaskDelay(1000/portTICK_PERIOD_MS);
        gpio_set_level(RGB_LED_R_PIN, 1);
    }
    

    esp_http_client_cleanup(client);
}




static void http_test_task(void *pvParameters)
{
    http_rest_with_url();

    vTaskDelete(NULL);
}



void app_main(void)
{

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    //GPIOs reset needed in ESP32-S2 BETA
    gpio_reset_pin(RGB_LED_R_PIN);
    gpio_reset_pin(RGB_LED_G_PIN);
    gpio_reset_pin(PWR_OFF_PIN);
    
    gpio_set_direction(RGB_LED_R_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(RGB_LED_G_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(PWR_OFF_PIN, GPIO_MODE_OUTPUT);
	
    gpio_set_level(RGB_LED_G_PIN, 0);
    gpio_set_level(RGB_LED_R_PIN, 0);


    ESP_LOGI("DASH_BUTTON_EXAMPLE", "START!");


    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    ESP_ERROR_CHECK(example_connect());
    ESP_LOGI(TAG, "Connected to AP");

    xTaskCreate(&http_test_task, "http_test_task", 8192, NULL, 5, NULL);

}
