/* 
    Stetel Things srl - RainMaker template for Maia DevBoard

    Unless required by applicable law or agreed to in writing, this
    software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
    CONDITIONS OF ANY KIND, either express or implied.
*/


#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <string.h>


#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <esp_log.h>
#include <nvs_flash.h>

#include <esp_rmaker_core.h>
#include <esp_rmaker_standard_params.h> 
#include <esp_rmaker_standard_devices.h>
#include <esp_rmaker_standard_types.h>

#include <iot_button.h>


#define REPORTING_PERIOD    60 /* Seconds */


#define BUTTON_GPIO          11
#define BUTTON_ACTIVE_LEVEL  0

#define OUTPUT_GPIO_RED   8
#define OUTPUT_GPIO_GREEN 9
#define OUTPUT_GPIO_BLUE  10



esp_err_t app_sensor_init(void);
float getTemperture();
void app_driver_init();
esp_err_t app_driver_set_gpio(const char *name, bool state);
