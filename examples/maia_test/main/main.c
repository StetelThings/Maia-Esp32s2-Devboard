/*  
    Maia DevBoard testing app
*/
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "esp_efuse.h"
//#include "tinyusb.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#include "soc/rtc.h"
#include "sdkconfig.h"

/* The examples use WiFi configuration that you can set via project configuration menu.

   If you'd rather not, just change the below entries to strings with
   the config you want - ie #define EXAMPLE_WIFI_SSID "mywifissid"
*/
#define EXAMPLE_ESP_WIFI_SSID      CONFIG_ESP_WIFI_SSID
#define EXAMPLE_ESP_WIFI_PASS      CONFIG_ESP_WIFI_PASSWORD
#define EXAMPLE_MAX_STA_CONN       CONFIG_ESP_MAX_STA_CONN

#define RGB_LED_R_PIN   8
#define RGB_LED_G_PIN   9
#define RGB_LED_B_PIN   10

#define RGB_LED_ON_LVL  	0

#define CHG_LED_DISABLE_PIN	    13
#define CHG_LED_DISABLE_LVL	    1

#define CHG_STATUS_PIN		    14
#define CHG_STATUS_CHG_LVL      1

#define PWR_OFF_PIN  	        12
#define PWR_OFF_LVL     	    1

#define USER_BUTTON_PIN         11
#define USER_BUTTON_PRESSED_LVL 1

#define BOOT_BUTTON_PIN         0
#define BOOT_BUTTON_PRESSED_LVL 0

static const char *TAG = "wifi softAP";
static int WiFiConnected = 0;
int i;

static int read_board_data()
{
    char buffer[20];
    char res[100];
    char tempstr[100];
    char p_key[100] = "\0";
    const char *padding="00000000";
    int targetStrLen = 8;
    int i;

    ESP_LOGI("DATA", "MAC Address: %x%08x", esp_efuse_read_reg(1, 1), esp_efuse_read_reg(1, 0));

    for (i=0; i<4; i++) {
        itoa(esp_efuse_read_reg(2, i), buffer, 16);
    
        int padLen = targetStrLen - strlen(buffer);
        if(padLen < 0) padLen = 0;
        sprintf(res, "%*.*s%s", padLen, padLen, padding, buffer);

        //ESP_LOGI(TAG, "string: %s", res);
        //ESP_LOGI(TAG, "inv_string: %c%c%c%c%c%c%c%c", res[6], res[7], res[4], res[5], res[2], res[3], res[0], res[1]);
        sprintf(tempstr, "%c%c%c%c%c%c%c%c", res[6], res[7], res[4], res[5], res[2], res[3], res[0], res[1]);
        strcat(p_key, tempstr);
    }

    ESP_LOGI("DATA", "P_KEY: %s", p_key);
    printf("\n");
    printf("%x%08x\t%s", esp_efuse_read_reg(1, 1), esp_efuse_read_reg(1, 0),p_key);
    printf("\n\n");
    return 0;

}

static int wait_for_user_button_confirmation()
{
    ESP_LOGW("GPIO", "Press user button within 10 seconds to confirm");
    for (int i=0; i<100; i++)
    {
        if (gpio_get_level(USER_BUTTON_PIN) == USER_BUTTON_PRESSED_LVL) return 1;
        vTaskDelay(100/portTICK_PERIOD_MS);
    }
    return 0;
}

static int wait_for_boot_button_confirmation()
{
    ESP_LOGW("GPIO", "Press boot button within 10 seconds to confirm");
    for (int i=0; i<100; i++)
    {
        if (gpio_get_level(BOOT_BUTTON_PIN) == BOOT_BUTTON_PRESSED_LVL) return 1;
        vTaskDelay(100/portTICK_PERIOD_MS);
    }
    return 0;
}

static int yesnobutton()
{
    ESP_LOGW("YES-NO", "Press USER button if YES or BOOT button if NO, within 30 seconds");
    for (int i=0; i<300; i++)
    {
        if (gpio_get_level(BOOT_BUTTON_PIN) == BOOT_BUTTON_PRESSED_LVL) return 0;
        if (gpio_get_level(USER_BUTTON_PIN) == USER_BUTTON_PRESSED_LVL) return 1;
        vTaskDelay(100/portTICK_PERIOD_MS);
    }
    return 0;
}

