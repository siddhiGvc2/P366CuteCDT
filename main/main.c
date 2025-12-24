
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_mac.h"
#include "esp_event.h"   
#include "esp_log.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/sockets.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"
#include "esp_timer.h"
#include "esp_ota_ops.h"
#include "driver/uart.h"
#include "esp_netif.h"
#include "rom/ets_sys.h"
#include "esp_smartconfig.h"
#include <sys/socket.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "NoConnectivity.c"
#include "BootingUp.c"
#include "CashReceived.c"
#include "SelectItem.c"
#include "ItemVend.c"
#include "WaitingCashLessDevice.c"
#include "NoStock.c"
#include "BlankImage.c"
#include "CoinInserted.c"

#include "calls.h"
#include "vars.h"





// void resolve_hostname(const char *);
// uint32_t millis(void);

// extern bool extractSubstring(const char* , char* );


static const char *TAG = "main";


static void memory_check(void *para) {
         vTaskDelay(pdMS_TO_TICKS(5000));
    while(1) {
        // lv_timer_handler();
                //    vTaskDelay(pdMS_TO_TICKS(5000));  // 30 msec delay
        size_t free_heap = heap_caps_get_free_size(MALLOC_CAP_8BIT);
        size_t free_internal_heap = heap_caps_get_free_size(MALLOC_CAP_INTERNAL); 
        ESP_LOGI(TAG, "Free memory: %d, free internal memory : %d ", free_heap, free_internal_heap);          
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

   
 


void show_qr_code(void *param) {
    if (DisplayMode != ModeQR) {
       

        // LV_IMG_DECLARE(QRcode);
        ESP_LOGI(TAG, "Displaying QrCode Image");
        // display_images(&QRcode);
        if (lv_obj_is_valid(img)) {
        lv_obj_del_async(img);  // Asynchronously delete the previous image object
        img = NULL;             // ✅ Good: prevent use-after-free
        }
       
        if (!lv_obj_is_valid(qr)) {

        qr = lv_qrcode_create(lv_scr_act(), QR_CODE_SIZE, lv_color_hex3(0x000), lv_color_hex3(0xFFF));
        }
        lv_qrcode_update(qr, QrString, strlen(QrString));
        lv_obj_align(qr, LV_ALIGN_CENTER, 0, -23);

        if(!lv_obj_is_valid(qr_label_bg))
        {
        qr_label_bg = lv_obj_create(lv_scr_act());
        }
        lv_obj_set_size(qr_label_bg, lv_disp_get_hor_res(NULL), 40);
        lv_obj_align_to(qr_label_bg, qr, LV_ALIGN_OUT_TOP_MID, 0, -5);
        lv_obj_set_style_bg_color(qr_label_bg, lv_color_black(), 0);
        lv_obj_set_style_bg_opa(qr_label_bg, LV_OPA_COVER, 0);
        lv_obj_clear_flag(qr_label_bg, LV_OBJ_FLAG_SCROLLABLE);

        if(!lv_obj_is_valid(qr_label))
        {
        qr_label = lv_label_create(qr_label_bg);
        }
        lv_label_set_text_fmt(qr_label, "S.No : %s", SerialNumber);
        lv_obj_align_to(qr_label, qr, LV_ALIGN_OUT_TOP_MID, 0, -20);

        if(!qr_style_initialized)
        {
        lv_style_init(&qr_style);
        lv_style_set_text_color(&qr_style, lv_color_white());
        lv_style_set_bg_color(&qr_style, lv_color_black());
        lv_style_set_bg_opa(&qr_style, LV_OPA_COVER);
        lv_style_set_pad_all(&qr_style, 4);
        qr_style_initialized=true;
        }
        lv_obj_add_style(qr_label, &qr_style, 0);

        DisplayMode = ModeQR;
    }
}


void display_image_task(void)
{
    while(1) {
        // lv_timer_handler();
                //    vTaskDelay(pdMS_TO_TICKS(5000));  // 30 msec delay
        // size_t free_heap = heap_caps_get_free_size(MALLOC_CAP_8BIT);
        // size_t free_internal_heap = heap_caps_get_free_size(MALLOC_CAP_INTERNAL); 
        // ESP_LOGI(TAG, "Free memory: %d, free internal memory : %d ", free_heap, free_internal_heap);          
        // display all images here
        // //ESP_LOGI(TAG,"%d",Image2BDisplayed);
        if (Image2BDisplayed == ImageBootingUp)
        {
            if (DisplayMode != ModeBootingUp)
            {
                // LV_IMG_DECLARE();
                LV_IMG_DECLARE(BootingUp);
                ESP_LOGI(TAG,"Displaying BootingUp Image");
                display_images(&BootingUp);
                DisplayMode = ModeBootingUp;
            }
        }

        if (Image2BDisplayed == ImageNoWifi)
        {
            if (DisplayMode != ModeNoWifi)
            {
                LV_IMG_DECLARE(NoConnectivity);
                 ESP_LOGI(TAG,"Displaying NoConnectivity Image");
                display_images(&NoConnectivity);
                DisplayMode = ModeNoWifi;
            }
        }
         
        if(Image2BDisplayed==ImageCashlessDevice)
        {
            if(DisplayMode != ModeCashlessDevice)
            {
                LV_IMG_DECLARE(WaitingCashLessDevice);
                 ESP_LOGI(TAG,"Displaying WaitingCashLessDevice Image");
                display_images(&WaitingCashLessDevice);
                DisplayMode=ModeCashlessDevice;
            }
        }

        if(Image2BDisplayed==ImageItemVend){
            if (DisplayMode != ModeItemVend)
            {
                LV_IMG_DECLARE(ItemVend);
                 ESP_LOGI(TAG,"Displaying Itemvend Image");
                display_images(&ItemVend);
                DisplayMode = ModeItemVend;
            }
        }

        if(Image2BDisplayed==ImageSelectItem)
        {
             if (DisplayMode != ModeSelectItem)
            {
                LV_IMG_DECLARE(SelectItem);
                 ESP_LOGI(TAG,"Displaying SelecteItem Image");
                display_images(&SelectItem);
                DisplayMode = ModeSelectItem;
            }
        }
        if(Image2BDisplayed==ImageCashReceived)
        {
              if (DisplayMode != ModeCashReceived)
            {
                LV_IMG_DECLARE(CashReceived);
                 ESP_LOGI(TAG,"Displaying CashReceived Image");
                display_images(&CashReceived);
                DisplayMode = ModeCashReceived;
            }
        }
         if(Image2BDisplayed==ImageCoinInserted)
        {
              if (DisplayMode != ModeCoinInserted)
            {
                LV_IMG_DECLARE(CoinInserted);
                 ESP_LOGI(TAG,"Displaying CoinInserted Image");
                display_images(&CoinInserted);
                DisplayMode = ModeCoinInserted;
            }
        }
        // this function added on 140525 by Vinay with guidance of Siddhi
        if (Image2BDisplayed == ImageQRCode)
        {
            lv_async_call(show_qr_code, NULL);
            // if (DisplayMode != ModeQR)
            // {
            //     LV_IMG_DECLARE(QRcode);
            //      ESP_LOGI(TAG,"Displaying QrCode Image");
            //     display_images(&QRcode);
                
            //     lv_obj_t *qr = lv_qrcode_create(img, QR_CODE_SIZE, lv_color_hex3(0x000), lv_color_hex3(0xFFF));
            //     lv_qrcode_update(qr, QrString, strlen(QrString));
            //     lv_obj_align(qr, LV_ALIGN_CENTER, 0, 10);

                // 2. Create a full-width black background container for the label
// lv_obj_t *label_bg = lv_obj_create(lv_scr_act());
// lv_obj_set_size(label_bg, lv_disp_get_hor_res(NULL), 40); // Full width, fixed height
// lv_obj_align_to(label_bg, qr, LV_ALIGN_OUT_TOP_MID, 0, -10); // Above the QR code

// lv_obj_set_style_bg_color(label_bg, lv_color_black(), 0);
// lv_obj_set_style_bg_opa(label_bg, LV_OPA_COVER, 0);
// lv_obj_set_style_border_width(label_bg, 0, 0);
// lv_obj_clear_flag(label_bg, LV_OBJ_FLAG_SCROLLABLE); // optional

//                 lv_obj_t *label = lv_label_create(label_bg);
//                 lv_label_set_text_fmt(label, "Serial Number : %s",SerialNumber);
//                 lv_obj_align_to(label, qr, LV_ALIGN_OUT_TOP_MID, 0, -15);  // below QR code

                // Optional: Set label style (white text on black background)
            //     static lv_style_t style;
            //     lv_style_init(&style);
            //     lv_style_set_text_color(&style, lv_color_white());
            //     lv_style_set_bg_color(&style, lv_color_black());
            //     lv_style_set_bg_opa(&style, LV_OPA_COVER);
            //     lv_style_set_pad_all(&style, 4);
            //     lv_obj_add_style(label, &style, 0);
            //     DisplayMode = ModeQR;
            // }    
        }
          if(Image2BDisplayed==ImageNoStock)
        {
              if (DisplayMode != ModeNoStock)
            {
                LV_IMG_DECLARE(NoStock);
                 ESP_LOGI(TAG,"Displaying NoStock Image");
                display_images(&NoStock);
                DisplayMode = ModeNoStock;
            }
        }
           if(Image2BDisplayed==StatusText)
        {
              if (DisplayMode != ModeStatusText)
            {
                 LV_IMG_DECLARE(BlankImage);
                ESP_LOGI(TAG,"Displaying BlankImage Image");
                display_images(&BlankImage);
                DisplayMode = ModeStatusText;
            }
           else {
                char payload[200] = {0};

                if (example_lvgl_lock(-1)) {

                    // Delete old label if it exists
                    if (lv_obj_is_valid(label)) {
                        lv_obj_del(label);
                        label = NULL;
                    }

                    // Create new label
                   if (label == NULL) {
        label = lv_label_create(lv_scr_act());
    }

                    if (!label) {
                        printf("Failed to create label!\n");
                        example_lvgl_unlock();
                        return;
                    }

                    // Set label text safely
                    snprintf(payload, sizeof(payload), "%s", TextStatus);
                    lv_label_set_text(label, payload);

                    // Initialize style2 once
                    if (!style2_initialized) {
                        lv_style_init(&style2);
                        lv_style_set_text_font(&style2, &lv_font_montserrat_28);
                        lv_style_set_text_color(&style2, lv_color_black());
                        style2_initialized = true;
                    }

                    // Apply style & align
                    lv_obj_add_style(label, &style2, 0);
                    lv_obj_align_to(label, img, LV_ALIGN_CENTER, 0, 0);

                    example_lvgl_unlock();
                }
            }

            
            
        }
        if(Image2BDisplayed>0)
        {
        Image2BDisplayed=0;
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void app_main(void)
{
    //Initialize NVS
    //esp_log_level_set("*", ESP_LOG_NONE);
    // set totals to 0
    char payload[200];
    VendingMode = VendingMobiVend;
    MQTTRequired = 1;
    for (int i = 0 ; i < 7 ; i++)
    {
        Totals[i] = 0;
        CashTotals[i] = 0;
    }   
    esp_log_level_set(TAG, ESP_LOG_DEBUG);
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    ESP_LOGI(TAG, "================================");
    ESP_LOGI(TAG, "*FW:%s#",FWVersion);
    ESP_LOGI(TAG, "================================");
    utils_nvs_init();
    status_leds_init();
    console_uart_init();
    uart_write_string(FWVersion);
    read_mac_address();
    xTaskCreate(display_image_task, "display_image_task", 8192, NULL, 7, NULL);
    xTaskCreate(tcpip_client_task, "tcpip_client_task", 8192, NULL, 7, NULL);
    load_settings_nvs();
    ESP_LOGI(TAG, "*Starting ICH#");
    ICH_init();
    ESP_LOGI(TAG, "*Starting S2P#");
    s2p_init();
    Out4094(0x00);; // set all outputs inactive
    TFT_main();
    DisplayBootingUp();
    displayStripes();
    showLogo();
    
    sprintf(payload,"*PID,%s#",SerialNumber);
    uart_write_string_ln(payload);
    uart_write_string_ln("*ARD+ESP#");
    
    // for (int i = 0 ; i < 3 ; i++)
    // {
        //     led_set_level(LEDR, 1);
        //     led_set_level(LEDG, 0);
    //     vTaskDelay(500/portTICK_PERIOD_MS);   
    //     led_set_level(LEDR, 0);
    //     led_set_level(LEDG, 1);
    //     vTaskDelay(500/portTICK_PERIOD_MS);   
    //     led_set_level(LEDR, 0);
    //     led_set_level(LEDG, 0);
    //     vTaskDelay(500/portTICK_PERIOD_MS);   
    // }
    
    ESP_LOGI(TAG, "*Starting WiFi#");
    SetCINHO();
    wifi_init_sta();
    // server_main();
    ESP_LOGI(TAG, "*Testing RGB #");
    TestRGB();
    
    xTaskCreate(sendHBT, "sendHBT", 4096, NULL, 6, NULL);
    // xTaskCreate(BlinkLED, "BlinkLED", 2048, NULL, 6, NULL);
   
    // xTaskCreate(TestCoin, "TestCoin", 2048, NULL, 6, NULL);
  

    // for (;;) 
    // xTaskCreate(memory_check, "display task", 1024 * 4, NULL, 10, NULL);

     while(1) {
        // lv_timer_handler();
            
        vTaskDelay(pdMS_TO_TICKS(5000));
    }

    // {   
//        lv_timer_handler();
        // vTaskDelay(pdMS_TO_TICKS(10));  // 30 msec delay
        // size_t free_heap = heap_caps_get_free_size(MALLOC_CAP_8BIT);
        // size_t free_internal_heap = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);
        // logic added on 251224
        // display No HBT For X minutes once every minute
        // and restart if no HBT for Y minutes
        // removed on 301224 as advised by Neeraj Ji
        // ServerHBTTimeOut++;
        // if ( ((ServerHBTTimeOut % 600) == 40) && (ServerHBTTimeOut > 620))
        // {
        //     sprintf(payload,"*No HBT For %d Minutes",ServerHBTTimeOut/600);
        //     uart_write_string_ln(payload);
        // }
        // if (ServerHBTTimeOut > HBTTIMEBEFORERESTART)
        // {
        //     RestartDevice();
        // }
        // 1 min = 60 sec, 30 min  = 1800 seconds = 18000 X 100 msec ticks  
        // 35 mins = 2100 seconds = 21000 X 100 msecs ticks
    // }
}
