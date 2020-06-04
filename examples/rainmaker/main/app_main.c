/* 
    Stetel Things srl - RainMaker template for Maia DevBoard

    Unless required by applicable law or agreed to in writing, this
    software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
    CONDITIONS OF ANY KIND, either express or implied.
*/


#include "app_driver.h"
#include <app_wifi.h>


static const char *TAG = "app_main";




/* Callback to handle commands received from the RainMaker cloud */
static esp_err_t gpio_callback(const char *dev_name, const char *name, esp_rmaker_param_val_t val, void *priv_data)
{
    if (app_driver_set_gpio(name, val.val.b) == ESP_OK) {
        esp_rmaker_update_param(dev_name, name, val);
    }
    return ESP_OK;
}




void app_main()
{
    /* Initialize Application specific hardware drivers and
     * set initial state.
     */
    app_driver_init();


    /* Initialize NVS. */
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK( err );

    /* Initialize Wi-Fi. Note that, this should be called before esp_rmaker_init()
     */
    app_wifi_init();
    
    /* Initialize the ESP RainMaker Agent.
     * Note that this should be called after app_wifi_init() but before app_wifi_start()
     * */
    esp_rmaker_config_t rainmaker_cfg = {
        .info = {
            .name = "Maia RainMaker template",
            .type = "Multi Device",
        },
        .enable_time_sync = false,
    };
    err = esp_rmaker_init(&rainmaker_cfg);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Could not initialise ESP RainMaker. Aborting!!!");
        vTaskDelay(5000/portTICK_PERIOD_MS);
        abort();
    }

    /* Create a Temperature Sensor device and add the relevant parameters to it */
    esp_rmaker_create_temp_sensor_device("Temperature Sensor", NULL, NULL, getTemperture());

    /* Create a device and add the relevant parameters to it */
    esp_rmaker_create_device("RGB_LED", NULL, gpio_callback, NULL);
    esp_rmaker_device_add_param("RGB_LED", "Red", esp_rmaker_bool(false), PROP_FLAG_READ | PROP_FLAG_WRITE);
    esp_rmaker_param_add_ui_type("RGB_LED", "Red", ESP_RMAKER_UI_TOGGLE);
    esp_rmaker_device_add_param("RGB_LED", "Green", esp_rmaker_bool(false), PROP_FLAG_READ | PROP_FLAG_WRITE);
    esp_rmaker_param_add_ui_type("RGB_LED", "Green", ESP_RMAKER_UI_TOGGLE);
    esp_rmaker_device_add_param("RGB_LED", "Blue", esp_rmaker_bool(false), PROP_FLAG_READ | PROP_FLAG_WRITE);
    esp_rmaker_param_add_ui_type("RGB_LED", "Blue", ESP_RMAKER_UI_TOGGLE);

    

    
    /* Start the ESP RainMaker Agent */
    esp_rmaker_start();

    /* Start the Wi-Fi.
     * If the node is provisioned, it will start connection attempts,
     * else, it will start Wi-Fi provisioning. The function will return
     * after a connection has been successfully established
     */
    app_wifi_start();
}