static int wait_for_wifi_connection()
{
    ESP_LOGW(TAG, "PLEASE CONNECT TO WIFI ACCESS POINT WITHIN 60 SECONDS.\n\tSSID: '"CONFIG_ESP_WIFI_SSID"'\n\tPASSWORD: '"CONFIG_ESP_WIFI_PASSWORD"'");
    for (i=0;i<60; i++)
    {
        vTaskDelay(1000/portTICK_PERIOD_MS);
        if (WiFiConnected) return 1;
    }
    return 0;
}

static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                    int32_t event_id, void* event_data)
{
    if (event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" join, AID=%d",
                 MAC2STR(event->mac), event->aid);
        WiFiConnected=1;
    } else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" leave, AID=%d",
                 MAC2STR(event->mac), event->aid);
        WiFiConnected=0;
    }
}

void wifi_init_softap(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .ssid_len = strlen(EXAMPLE_ESP_WIFI_SSID),
            .password = EXAMPLE_ESP_WIFI_PASS,
            .max_connection = EXAMPLE_MAX_STA_CONN,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK
        },
    };
    if (strlen(EXAMPLE_ESP_WIFI_PASS) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_softap finished. SSID:%s password:%s",
             EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
}

void app_main(void)
{
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    int i;
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    //GPIOs reset needed in ESP32-S2 BETA
    gpio_reset_pin(RGB_LED_R_PIN);
    gpio_reset_pin(RGB_LED_G_PIN);
    gpio_reset_pin(RGB_LED_B_PIN);
    gpio_reset_pin(PWR_OFF_PIN);
	gpio_reset_pin(CHG_LED_DISABLE_PIN);
    gpio_reset_pin(CHG_STATUS_PIN);
    gpio_reset_pin(USER_BUTTON_PIN);
    
    gpio_set_direction(RGB_LED_R_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(RGB_LED_G_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(RGB_LED_B_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(PWR_OFF_PIN, GPIO_MODE_OUTPUT);
	gpio_set_direction(CHG_LED_DISABLE_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(CHG_STATUS_PIN, GPIO_MODE_INPUT);
    gpio_set_direction(USER_BUTTON_PIN, GPIO_MODE_INPUT);

    gpio_set_level(RGB_LED_G_PIN, 0);
    gpio_set_level(RGB_LED_B_PIN, 0);
    gpio_set_level(RGB_LED_R_PIN, 0);

    ESP_LOGI("TEST", "******************* MAIA BOARD TEST **********************");

    vTaskDelay(1000/portTICK_PERIOD_MS);

    ESP_LOGW("TEST", "Press USER button to start test");

    while (gpio_get_level(USER_BUTTON_PIN) != USER_BUTTON_PRESSED_LVL)
    {
        vTaskDelay(100/portTICK_PERIOD_MS);
    }
    ESP_LOGI("TEST", "TEST STARTED! User Button WORKING!");

    //////////////////////////////////////// PRINT BOARD DATA ///////////////////

    read_board_data();

    //////////////////////////////////////// BOOT button test ////////////////////////
    vTaskDelay(500/portTICK_PERIOD_MS);
    ESP_LOGW("TEST", "Press BOOT button to continue");

    while (gpio_get_level(BOOT_BUTTON_PIN) != BOOT_BUTTON_PRESSED_LVL)
    {
        vTaskDelay(100/portTICK_PERIOD_MS);
    }
    ESP_LOGI("TEST", "User Button WORKING!");

    ///////////////////////////////////// 32 kHz OSCILLATOR TEST ///////////////////

    ESP_LOGI("RTC CLOCK", "32 kHZ CRYSTAL OSCILLATOR  TEST");

    // rtc_clk_cal returns the period of the RTC clock in microseconds, with fixed point Q13.19 format
    // Note that the period is measured with the main clock precision.
    uint32_t cal_val = rtc_clk_cal(RTC_SLOW_FREQ_32K_XTAL, CONFIG_ESP32S2_RTC_CLK_CAL_CYCLES);
    uint32_t cal_val_int = (cal_val & 0xFFF80000) >> 19;  
    uint32_t cal_val_frac = (cal_val & 0x7FFFF);
    ESP_LOGI("RTC CLOCK", "RTC CLOCK PERIOD = %u.%u microseconds" , cal_val_int, cal_val_frac);
    if (cal_val_int > 28 && cal_val_int<32)
    {
        ESP_LOGI("RTC CLOCK", "32 kHz RTC crystal oscillator test PASSED");
    }
    else
    {
        ESP_LOGE("RTC CLOCK", "32 kHz RTC clock test FAILED");
    }

    //////////////////////////////////////// RED LED TEST ///////////////////
    ESP_LOGW("GPIO", "Red LED test: led is ON?");
    vTaskDelay(1000/portTICK_PERIOD_MS);
    gpio_set_level(RGB_LED_R_PIN, 1);
    //if(wait_for_user_button_confirmation()) 
    if(yesnobutton()) 
    {
        ESP_LOGI("GPIO", "Red LED test PASSED");
    }
    else
    {
        ESP_LOGE("GPIO", "Red LED test FAILED");
    }
    gpio_set_level(RGB_LED_R_PIN, 0);

    //////////////////////////////////////// GREEN LED TEST ///////////////////
    ESP_LOGW("GPIO", "Green LED test: led is ON?");
    vTaskDelay(1000/portTICK_PERIOD_MS);
    gpio_set_level(RGB_LED_G_PIN, 1);
    //if(wait_for_user_button_confirmation()) 
    if(yesnobutton())
    {
        ESP_LOGI("GPIO", "Green LED test PASSED");
    }
    else
    {
        ESP_LOGE("GPIO", "Green LED test FAILED");
    }
    gpio_set_level(RGB_LED_G_PIN, 0);

    //////////////////////////////////////// BLUE LED TEST ///////////////////
    ESP_LOGW("GPIO", "Blue LED test: led is ON?");
    vTaskDelay(1000/portTICK_PERIOD_MS);
    gpio_set_level(RGB_LED_B_PIN, 1);
    //if(wait_for_user_button_confirmation())
    if(yesnobutton())
    {
        ESP_LOGI("GPIO", "Blue LED test PASSED");
    }
    else
    {
        ESP_LOGE("GPIO", "Blue LED test FAILED");
    }
    gpio_set_level(RGB_LED_B_PIN, 0);

    //ALL LEDs ON
	ESP_LOGI("GPIO", "All LEDs on");
    vTaskDelay(100/portTICK_PERIOD_MS);
    gpio_set_level(RGB_LED_G_PIN, 1);
    gpio_set_level(RGB_LED_B_PIN, 1);
    gpio_set_level(RGB_LED_R_PIN, 1);
	vTaskDelay(500/portTICK_PERIOD_MS);
	gpio_set_level(RGB_LED_G_PIN, 0);
    gpio_set_level(RGB_LED_B_PIN, 0);
    gpio_set_level(RGB_LED_R_PIN, 0);


    //////////////////////////////////////// CHARGING TEST, CASE 1 ///////////////////
    vTaskDelay(1000/portTICK_PERIOD_MS);
    ESP_LOGI("Battery","Battery charging sensing and LED test - CASE 1");
    ESP_LOGW("Battery", "PLEASE MAKE SURE THAT:\n\t1- USB POWER IS CONNECTED,\n\t2- BATTERY IS NOT CONNECTED,\n\tTHEN PRESS USER BUTTON TO CONTINUE");
    while (gpio_get_level(USER_BUTTON_PIN) != USER_BUTTON_PRESSED_LVL) vTaskDelay(200/portTICK_PERIOD_MS);
	
    if (gpio_get_level(CHG_STATUS_PIN)==CHG_STATUS_CHG_LVL) 
	{
		ESP_LOGI("Battery","Battery charging sensing is: CHARGING");
        ESP_LOGI("Battery","Battery charging sensing test PASSED for CASE 1");
	}
    else
    {
        ESP_LOGI("Battery","Battery cherging sensing is: NOT CHARGING");
        ESP_LOGE("Battery","Battery charging sensing test FAILED for CASE 1");
    }

    vTaskDelay(1000/portTICK_PERIOD_MS);
    ESP_LOGW("Battery", "IS CHARGING STATUS LED ON?");
    //if(wait_for_user_button_confirmation())
    if(yesnobutton())
    {
        ESP_LOGI("Battery", "Charging status LED ON test PASSED for CASE 1");
    }
    else
    {
        ESP_LOGE("Battery", "Charging status LED ON test FAILED for CASE 1");
    }

    //////////////////////////////////////// CHARGING TEST, CASE 2 ///////////////////
    vTaskDelay(1000/portTICK_PERIOD_MS);
    ESP_LOGI("Battery","Battery charging sensing and LED test - CASE 2");
    ESP_LOGW("Battery", "PLEASE MAKE SURE THAT:\n\t1- USB POWER IS CONNECTED,\n\t2- BATTERY IS CONNECTED,\n\t3- BATTERY IS NOT FULL CHARGED,\n\tTHEN PRESS USER BUTTON TO CONTINUE");
    while (gpio_get_level(USER_BUTTON_PIN) != USER_BUTTON_PRESSED_LVL) vTaskDelay(200/portTICK_PERIOD_MS);
	
    if (gpio_get_level(CHG_STATUS_PIN)==CHG_STATUS_CHG_LVL) 
	{
		ESP_LOGI("Battery","Battery charging sensing is: CHARGING");
        ESP_LOGI("Battery","Battery charging sensing test PASSED for CASE 2");
	}
    else
    {
        ESP_LOGI("Battery","Battery cherging sensing is: NOT CHARGING");
        ESP_LOGE("Battery","Battery charging sensing test FAILED for CASE 2");
    }
    
    vTaskDelay(1000/portTICK_PERIOD_MS);
    ESP_LOGW("Battery", "IS CHARGING LED ON?");
    //if(wait_for_user_button_confirmation())
    if(yesnobutton())
    {
        ESP_LOGI("Battery", "Charging status LED ON test PASSED for CASE 2");
    }
    else
    {
        ESP_LOGE("Battery", "Charging status LED ON test FAILED for CASE 2");
    }

    vTaskDelay(1000/portTICK_PERIOD_MS);
    ESP_LOGI("Battery", "Disabling charging status LED");
    gpio_set_level(CHG_LED_DISABLE_PIN, 1);
    ESP_LOGW("Battery", "IS CHARGING STATUS LED OFF?");
    //if(wait_for_user_button_confirmation())
    if(yesnobutton())
    {
        ESP_LOGI("Battery", "Charging status LED OFF test PASSED for CASE 2");
    }
    else
    {
        ESP_LOGE("Battery", "Charging status LED OFF test FAILED for CASE 2");
    }
	ESP_LOGI("Battery","Re-enabling charging status LED");
    gpio_set_level(CHG_LED_DISABLE_PIN, 0);
        

    //////////////////////////////////////// CHARGING TEST, CASE 3 ///////////////////
    // vTaskDelay(1000/portTICK_PERIOD_MS);
    // ESP_LOGI("Battery","Battery charging sensing and LED test - CASE 3");
    // ESP_LOGW("Battery", "PLEASE MAKE SURE THAT:\n1- BATTERY IS CONNECTED,\n\t2- BATTERY VOLTAGE > 3.6 Volt,\n\t3- USB POWER IS NOT CONNECTED,\n\tTHEN PRESS USER BUTTON TO CONTINUE");
    // while (gpio_get_level(USER_BUTTON_PIN) != USER_BUTTON_PRESSED_LVL) vTaskDelay(200/portTICK_PERIOD_MS);
	
    // if (gpio_get_level(CHG_STATUS_PIN)==CHG_STATUS_CHG_LVL) 
	// {
	// 	ESP_LOGI("Battery","Battery charging sensing is: CHARGING");
    //     ESP_LOGE("Battery","Battery charging sensing test FAILED for CASE 3");
	// }
    // else
    // {
    //     ESP_LOGI("Battery","Battery cherging sensing is: NOT CHARGING");
    //     ESP_LOGI("Battery","Battery charging sensing test PASSED for CASE 3");
    // }

    // vTaskDelay(1000/portTICK_PERIOD_MS);
    // ESP_LOGW("Battery", "IS CHARGING STATUS LED OFF?");
    // //if(wait_for_user_button_confirmation())
    // if(yesnobutton())
    // {
    //     ESP_LOGI("Battery", "Charging status LED OFF test PASSED for CASE 3");
    // }
    // else
    // {
    //     ESP_LOGE("Battery", "Charging status LED OFF test FAILED for CASE 3");
    // }
    
    //////////////////////////////////////// CHARGING TEST, CASE 4 ///////////////////
    vTaskDelay(1000/portTICK_PERIOD_MS);
    ESP_LOGI("Battery","Battery charging sensing and LED test - CASE 4");
    ESP_LOGW("Battery", "PLEASE MAKE SURE THAT:\n\t1- USB POWER IS CONNECTED,\n\t2- BATTERY IS CONNECTED,\n\t3- BATTERY IS FULL CHARGED,\n\tTHEN PRESS USER BUTTON TO CONTINUE");
    while (gpio_get_level(USER_BUTTON_PIN) != USER_BUTTON_PRESSED_LVL) vTaskDelay(200/portTICK_PERIOD_MS);
	
    if (gpio_get_level(CHG_STATUS_PIN)==CHG_STATUS_CHG_LVL) 
	{
		ESP_LOGI("Battery","Battery charging sensing is: CHARGING");
        ESP_LOGE("Battery","Battery charging sensing test FAILED for CASE 4");
        gpio_set_level(RGB_LED_R_PIN, 1);
	}
    else
    {
        ESP_LOGI("Battery","Battery cherging sensing is: NOT CHARGING");
        ESP_LOGI("Battery","Battery charging sensing test PASSED for CASE 4");
        gpio_set_level(RGB_LED_G_PIN, 1);
    }
    vTaskDelay(1000/portTICK_PERIOD_MS);
    while (gpio_get_level(USER_BUTTON_PIN) != USER_BUTTON_PRESSED_LVL) vTaskDelay(200/portTICK_PERIOD_MS);
    gpio_set_level(RGB_LED_G_PIN, 0);
    gpio_set_level(RGB_LED_R_PIN, 0);

   /* 
    vTaskDelay(1000/portTICK_PERIOD_MS);
    ESP_LOGW("Battery", "IS CHARGING LED OFF?");
    //if(wait_for_user_button_confirmation())
    if(yesnobutton())
    {
        ESP_LOGI("Battery", "Charging status LED OFF test PASSED for CASE 4");
    }
    else
    {
        ESP_LOGE("Battery", "Charging status LED OFF test FAILED for CASE 4");
    }
    */
    //////////////////////////////////////// WIFI TEST ///////////////////
    ESP_LOGI(TAG, "Starting ESP_WIFI_MODE_AP...");
    vTaskDelay(1000/portTICK_PERIOD_MS);
    wifi_init_softap();
    vTaskDelay(3000/portTICK_PERIOD_MS);
    ESP_LOGI(TAG, "WiFi Access Point test");
    ESP_LOGW(TAG, "Is '"CONFIG_ESP_WIFI_SSID"' WiFi active?");
    if (yesnobutton())
    {
        ESP_LOGI(TAG, "WiFi Access Point test PASSED");
    }
    else
    {
        ESP_LOGE(TAG, "WiFi Access Point test FAILED");
    }

    //////////////////////////////////////// POWER OFF TEST ///////////////////
    vTaskDelay(1000/portTICK_PERIOD_MS);
    gpio_set_level(RGB_LED_B_PIN, 1);
    ESP_LOGI("Power", "Board power down test");
    ESP_LOGI ("Power", "Blue led is for power status check");
    ESP_LOGW("Power", "PRESS BOOT BUTTON TO POWER OFF THE BOARD");
    while(gpio_get_level(BOOT_BUTTON_PIN) != BOOT_BUTTON_PRESSED_LVL) vTaskDelay(100/portTICK_PERIOD_MS); 
    ESP_LOGI("Power","Powering off: PASSED unless other logs.");
    fflush(stdout);
    vTaskDelay(200/portTICK_PERIOD_MS);
    gpio_set_level(PWR_OFF_PIN, PWR_OFF_LVL);
    vTaskDelay(2000/portTICK_PERIOD_MS);
    ESP_LOGE("Power","Powering off test FAILED");
    
    while (1)
    {
        ESP_LOGW("Power", "Board is still powered on");
        vTaskDelay(2000/portTICK_PERIOD_MS);
    }
}
