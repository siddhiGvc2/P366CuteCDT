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
#include "lvgl.h"
#include "externVars.h"
#include "calls.h"




static const char *TAG = "MAIN";

lv_obj_t *time_label;  // Global time label referenc
lv_obj_t *color_label[4];  // For 4 CDTColor labels



void decrement_CDTColor() {
    for (int i = 0; i < 4; i++) {
        int value;
        sscanf(CDTColor[i] + 1, "%d", &value);  // Extract number
        if (value > 0) {
            value--;
          
            snprintf(CDTColor[i], sizeof(CDTColor[i]), "%c%02d", CDTColor[i][0], value % 100);
            printf("Updated CDTColor[%d]: %s\n", i, CDTColor[i]);
        }

        if (color_label[i] != NULL) {
            lv_label_set_text_fmt(color_label[i], "%s", CDTColor[i]);
        }
        
       
    }
}



// ⏰ Timer callback function
static void update_time_label(lv_timer_t *timer) {
    Secs++;
    if (Secs >= 60) {
        Secs = 0;
        Mins++;
        if (Mins >= 60) {
            Mins = 0;
            Hours++;
            if (Hours >= 24) {
                Hours = 0;
            }
        }
    }
  
    decrement_CDTColor(); 

   
        if (time_label != NULL) {
            lv_label_set_text_fmt(time_label, "Mode - %s, %02d:%02d:%02d", Command, Hours, Mins, Secs);
        }
      
}


// ✅ In your displayStripes() or app_main() function:



void displayStripes(void) {
  
    lv_obj_t * parent = lv_scr_act();
    lv_obj_set_style_bg_color(parent, lv_color_white(), LV_PART_MAIN); // Set white background

    // Define colors and positions
    lv_color_t colors[] = {
        lv_color_white(),
        lv_color_make(0, 0, 255),
        lv_color_make(0, 152, 255),
        lv_color_make(0, 128, 0),
        // lv_color_make(0, 255, 0), // Green
        
    };
    const char *color_names[] = {"WHITE", "RED", "ORANGE", "GREEN", "GREEN"};
    
    int y_offset = 0;
    int stripe_height = 50; // Adjust stripe height as needed

    for (int i = 0; i < 5; i++) {
        // Create a colored stripe
        lv_obj_t * stripe = lv_obj_create(parent);
        lv_obj_set_size(stripe, lv_pct(100), stripe_height);
        lv_obj_align(stripe, LV_ALIGN_TOP_LEFT, 0, y_offset);
     
        if(i==0)
        {
            time_label = lv_label_create(stripe);
            lv_label_set_text_fmt(time_label,"Mode- %s, %d:%d:%d",Command,Hours,Mins,Secs);
            lv_obj_set_style_text_color(time_label, lv_color_black(), LV_PART_MAIN);
            lv_obj_set_style_bg_color(stripe,colors[0], LV_PART_MAIN);
            lv_obj_align(time_label, LV_ALIGN_CENTER, 0, 0); // Center text within the stripe
        }

        if(i>0)
        {
        if(strstr(CDTColor[i-1],"R")!=NULL)
        {
            lv_obj_set_style_bg_color(stripe, colors[1], LV_PART_MAIN);
        }
        else if(strstr(CDTColor[i-1],"A")!=NULL)
        {
            lv_obj_set_style_bg_color(stripe, colors[2], LV_PART_MAIN);
        }
        else if(strstr(CDTColor[i-1],"G")!=NULL)
        {
            lv_obj_set_style_bg_color(stripe, colors[3], LV_PART_MAIN);
        }
        else{
            lv_obj_set_style_bg_color(stripe, lv_color_black(), LV_PART_MAIN);
        }
        color_label[i - 1] = lv_label_create(stripe);  // Create label and assign to array
        lv_label_set_text_fmt(color_label[i - 1], "%s", CDTColor[i - 1]);
        lv_obj_set_style_text_color(color_label[i - 1], lv_color_white(), LV_PART_MAIN);
        lv_obj_align(color_label[i - 1], LV_ALIGN_CENTER, 0, 0); // Center text within the stripe
        
        }
      
     
        // Create label for the color name
       
      
        // lv_obj_set_style_text_color(label, lv_color_white(), LV_PART_MAIN); // Default to white text

    

      
        y_offset += stripe_height; // Move to next stripe position
    }
    lv_timer_create(update_time_label, 1000, NULL);  // 1000ms = 1s
    // lv_timer_create(update_time_label, 1000, NULL);
}