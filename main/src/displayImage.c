#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_mac.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "driver/uart.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/sockets.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"
#include "esp_netif.h"
#include "esp_smartconfig.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "lvgl.h"
#include "lv_conf.h"
#include "esp_timer.h"
#include "rom/ets_sys.h"
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_err.h"
#include "esp_spiffs.h"
#include "externVars.h"
#include "calls.h"
#include "gvcLogo.c"

#include "QRcode.c"

static const char *TAG = "LVGL";



void display_images(const lv_img_dsc_t *ig) {
    // Delete previous objects safely
    if (example_lvgl_lock(-1)) {
        if (lv_obj_is_valid(img)) {
            lv_obj_del(img);
            img = NULL;
        }
        if (lv_obj_is_valid(label)) {
            lv_obj_del(label);
            label = NULL;
        }
        example_lvgl_unlock();
    }

    // Create new objects
    if (example_lvgl_lock(-1)) {
        img = lv_img_create(lv_scr_act());
        label = lv_label_create(lv_scr_act());
        example_lvgl_unlock();
    }

    if (!lv_obj_is_valid(img)) {
        printf("Failed to create image!\n");
        return;
    }

    // Set image
    if (example_lvgl_lock(-1)) {
        lv_img_set_src(img, ig);
        lv_obj_align(img, LV_ALIGN_TOP_MID, 0, 0);
        example_lvgl_unlock();
    }

    // Label + style logic
    if (example_lvgl_lock(-1)) {
        char payload[200] = {0};

        if (Image2BDisplayed == 5) {
            snprintf(payload, sizeof(payload), "ItemCode-%s\nPrice-%s", itemCode, rawPrice);
            lv_label_set_text(label, payload);

            if (!style1_initialized) {
                lv_style_init(&style1);
                lv_style_set_text_font(&style1, &lv_font_montserrat_22);
                lv_style_set_text_color(&style1, lv_color_black());
                style1_initialized = true;
            }
            lv_obj_add_style(label, &style1, 0);
            lv_obj_align_to(label, img, LV_ALIGN_BOTTOM_MID, 0, -100);

        } else if (Image2BDisplayed == 9) {
            snprintf(payload, sizeof(payload), "%s", TextStatus);
            lv_label_set_text(label, payload);

            if (!style2_initialized) {
                lv_style_init(&style2);
                lv_style_set_text_font(&style2, &lv_font_montserrat_28);
                lv_style_set_text_color(&style2, lv_color_black());
                style2_initialized = true;
            }
            lv_obj_add_style(label, &style2, 0);
            lv_obj_align_to(label, img, LV_ALIGN_CENTER, 0, 0);

        } else if (Image2BDisplayed == 1) {
            lv_label_set_text(label, "Mobivend 140525");

            if (!style3_initialized) {
                lv_style_init(&style3);
                lv_style_set_text_font(&style3, &lv_font_montserrat_22);
                lv_style_set_text_color(&style3, lv_color_black());
                style3_initialized = true;
            }
            lv_obj_add_style(label, &style3, 0);
            lv_obj_align_to(label, img, LV_ALIGN_BOTTOM_MID, 0, -10);
        }

        example_lvgl_unlock();
    }
}




// void display_images(const lv_img_dsc_t *ig)
// {
//     // 1️⃣ Delete previous image safely
//     if (lv_obj_is_valid(img)) {
//         lv_obj_del_async(img);
//         img = NULL;
//     }

//     // 2️⃣ Create image
//     img = lv_img_create(lv_scr_act());
//     if (!lv_obj_is_valid(img)) {
//         printf("Failed to create image!\n");
//         return;
//     }
//     lv_img_set_src(img, ig);
//     lv_obj_align(img, LV_ALIGN_TOP_MID, 0, 0);

//     // 3️⃣ Delete previous label if it exists
//     if (lv_obj_is_valid(label)) {
//         lv_obj_del_async(label);
//         label = NULL;
//     }

//     // 4️⃣ Create new label based on Image2BDisplayed
//     label = lv_label_create(lv_scr_act());
//     static lv_style_t style;
//     lv_style_init(&style);
//     lv_style_set_text_color(&style, lv_color_black());

