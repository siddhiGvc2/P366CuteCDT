
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
#include "lv_png.h"  // Include the PNG decoder header
#include "lv_conf.h"
#include "esp_timer.h"
#include "rom/ets_sys.h"
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_err.h"
#include "esp_spiffs.h"
#include "externVars.h"
#include "calls.h"
static const char *TAG = "COLORS";

lv_obj_t *time_label;  // Global time label referenc
lv_obj_t *color_label[4];  // For 4 CDTColor labels
lv_obj_t *stripe_colors1[4];  // For 4 CDTColor labels
lv_obj_t *stripe_colors2[4];  // For 4 CDTColor labels
lv_obj_t *stripe_colors3[4];  // For 4 CDTColor labels
lv_obj_t *stripe_colors4[4];  // For 4 CDTColor labels

lv_obj_t **stripe_groups[4] = {stripe_colors1, stripe_colors2, stripe_colors3, stripe_colors4};
lv_color_t colors[5];


void init_colors() {
    colors[0] = lv_color_white();
    colors[1] = lv_color_make(0, 0, 255); //red
    colors[2] =  lv_color_make(0, 152, 255); //orange
    colors[3] =  lv_color_make(0, 128, 0);//green
    colors[4] =  lv_color_make(0, 128, 0);//green
    colors[5] =  lv_color_black();
    
}

