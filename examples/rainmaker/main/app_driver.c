/*  
   Stetel Things srl - RainMaker template for Maia DevBoard

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

//#include <sdkconfig.h>
#include <driver/rmt.h>

#include <esp_system.h>
#include "app_driver.h"
#include "driver/temp_sensor.h"



static const char *TAG = "app_driver";

bool _initialized = false;
float tsens_out;
static esp_timer_handle_t sensor_timer;


float round_f(float var) 
{ 
    float value = (int)(var * 10 + .5); 
    return (float)value / 10; 
} 

void initSensTemp(){
    ESP_LOGI(TAG, "Initializing Temperature sensor");
    temp_sensor_config_t temp_sensor = TSENS_CONFIG_DEFAULT();
    temp_sensor_get_config(&temp_sensor);
    ESP_LOGI(TAG, "default dac %d, clk_div %d", temp_sensor.dac_offset, temp_sensor.clk_div);
    temp_sensor.dac_offset = TSENS_DAC_DEFAULT; // DEFAULT: range:-10℃ ~  80℃, error < 1℃.
    temp_sensor_set_config(temp_sensor);
    temp_sensor_start();
    ESP_LOGI(TAG, "Temperature sensor started");
}

float getTemperture()
{
    if(!_initialized) {
        initSensTemp();
        _initialized = true;
    }

    temp_sensor_read_celsius(&tsens_out);
    ESP_LOGI(TAG, "Temperature out celsius %f°C", round_f(tsens_out));

    return round_f(tsens_out);
}

static void app_sensor_update(void *priv)
{
    tsens_out = getTemperture();
    esp_rmaker_update_param("Temperature Sensor", ESP_RMAKER_DEF_TEMPERATURE_NAME, esp_rmaker_float(tsens_out)); 
}

esp_err_t app_sensor_init(void)
{
    
    tsens_out = getTemperture();
    esp_timer_create_args_t sensor_timer_conf = {
        .callback = app_sensor_update,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "app_sensor_update_tm"
    };
    if (esp_timer_create(&sensor_timer_conf, &sensor_timer) == ESP_OK) {
        esp_timer_start_periodic(sensor_timer, CONFIG_APP_REPORTING_PERIOD * 1000000U);
        return ESP_OK;
    }
    return ESP_FAIL;
    
}

static void button_press_3sec_cb(void *arg)
{
    nvs_flash_deinit();
    nvs_flash_erase();
    esp_restart();
}

esp_err_t app_driver_set_gpio(const char *name, bool state)
{
    if (strcmp(name, "Red") == 0) {
        gpio_set_level(CONFIG_APP_REDLED_GPIO, state);
    } else if (strcmp(name, "Green") == 0) {
        gpio_set_level(CONFIG_APP_GREENLED_GPIO, state);
    } else if (strcmp(name, "Blue") == 0) {
        gpio_set_level(CONFIG_APP_BLUELED_GPIO, state);
    } else {
        return ESP_FAIL;
    }
    return ESP_OK;
}

void app_driver_init()
{

    button_handle_t btn_handle = iot_button_create(CONFIG_APP_BUTTON_GPIO, CONFIG_APP_BUTTON_ACTIVE_LEVEL);
    if (btn_handle) {
        iot_button_add_on_press_cb(btn_handle, 3, button_press_3sec_cb, NULL);
    }


    gpio_config_t io_conf = {
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = 1,
    };
    uint64_t pin_mask = (((uint64_t)1 << CONFIG_APP_REDLED_GPIO ) | ((uint64_t)1 << CONFIG_APP_GREENLED_GPIO ) | ((uint64_t)1 << CONFIG_APP_BLUELED_GPIO ));
    io_conf.pin_bit_mask = pin_mask;
   
    gpio_config(&io_conf);
    gpio_set_level(CONFIG_APP_REDLED_GPIO, false);
    gpio_set_level(CONFIG_APP_GREENLED_GPIO, false);
    gpio_set_level(CONFIG_APP_BLUELED_GPIO, false);


    app_sensor_init();

}