//     if (Image2BDisplayed == 5) {
//         char payload[200];
//         sprintf(payload, "ItemCode-%s\nPrice-%s", itemCode, rawPrice);
//         lv_label_set_text(label, payload);
//         lv_style_set_text_font(&style, &lv_font_montserrat_22);
//         lv_obj_add_style(label, &style, 0);
//         lv_obj_align_to(label, img, LV_ALIGN_BOTTOM_MID, 0, -100);
//     }
//     else if (Image2BDisplayed == 9) {
//         lv_label_set_text(label, TextStatus);
//         lv_style_set_text_font(&style, &lv_font_montserrat_28);
//         lv_obj_add_style(label, &style, 0);
//         lv_obj_align_to(label, img, LV_ALIGN_CENTER, 0, 0);
//     }
//     else if (Image2BDisplayed == 1) {
//         lv_label_set_text(label, "Mobivend 140525");
//         lv_style_set_text_font(&style, &lv_font_montserrat_22);
//         lv_obj_add_style(label, &style, 0);
//         lv_obj_align_to(label, img, LV_ALIGN_BOTTOM_MID, 0, -10);
//     }
// }


static lv_obj_t * img6 = NULL;
void showLogo(void)
{
    LV_IMG_DECLARE(gvcLogo);
     if (example_lvgl_lock(-1)) {
    img6 = lv_img_create(lv_scr_act());
    lv_img_set_src(img6, &gvcLogo);
    lv_obj_align(img6, LV_ALIGN_TOP_MID, 0, 255);
      example_lvgl_unlock();
     }

}


void DisplayBootingUp(void)
{
    // Image2BDisplayed = ImageBootingUp;


}

void DisplayNoWiFi(void)
{

    // Image2BDisplayed = ImageNoWifi;

}

void DisplayCashReceived(void)
{
    // Image2BDisplayed = ImageCashReceived;

}

void DisplaySelectItem(void)
{
    // Image2BDisplayed = ImageSelectItem;
   
}

void DisplayItemVend(void)
{
    // Image2BDisplayed = ImageItemVend;
    
}


//added on 120525

void DisplayCashlessDevice(void)
{
    // Image2BDisplayed = ImageCashlessDevice;
    
}

void DisplayNoStock(void)
{
    // Image2BDisplayed = ImageNoStock;
    //DisplayMode = ModeNoStock;
}

void DisplayStatusText(void)
{
    DisplayMode = ModeNone;
    // Image2BDisplayed = StatusText;
    
}

void DisplayCoinInserted(void)
{
    // Image2BDisplayed = ImageCoinInserted;

}

// changed on 140525
// all code shifted to display_image_task/main.c
 void dispayQR(void){
    if (DisplayMode != ModeQR)
    {
        // if INHvalue == 0 then dispaly No stock in place of QR Code
        if(INHInputValue==0)
        {
            DisplayNoStock();
        }
        else{
        // Image2BDisplayed = ImageQRCode;
        }
    }
//         if (lv_obj_is_valid(img))
//         {
//         lv_obj_del(img); // Delete the flame icon if it exists
//         img = NULL;
//         }
//         else{
//             img = NULL; 
//         }
//      DisplayMode = ModeQR;   
    
//      LV_IMG_DECLARE(QRcode);

//     //  display_images(&image);
//     if (example_lvgl_lock(-1)) {
//         img = lv_img_create(lv_scr_act());
//         example_lvgl_unlock();
//     }

//     if (example_lvgl_lock(-1)) {
//         lv_img_set_src(img, &QRcode);
//         example_lvgl_unlock();
//     }
//     lv_obj_align(img, LV_ALIGN_TOP_MID, 0, 0);
   
   

//     // Refresh the display to show the first image
  
//     // //delay

    
//     // // Display "Hello World" text
//     // lv_obj_t *label = lv_label_create(scr);
//     // lv_label_set_text(label, QrString);
//     // lv_obj_align(label, LV_ALIGN_CENTER, 0, 0); // Center the text

//     // // Create a QR code object
//     lv_obj_t *qr = lv_qrcode_create(img, QR_CODE_SIZE, lv_color_hex3(0x000), lv_color_hex3(0xFFF));
  

//     // // Set the QR code data
//    //const char *data = "https://www.google.com";
//     lv_qrcode_update(qr, QrString, strlen(QrString));
  

//     // // Center the QR code on the screen
//     lv_obj_align(qr, LV_ALIGN_CENTER, 0, 10);
   

//     // ***********Haresh
//     // while(1) {
//     //     lv_timer_handler();
//     //     vTaskDelay(pdMS_TO_TICKS(50));
//     // }
//     }
}