void decrement_CDTColor() {
    // Create an array of pointers for easy mapping
   

    for (int i = 0; i < 4; i++) {
        // ✅ When CDTimeInput[i] is 0, display the mode
        if (color_label[i] != NULL) {
            if (strstr(CDTColor[i], "R") != NULL) {
                lv_obj_set_style_text_color(color_label[i], colors[1], LV_PART_MAIN);
            } else if (strstr(CDTColor[i], "A") != NULL) {
                lv_obj_set_style_text_color(color_label[i], colors[2], LV_PART_MAIN);
            } else if (strstr(CDTColor[i], "G") != NULL) {
                lv_obj_set_style_text_color(color_label[i], colors[3], LV_PART_MAIN);
            } else {
                lv_obj_set_style_text_color(color_label[i], lv_color_black(), LV_PART_MAIN);
            }
        }
        if (color_label[i] != NULL && CDTimeInput[i] == 0) {
            lv_label_set_text_fmt(color_label[i], "%s", Command);
        }        
        // ✅ If label is valid and time not zero, update the label with time
        else if (color_label[i] != NULL && CDTimeInput[i] != 0) {
            lv_label_set_text_fmt(color_label[i], "%d", CDTime[i]);
        }

        // ✅ Check if CDTColor is not empty and apply color accordingly
          // ✅ Color assignment based on CDTColor
        
          if (CDTColor[i][0] != '\0') {
            if (strstr(CDTColor[i], "R") != NULL) {
                lv_obj_set_style_bg_color(stripe_groups[i][0], colors[1], LV_PART_MAIN); // For R at i == 1
            }
            if (strstr(CDTColor[i], "A") != NULL) {
                lv_obj_set_style_bg_color(stripe_groups[i][1], colors[2], LV_PART_MAIN); // For A at i == 2
            }
            if (strstr(CDTColor[i], "G") != NULL) {
                // ✅ For G at i == 3 & i == 4
                lv_obj_set_style_bg_color(stripe_groups[i][2], colors[3], LV_PART_MAIN);
                lv_obj_set_style_bg_color(stripe_groups[i][3], colors[3], LV_PART_MAIN);
            }
        }


        // ✅ Decrement CDTime if greater than 0
        if (CDTime[i] > 0) {
            CDTime[i]--;
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
    //ESP_LOGI(TAG,"Time is - %02d:%02d:%02d",Hours,Mins,Secs);
    for (int i = 0; i < 4; i++){
        for (int j = 0; j < 4; j++) {
            if (stripe_groups[i][j] != NULL) {
                lv_obj_set_style_bg_color(stripe_groups[i][j], colors[0], LV_PART_MAIN); // White color
            }
        }
    }
    decrement_CDTColor(); 

   
        if (time_label != NULL) {
            lv_label_set_text_fmt(time_label, "%02d:%02d:%02d",Hours, Mins, Secs);
        }
      
}


// ✅ In your displayStripes() or app_main() function:



void displayStripes(void) {
    if(img)
    {
    lv_obj_del(img); // Delete the image if it exists
    img = NULL;
    }
    init_colors();

    lv_obj_t * parent = lv_scr_act();
    lv_obj_set_style_bg_color(parent, lv_color_white(), LV_PART_MAIN); // Set white background
    lv_obj_set_scrollbar_mode(parent, LV_SCROLLBAR_MODE_OFF);
    static lv_style_t style_label;
   
    const char *color_names[] = {"WHITE", "RED", "ORANGE", "GREEN", "GREEN"};
   

    int y_offset = 55;
    int spacing=10;
    int stripe_height = 50; // Adjust stripe height as needed
    int stripe_width = 50; // Adjust stripe height as needed


    lv_obj_t *strip = lv_obj_create(parent);
    lv_obj_set_size(strip, lv_pct(100), stripe_height);
    lv_obj_align(strip, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_style_bg_color(strip, colors[0], LV_PART_MAIN);
    
    time_label = lv_label_create(strip);
    lv_obj_set_scrollbar_mode(time_label, LV_SCROLLBAR_MODE_OFF);
    lv_label_set_text_fmt(time_label, "%d:%d:%d", Hours, Mins, Secs);
    lv_obj_set_style_text_color(time_label, lv_color_black(), LV_PART_MAIN);
    lv_obj_align(time_label, LV_ALIGN_CENTER, 0, 0); 
    lv_style_init(&style_label);
    lv_style_set_text_font(&style_label, &lv_font_montserrat_22);
    lv_obj_add_style(time_label, &style_label, LV_PART_MAIN);
    
    // Loop to create 5 stripes
    for (int i = 0; i < 5; i++) {
        lv_obj_t *stripe = lv_obj_create(parent); // Create new stripe each iteration
        lv_obj_set_size(stripe, stripe_width, stripe_height);
        lv_obj_align(stripe, LV_ALIGN_TOP_LEFT, 0 + spacing, y_offset);
        // lv_obj_set_style_bg_color(stripe, colors[i % 5], LV_PART_MAIN); // Default color
    
        if (i == 0) {
            // Label for first stripe
            color_label[0] = lv_label_create(stripe);
            lv_obj_set_scrollbar_mode(color_label[0], LV_SCROLLBAR_MODE_OFF);
            lv_label_set_text_fmt(color_label[0], "%d", CDTime[0]);

            if (strstr(CDTColor[0], "R") != NULL && i == 1) {
                lv_obj_set_style_text_color(color_label[0], colors[1], LV_PART_MAIN);
            } else if (strstr(CDTColor[0], "A") != NULL && i == 2) {
                lv_obj_set_style_text_color(color_label[0], colors[2], LV_PART_MAIN);
            } else if (strstr(CDTColor[0], "G") != NULL && i == 3) {
                lv_obj_set_style_text_color(color_label[0], colors[3], LV_PART_MAIN);
            }
            else{
                lv_obj_set_style_text_color(color_label[0], lv_color_black(), LV_PART_MAIN);
            }
         
           
            lv_obj_align(color_label[0], LV_ALIGN_CENTER, 0, 0);
            lv_style_init(&style_label);
            lv_style_set_text_font(&style_label, &lv_font_montserrat_18);
            lv_obj_add_style(color_label[0], &style_label, LV_PART_MAIN);
            // lv_obj_add_style(color_label[0], &style_label, LV_PART_MAIN);
        } else if (i > 0) {
            stripe_colors1[i - 1] = stripe;
            if (stripe_colors1[i - 1] != NULL) {
            lv_obj_set_style_border_color(stripe_colors1[i - 1],colors[i], LV_PART_MAIN);  // Red border
            lv_obj_set_style_border_width( stripe_colors1[i - 1], 2, LV_PART_MAIN);  // Border width of 2 pixels
            
            if (strstr(CDTColor[0], "R") != NULL && i == 1) {
                lv_obj_set_style_bg_color(stripe_colors1[0], colors[1], LV_PART_MAIN);
            } else if (strstr(CDTColor[0], "A") != NULL && i == 2) {
                lv_obj_set_style_bg_color(stripe_colors1[1], colors[2], LV_PART_MAIN);
            } else if (strstr(CDTColor[0], "G") != NULL && i == 3) {
                lv_obj_set_style_bg_color(stripe_colors1[2], colors[3], LV_PART_MAIN);
                
            }
            else if (strstr(CDTColor[0], "G") != NULL && i == 4) {
                lv_obj_set_style_bg_color(stripe_colors1[3], colors[3], LV_PART_MAIN);
                
            }
           }
        }
    
        y_offset += stripe_height; // Move to next stripe position with spacing
    }
    y_offset=55;
    for (int i = 0; i < 5; i++) {
        // Create a colored stripe
        lv_obj_t *stripe = lv_obj_create(parent);
        lv_obj_set_size(stripe, stripe_width, stripe_height);
        lv_obj_align(stripe, LV_ALIGN_TOP_LEFT, 55 + spacing, y_offset); // Fixed spacing logic
    
        if (i == 0) {
            // Create label for the first stripe
            color_label[1] = lv_label_create(stripe);
             lv_obj_set_scrollbar_mode(color_label[1], LV_SCROLLBAR_MODE_OFF);
            lv_label_set_text_fmt(color_label[1], "%d", CDTime[1]);
            if (strstr(CDTColor[1], "R") != NULL && i == 1) {
                lv_obj_set_style_text_color(color_label[1], colors[1], LV_PART_MAIN);
            } else if (strstr(CDTColor[1], "A") != NULL && i == 2) {
                lv_obj_set_style_text_color(color_label[1], colors[2], LV_PART_MAIN);
            } else if (strstr(CDTColor[1], "G") != NULL && i == 3) {
                lv_obj_set_style_text_color(color_label[1], colors[3], LV_PART_MAIN);
            }
            else{
                lv_obj_set_style_text_color(color_label[1], lv_color_black(), LV_PART_MAIN);
            }
         
            lv_obj_align(color_label[1], LV_ALIGN_CENTER, 0, 0);
            lv_style_init(&style_label);
            lv_style_set_text_font(&style_label, &lv_font_montserrat_18);
            lv_obj_add_style(color_label[1], &style_label, LV_PART_MAIN);
        } else if (i > 0){
            // For subsequent stripes
            stripe_colors2[i - 1] = stripe;
            
            if (stripe_colors2[i - 1] != NULL) {
            lv_obj_set_style_border_color(stripe_colors2[i - 1],colors[i], LV_PART_MAIN);  // Red border
            lv_obj_set_style_border_width( stripe_colors2[i - 1], 2, LV_PART_MAIN);  // Border width of 2 pixels
            

            // Set colors based on CDTColor[1]
            if (strstr(CDTColor[1], "R") != NULL && i == 1) {
                lv_obj_set_style_bg_color(stripe_colors2[0], colors[1], LV_PART_MAIN);
            } else if (strstr(CDTColor[1], "A") != NULL && i == 2) {
                lv_obj_set_style_bg_color(stripe_colors2[1], colors[2], LV_PART_MAIN);
            } else if (strstr(CDTColor[1], "G") != NULL && i == 3) {
                lv_obj_set_style_bg_color(stripe_colors2[2], colors[3], LV_PART_MAIN);
                
            }
            else if (strstr(CDTColor[1], "G") != NULL && i == 4) {
                lv_obj_set_style_bg_color(stripe_colors2[3], colors[3], LV_PART_MAIN);
                
            }
           }
        }
    
        y_offset += stripe_height; // Move to next stripe position with spacing
    }
    y_offset=55;
    for (int i = 0; i < 5; i++) {
        // Create a colored stripe
        lv_obj_t *stripe = lv_obj_create(parent);
        lv_obj_set_size(stripe, stripe_width, stripe_height);
        lv_obj_align(stripe, LV_ALIGN_TOP_LEFT, 110 + spacing, y_offset); // Fixed spacing logic
    
        if (i == 0) {
            // Create label for the first stripe
            color_label[2] = lv_label_create(stripe);
             lv_obj_set_scrollbar_mode(color_label[2], LV_SCROLLBAR_MODE_OFF);
            lv_label_set_text_fmt(color_label[2], "%d", CDTime[2]);
            if (strstr(CDTColor[2], "R") != NULL && i == 1) {
                lv_obj_set_style_text_color(color_label[2], colors[1], LV_PART_MAIN);
            } else if (strstr(CDTColor[2], "A") != NULL && i == 2) {
                lv_obj_set_style_text_color(color_label[2], colors[2], LV_PART_MAIN);
            } else if (strstr(CDTColor[2], "G") != NULL && i == 3) {
                lv_obj_set_style_text_color(color_label[2], colors[3], LV_PART_MAIN);
            }
            else{
                lv_obj_set_style_text_color(color_label[2], lv_color_black(), LV_PART_MAIN);
            }
         
            lv_obj_align(color_label[2], LV_ALIGN_CENTER, 0, 0);
            lv_style_init(&style_label);
            lv_style_set_text_font(&style_label, &lv_font_montserrat_18);
            lv_obj_add_style(color_label[2], &style_label, LV_PART_MAIN);
            // lv_obj_add_style(color_label[2], &style_label, LV_PART_MAIN);
        } else if (i > 0){
            // For subsequent stripes
            stripe_colors3[i - 1] = stripe;
            if (stripe_colors3[i - 1] != NULL) {
            lv_obj_set_style_border_color(stripe_colors3[i - 1],colors[i], LV_PART_MAIN);  // Red border
            lv_obj_set_style_border_width( stripe_colors3[i - 1], 2, LV_PART_MAIN);  // Border width of 2 pixels
            
            if (strstr(CDTColor[2], "R") != NULL && i == 1) {
                lv_obj_set_style_bg_color(stripe_colors3[0], colors[1], LV_PART_MAIN);
            } else if (strstr(CDTColor[2], "A") != NULL && i == 2) {
                lv_obj_set_style_bg_color(stripe_colors3[1], colors[2], LV_PART_MAIN);
            } else if (strstr(CDTColor[2], "G") != NULL && i == 3) {
                lv_obj_set_style_bg_color(stripe_colors3[2], colors[3], LV_PART_MAIN);
               
            }
            else if (strstr(CDTColor[2], "G") != NULL && i == 4) {
                lv_obj_set_style_bg_color(stripe_colors3[3], colors[3], LV_PART_MAIN);
                
            }
           

          }
        }
    
        y_offset += stripe_height; // Move to next stripe position with spacing
    }
    y_offset=55;
    for (int i = 0; i < 5; i++) {
        // Create a colored stripe
        lv_obj_t *stripe = lv_obj_create(parent);
        lv_obj_set_size(stripe, stripe_width, stripe_height);
        lv_obj_align(stripe, LV_ALIGN_TOP_LEFT, 165 + spacing, y_offset); // Fixed alignment
    
        if (i == 0) {
            // Create label for the first stripe
            color_label[3] = lv_label_create(stripe);
             lv_obj_set_scrollbar_mode(color_label[3], LV_SCROLLBAR_MODE_OFF);
            lv_label_set_text_fmt(color_label[3], "%d", CDTime[3]);
            if (strstr(CDTColor[3], "R") != NULL && i == 1) {
                lv_obj_set_style_text_color(color_label[3], colors[1], LV_PART_MAIN);
            } else if (strstr(CDTColor[3], "A") != NULL && i == 2) {
                lv_obj_set_style_text_color(color_label[3], colors[2], LV_PART_MAIN);
            } else if (strstr(CDTColor[3], "G") != NULL && i == 3) {
                lv_obj_set_style_text_color(color_label[3], colors[3], LV_PART_MAIN);
            }
            else{
                lv_obj_set_style_text_color(color_label[3], lv_color_black(), LV_PART_MAIN);
            }
         
            lv_obj_align(color_label[3], LV_ALIGN_CENTER, 0, 0);
            lv_style_init(&style_label);
            lv_style_set_text_font(&style_label, &lv_font_montserrat_18);
            lv_obj_add_style(color_label[3], &style_label, LV_PART_MAIN);
            // lv_obj_add_style(color_label[3], &style_label, LV_PART_MAIN);
        } else if (i > 0){
            // For subsequent stripes
            stripe_colors4[i - 1] = stripe;
            if (stripe_colors4[i - 1] != NULL) {
            lv_obj_set_style_border_color(stripe_colors4[i - 1],colors[i], LV_PART_MAIN);  // Red border
            lv_obj_set_style_border_width( stripe_colors4[i - 1], 2, LV_PART_MAIN);  // Border width of 2 pixels
            
            // Set colors based on CDTColor[3]
            if (strstr(CDTColor[3], "R") != NULL && i == 1) {
                lv_obj_set_style_bg_color(stripe_colors4[0], colors[1], LV_PART_MAIN);
            } else if (strstr(CDTColor[3], "A") != NULL && i == 2) {
                lv_obj_set_style_bg_color(stripe_colors4[1], colors[2], LV_PART_MAIN);
            } else if (strstr(CDTColor[3], "G") != NULL && i == 3) {
                lv_obj_set_style_bg_color(stripe_colors4[2], colors[3], LV_PART_MAIN);
               
            }
            else if (strstr(CDTColor[3], "G") != NULL && i == 4) {
                lv_obj_set_style_bg_color(stripe_colors4[3], colors[3], LV_PART_MAIN);
                
            }
           }
        }
    
        y_offset += stripe_height ; // Move to next stripe position with spacing
    }
    if (TimerSet == 0)
    {
        lv_timer_create(update_time_label, 1000, NULL);  // 1000ms = 1s
        TimerSet = 1;        
    }
    // showLogo();
    // lv_timer_create(update_time_label, 1000, NULL);
}