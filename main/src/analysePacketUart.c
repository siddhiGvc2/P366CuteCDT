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

#include "externVars.h"
#include "calls.h"
#include "mbedtls/base64.h"



static const char *TAG = "UART";


void uart_write_number(uint8_t );
void uart_write_string(const char * );
void uart_write_string_ln(const char * );


void process_uart_packet(const char *);


void decode_base64_pkt(char *pkt) {
    int len = strlen(pkt);
    bool PacketOkay;

    // Ensure it starts with "FF" and ends with "FE"
   
     ESP_LOGI(TAG, "Started Decoding Packet" );

    // Extract Base64-encoded data (skip "FF" and trim "FE")
    int base64_len = len - 2;
    char base64_data[base64_len + 1];
    strncpy(base64_data, pkt + 1, base64_len);
    base64_data[base64_len] = '\0';  // Null-terminate
    // ESP_LOGI(TAG, "Packet Stripped %s",base64_data);
    // Remove any unwanted characters (spaces/newlines)
    //clean_base64(base64_data);
    
    // ESP_LOGI(TAG, "Base64 Encoded Data (cleaned): %s", base64_data);

    // Decode Base64
    uint8_t decoded_data[256];  // Adjust buffer size as needed
    size_t output_len = 0;
    int ret = mbedtls_base64_decode(decoded_data, sizeof(decoded_data), &output_len, 
                                    (const unsigned char*)base64_data, strlen(base64_data));

    if (ret == 0) {
        // Print decoded data in hex format
        // ESP_LOGI(TAG, "Decoded Hex Data:");
        for (size_t i = 0; i < output_len; i++) {
            // printf("%02X ", decoded_data[i]);
        }
        printf("\n");
    } else {
        ESP_LOGI(TAG, "Base64 decode failed! Error code: %d", ret);
    }
    int x = decoded_data[0];
    int y = output_len - 3;
    if (x ==  y)
    {
        PacketOkay = 1;
        // ESP_LOGI(TAG, "Length Byte Matched Actual is %d , Decoded is %d" , y , x);
    }
    else
    {
        PacketOkay = 0;
        return;
        // ESP_LOGI(TAG, "Length Error Actual is %d , Decoded is %d" , y , x);
    }

    Hours = decoded_data[1];
    Mins = decoded_data[2];
    Secs = decoded_data[3];
     ESP_LOGI (TAG, "Time is %d:%d:%d", Hours,Mins,Secs);

    
    CDTime[0] = decoded_data[7];
    CDTime[1] = decoded_data[8];
    CDTime[2] = decoded_data[9];
    CDTime[3] = decoded_data[10];

    // added on 22-02-025 for  // when 0 show mode(Fixed,Manual, VA etc..)
    CDTimeInput[0]=decoded_data[7];
    CDTimeInput[1]=decoded_data[8];
    CDTimeInput[2]=decoded_data[9];
    CDTimeInput[3]=decoded_data[10];
    
  
    if (decoded_data[29] >= 9)
        decoded_data[29] = 9;
    strcpy(Command,CommandTable[decoded_data[29]]);
  
    strcpy(CDTColor[0],CDTColorTable[(decoded_data[23] & 0xe0) >> 5]);
    strcpy(CDTColor[1],CDTColorTable[(decoded_data[23] & 0x1C) >> 2]);
    strcpy(CDTColor[2],CDTColorTable[((decoded_data[23] & 0x03)<< 1) + ((decoded_data[24] & 0x80) >> 7)]) ;
    strcpy(CDTColor[3],CDTColorTable[(decoded_data[24] & 0x70) >> 4]);
    ESP_LOGI (TAG, "CDT Details are %s - %d%s:%d%s:%d%s:%d%s", Command,CDTime[0],CDTColor[0],CDTime[1],CDTColor[1],CDTime[2],CDTColor[2],CDTime[3],CDTColor[3]);
    char payload[200];
    sprintf(payload,"*TL,%s,%s,%d%s,%d%s,%d%s,%d%s#",SerialNumber,Command,CDTime[0],CDTColor[0],CDTime[1],CDTColor[1],CDTime[2],CDTColor[2],CDTime[3],CDTColor[3]);
    send(sock, payload, strlen(payload), 0);
    if(MQTTRequired)
    {
    mqtt_publish_msg(payload);
    }
}


//added on 120525
void UartDataReceivedShowQRCode (void)
{
    UartPacketReceived = 1;
}

void process_uart_packet(const char *pkt){
    rx_event_pending = 1;
    char buf[200];
    char buffer[1400]; 
    char payload[240]; 
    // sprintf(buffer,"*MAC:%s:%s#", MAC_ADDRESS_ESP,SerialNumber);
    // uart_write_string_ln(buffer);
    // sprintf(payload, "*HBT,%s,%s#", MAC_ADDRESS_ESP,SerialNumber);
    // uart_write_string_ln(payload);
    //uart_write_string_ln(pkt);
    ESP_LOGI(TAG,"%s",pkt);


    if(strstr(pkt, "AmountReceived"))
    {
        ESP_LOGI(TAG,"AmountReceived pkt accepeted");
        mqtt_publish_msg(pkt);
        // uart_write_string_ln("*SUCCESS#"); // 140525 removed and sent by Server
    }
    else if(strncmp(pkt, "*CA?#", 5) == 0){
       
       sprintf(buffer,"*CA-OK,%s,%s,%d,%d#",CAuserName,CAdateTime,pulseWitdh,SignalPolarity);

       if (UartDebugInfoRequired)
             uart_write_string_ln(buffer);
        tx_event_pending = 1;
    }
    else if(strncmp(pkt, "*CC#", 4) == 0){
         for (int i = 0 ; i < 7 ; i++)
        {
            Totals[i] = 0;
            CashTotals[i] = 0;
        } 
        utils_nvs_set_int(NVS_CASH1_KEY, CashTotals[0]);
        utils_nvs_set_int(NVS_CASH2_KEY, CashTotals[1]);
        utils_nvs_set_int(NVS_CASH3_KEY, CashTotals[2]);
        utils_nvs_set_int(NVS_CASH4_KEY, CashTotals[3]);
        utils_nvs_set_int(NVS_CASH5_KEY, CashTotals[4]);
        utils_nvs_set_int(NVS_CASH6_KEY, CashTotals[5]);
        utils_nvs_set_int(NVS_CASH7_KEY, CashTotals[6]);
       
        sprintf(buffer, "*CC-OK#");
      
        if (UartDebugInfoRequired)
        uart_write_string_ln(buffer);
        tx_event_pending = 1;
    }
    
      else if(strncmp(pkt, "*SN:", 4) == 0){
      
       sscanf(pkt, "*SN:%[^#]#",buf);
        strcpy(SerialNumber, buf);
        strcpy(SNuserName, "LOCAL");
        strcpy(SNdateTime, "00/00/00");
        utils_nvs_set_str(NVS_SERIAL_NUMBER, SerialNumber);
        utils_nvs_set_str(NVS_SN_USERNAME,SNuserName);
        utils_nvs_set_str(NVS_SN_DATETIME,SNdateTime);
        
      
        if (UartDebugInfoRequired)
        uart_write_string_ln("*SN-OK#");
        tx_event_pending = 1;
        
    }
     else if(strncmp(pkt, "*PT:", 4) == 0){
      
       sscanf(pkt, "*PT:%[^#]#",PassThruValue);
      
        strcpy(PTuserName, "LOCAL");
        strcpy(PTdateTime, "00/00/00");
        utils_nvs_set_str(NVS_PASS_THRU, PassThruValue);
        utils_nvs_set_str(NVS_PT_USERNAME,PTuserName);
        utils_nvs_set_str(NVS_PT_DATETIME,PTdateTime);
        
        if (UartDebugInfoRequired)
    
        uart_write_string_ln("*PT-OK#");
        tx_event_pending = 1;
        
    }
       else if(strncmp(pkt, "*PT?#", 5) == 0){
      
        sprintf(buffer, "*PT,%s,%s,%s#",PTuserName,PTdateTime,PassThruValue); //actual when in production
        if (UartDebugInfoRequired)
    
        uart_write_string_ln(buffer);
      
      
        tx_event_pending = 1;
        
    }
     else if(strncmp(pkt, "*SN?#", 5) == 0){
      
        sprintf(buffer, "*SN,%s,%s,%s#",SNuserName,SNdateTime,SerialNumber); //actual when in production
        if (UartDebugInfoRequired)
        uart_write_string_ln(buffer);
      
      
        tx_event_pending = 1;
        
    }
    else if(strncmp(pkt, "*CA:", 4) == 0){
        sscanf(pkt, "*CA:%d:%d#",&numValue,&polarity);
        strcpy(CAuserName,"LOCAL");
        strcpy(CAdateTime,"00/00/00");
        
        sprintf(buffer,"*CA-OK,%d,%d#",numValue,polarity);
        utils_nvs_set_str(NVS_CA_USERNAME, CAuserName);
        utils_nvs_set_str(NVS_CA_DATETIME, CAdateTime);
        if (UartDebugInfoRequired)
       uart_write_string_ln(buffer);
        tx_event_pending = 1;
        
    }
   
     else if(strncmp(pkt, "*RST#", 5) == 0){
    
        sprintf(buffer, "*RST-OK#");
        uart_write_string_ln(buffer);
        uart_write_string_ln("*Resetting device#");
        RestartDevice();
    
      
    }
     else if(strncmp(pkt, "*D:",3) == 0){
        sscanf(pkt, "*D:%[^:#]#",UniqueTimeStamp);
        utils_nvs_set_str(NVS_UNIX_TS,UniqueTimeStamp);
        sprintf(buffer,"*D-OK,%s#",UniqueTimeStamp); 
        if (UartDebugInfoRequired)
          uart_write_string_ln(buffer);
    }
    else if(strncmp(pkt, "*D?#",4) == 0){
        
        sprintf(buffer, "*D-OK,%s#",UniqueTimeStamp); 
        if (UartDebugInfoRequired)
           uart_write_string_ln(buffer);
    }            
    else if(strncmp(pkt, "*V:", 3) == 0){
        if(edges==0)
        {
           sscanf(pkt, "*V:%[^:]:%d:%d#",TID,&pin,&pulses);
            // if (INHInputValue == INHIBITLevel)
            // {
            //     ESP_LOGI(TAG, "*UNIT DISABLED#");
            //     char buffer[100]; 
            //     sprintf(buffer, "*VEND DISABLED#");
            //     uart_write_string_ln(buffer);
                
            // }
            //  else if (TID != LastTID)
            if (memcmp(TID, LastTID, 100) != 0)
            {
                edges = pulses*2;  // doubled edges
                // strcpy(WIFI_PASS_2, buf);
                // utils_nvs_set_str(NVS_PASS_2_KEY, WIFI_PASS_2);
                ESP_LOGI(TAG, "*V-OK,%s,%d,%d#",TID,pin,pulses);
                 
                sprintf(buffer, "*V-OK,%s,%d,%d#", TID,pin,pulses); //actual when in production
                if (UartDebugInfoRequired)
                uart_write_string_ln(buffer);
                vTaskDelay(1000/portTICK_PERIOD_MS);
                sprintf(buffer, "*T-OK,%s,%d,%d#",TID,pin,pulses); //actual when in production
                ESP_LOGI(TAG, "*T-OK,%s,%d,%d#",TID,pin,pulses);
                if (UartDebugInfoRequired)
                uart_write_string_ln(buffer);
                tx_event_pending = 1;
                Totals[pin-1] += pulses;
                strcpy(LastTID,TID);
                utils_nvs_set_str(NVS_LAST_TID,LastTID);
            }
            else
            {
                ESP_LOGI(TAG, "Duplicate TID");
           
                
                sprintf(buffer,  "*DUP TID#");
                if (UartDebugInfoRequired)
                uart_write_string_ln(buffer);
            }  
      
      
        }
    
      
    }
     else if(strncmp(pkt, "*QR:",4) == 0){
            char tempBuf[500];
            sscanf(pkt, "*QR:%[^#]#",tempBuf);
            strcpy(QrString,tempBuf);
            sprintf(buffer, "*QR-OK,%s#",QrString);
            utils_nvs_set_str(NVS_QR_STRING,QrString);
            DisplayMode=ModeNone;
            dispayQR();
            if (UartDebugInfoRequired){
            uart_write_string_ln(payload);
            }
        }
        else if(strncmp(pkt, "*QR?#",5) == 0){
            
            sprintf(buffer, "*QR-OK,%s#",QrString); 
            if (UartDebugInfoRequired){
            uart_write_string_ln(buffer);
            }
        }
         else if(strncmp(pkt, "*STATUS?#",9) == 0){
           
           
            if(fotaStatus==1)
            {
              sprintf(buffer, "*FOTA#");
              if (UartDebugInfoRequired)
              uart_write_string_ln(buffer);   
            }
            else if(serverStatus==0)
            {
             sprintf(buffer, "*NOSERVER#");
             if (UartDebugInfoRequired)
             uart_write_string_ln(buffer); 
            }
            else if(serverStatus==1){
              sprintf(buffer, "*QR:%s#",QrString);
              if (UartDebugInfoRequired) 
              uart_write_string_ln(buffer);
            }
            
            
        }

        
      else if(strncmp(pkt, "*FW?#", 5) == 0){
         ESP_LOGI(TAG, "*%s#",FWVersion);
         if (UartDebugInfoRequired)
         {
         uart_write_string_ln(FWVersion);
         }
      
        tx_event_pending = 1;
    }
    else if(strncmp(pkt, "*SS:", 4) == 0){
        sscanf(pkt, "*SS:%[^#]#",buf);
        //uart_write_string_ln(buf);
        strcpy(WIFI_SSID_1, buf);
        utils_nvs_set_str(NVS_SSID_1_KEY, WIFI_SSID_1);
        if (UartDebugInfoRequired)
           uart_write_string_ln("*SS-OK#");
        tx_event_pending = 1;
    }else if(strncmp(pkt, "*SS1:", 5) == 0){
        sscanf(pkt, "*SS1:%[^#]#", buf);
        strcpy(WIFI_SSID_2, buf);
        utils_nvs_set_str(NVS_SSID_2_KEY, WIFI_SSID_2);
        if (UartDebugInfoRequired)
            uart_write_string_ln("*SS1-OK#");
        tx_event_pending = 1;
    }else if(strncmp(pkt, "*SS2:", 5) == 0){
        sscanf(pkt, "*SS2:%[^#]#", buf);
        strcpy(WIFI_SSID_3, buf);
        utils_nvs_set_str(NVS_SSID_3_KEY, WIFI_SSID_3);
        if (UartDebugInfoRequired)
            uart_write_string_ln("*SS2-OK#");
        tx_event_pending = 1;
    }else if(strncmp(pkt, "*PW:", 4) == 0){
        sscanf(pkt, "*PW:%[^#]#", buf);
        strcpy(WIFI_PASS_1, buf);
        utils_nvs_set_str(NVS_PASS_1_KEY, WIFI_PASS_1);
        if (UartDebugInfoRequired)
           uart_write_string_ln("*PW-OK#");
        tx_event_pending = 1;
    }else if(strncmp(pkt, "*PW1:", 5) == 0){
        sscanf(pkt, "*PW1:%[^#]#", buf);
        strcpy(WIFI_PASS_2, buf);
        utils_nvs_set_str(NVS_PASS_2_KEY, WIFI_PASS_2);
        if (UartDebugInfoRequired)
           uart_write_string_ln("*PW1-OK#");
        tx_event_pending = 1;
    }else if(strncmp(pkt, "*PW2:", 5) == 0){
        sscanf(pkt, "*PW2:%[^#]#", buf);
        strcpy(WIFI_PASS_3, buf);
        utils_nvs_set_str(NVS_PASS_3_KEY, WIFI_PASS_3);
        if (UartDebugInfoRequired)
           uart_write_string_ln("*PW2-OK#");
        tx_event_pending = 1;
    }else if(strncmp(pkt, "*URL:", 5) == 0){
        sscanf(pkt, "*URL:%[^#]#", buf);
        strcpy(FOTA_URL, buf);
        strcpy(URLuserName,"LOCAL");
        strcpy(URLdateTime,"00/00/00");
        utils_nvs_set_str(NVS_OTA_URL_KEY, FOTA_URL);
        utils_nvs_set_str(NVS_URL_USERNAME, URLuserName);
        utils_nvs_set_str(NVS_URL_DATETIME, URLdateTime);
        if (UartDebugInfoRequired)
           uart_write_string_ln("*URL-OK#");
        tx_event_pending = 1;
    }else if(strncmp(pkt, "*FOTA#", 6) == 0){
        uart_write_string_ln("*FOTA-OK#");
        tx_event_pending = 1;
        http_fota();
    }else if(strncmp(pkt, "*URL?#", 6) == 0){
       
       sprintf(buffer,"*%s,%s,%s#",URLuserName,URLdateTime,FOTA_URL);
       if (UartDebugInfoRequired)
         uart_write_string_ln(buffer);
    tx_event_pending = 1;
    
    }
//    WIFI_SSID_1,WIFI_SSID_2,WIFI_SSID_3
    else if(strncmp(pkt, "*SSID?#", 7) == 0){

        uart_write_string("SSID Current/1/2/3 are - ");
        uart_write_number(WiFiNumber);
        uart_write_string(" , ");
        uart_write_string(WIFI_SSID_1);
        uart_write_string(" , ");
        uart_write_string(WIFI_SSID_2);
        uart_write_string(" , ");
        uart_write_string_ln(WIFI_SSID_3);

        tx_event_pending = 1;
    
    }
     else if(strncmp(pkt, "*TESTON#",8) == 0)
        {
            HardwareTestMode = 1;    
            pin = 0;    
            ESP_LOGI(TAG, "*Hardware Test Started#");
            if (UartDebugInfoRequired){
            uart_write_string_ln("*Hardware Test Started#");
            }
            // clear TC also
            for (int i = 0 ; i < 7 ; i++)
            {
                CashTotals[i] = 0;
            } 
    }        
        else if(strncmp(pkt, "*TESTOFF#",9) == 0)
        {
            HardwareTestMode = 0;    
            pin = 0;    
            ESP_LOGI(TAG, "*Hardware Test Stopped#");
            if (UartDebugInfoRequired){
            uart_write_string_ln("*Hardware Test Stopped#");
            }
            RestartDevice();

    }    
    
    else if(strncmp(pkt, "*TC?#", 5) == 0){
      
        sprintf(buffer, "*TC,%s,%d,%d,%d,%d,%d,%d,%d#", 
             UniqueTimeStamp,CashTotals[0],CashTotals[1],CashTotals[2],CashTotals[3],CashTotals[4],CashTotals[5],CashTotals[6]);

        if (UartDebugInfoRequired)
        {
        uart_write_string_ln(buffer);
        }
        tx_event_pending = 1;
    }
    else if(strncmp(pkt, "*TV?#", 5) == 0){
       
        sprintf(buffer, "*TV,%d,%d,%d,%d,%d,%d,%d#", 
            Totals[0], Totals[1], Totals[2], Totals[3], Totals[4], Totals[5], Totals[6]);

        if (UartDebugInfoRequired)
            uart_write_string_ln(buffer);
        tx_event_pending = 1;
    
    }
    else if(strncmp(pkt, "*SIP:", 5) == 0){
        sscanf(pkt, "*SIP:%d#",&SipNumber);
        strcpy(SIPuserName,"LOCAL");
        strcpy(SIPdateTime,"00/00/00");

          if ( (SipNumber == 0) || (SipNumber > MAXSIPNUMBER) )  
            {  
                //sprintf(payload, "*SIP-Error#");
                ESP_LOGI(TAG,"*SIP-ERROR#");
                if (UartDebugInfoRequired)
                   uart_write_string_ln("*SIP-ERROR#");

            }else 
            {
                sprintf(payload, "*SIP-OK,%s,%s#",SIPuserName,SIPdateTime);                                                   
                uart_write_string_ln(payload);
                utils_nvs_set_int(NVS_SIP_NUMBER, SipNumber);
                utils_nvs_set_str(NVS_SIP_USERNAME, SIPuserName);
                utils_nvs_set_str(NVS_SIP_DATETIME, SIPdateTime);
                ESP_LOGI(TAG,"*SIP-OK,%s,%s#",SIPuserName,SIPdateTime);
            } 

        // sprintf(buf, "%s", server_ip_addr);
        
        // utils_nvs_set_str(NVS_SERVER_IP_KEY, buf);
        // utils_nvs_set_int(NVS_SERVER_PORT_KEY, sp_port);

        // utils_nvs_set_str(NVS_SIP_USERNAME, SIPuserName);
        // utils_nvs_set_str(NVS_SIP_DATETIME, SIPdateTime);
        // uart_write_string_ln("*SIP-OK#");
        tx_event_pending = 1;
    } else if(strncmp(pkt, "*SIP?#", 6) == 0){
        
       sprintf(buffer,"*SIP,%s,%s,%s,%d#",SIPuserName,SIPdateTime,server_ip_addr,
                                        sp_port );

   
        if (UartDebugInfoRequired)
           uart_write_string_ln(buffer);
        tx_event_pending = 1;
    
    }
    else if (strncmp(pkt, "*ERASE:", 7) == 0){
         sscanf(pkt, "*ERASE:%[^:#]",ErasedSerialNumber);
         if (strcmp(ErasedSerialNumber, SerialNumber) != 0) {
            // send (payload,"*Erase:Serial Not Matched Command:%s Actual:%s#",tempBuf,SerialNumber);
            const char* errorMsg = "Erase:Serial Not Matched";
            // send(sock, errorMsg, strlen(errorMsg), 0);
            uart_write_string_ln(errorMsg);
        }
        else{
       
        strcpy(ERASEuserName,"LOCAL");
        strcpy(ERASEdateTime,"00/00/00");
        utils_nvs_set_str(NVS_ERASE_USERNAME, ERASEuserName);
        utils_nvs_set_str(NVS_ERASE_DATETIME, ERASEdateTime);
        utils_nvs_set_str(NVS_ERASED_SERIAL_NUMBER, ErasedSerialNumber);
        utils_nvs_erase_all();
        utils_nvs_set_str(NVS_SERIAL_NUMBER, ErasedSerialNumber);
        if (UartDebugInfoRequired)
           uart_write_string_ln("*ERASE:OK#");
        }

    }
     else if (strncmp(pkt, "*ERASE?", 7) == 0){
   
     sprintf(buffer,"*ERASE,%s,%s,%s#",ERASEuserName,ERASEdateTime,ErasedSerialNumber); 
     if (UartDebugInfoRequired)
        uart_write_string_ln(payload);
        
    }
    else if(strncmp(pkt, "*RESTART#", 9) == 0){
        if (UartDebugInfoRequired)
            uart_write_string("*RESTART:OK#");
        if (UartDebugInfoRequired)
            uart_write_string_ln("*Resetting device#");
        RestartDevice();
    }
    else if(strncmp(pkt, "*LS?#", 5) == 0){
       
        sprintf(payload,"LED State is %d",led_state);
        if (UartDebugInfoRequired)
            uart_write_string_ln(payload);
    }
    else if(strncmp(pkt, "*MS?#", 5) == 0){
        if (UartDebugInfoRequired)
            uart_write_string_ln(pkt);
        sprintf(payload,"*MS%ld#",MQTT_CONNEECTED);
        uart_write_string_ln(payload);
    }
    else if(strncmp(pkt, "*WS?#", 5) == 0){
        if (UartDebugInfoRequired)
           uart_write_string_ln(pkt);
        sprintf(payload,"*WS%d#",connected_to_wifi);
        UartDataReceivedShowQRCode();
        uart_write_string_ln(payload);
        if (connected_to_wifi==false)
            ESP_LOGI(TAG,"********WS? = 0#############");
    }
    else if (strncmp(pkt, "*SELL,", 6) == 0) {
        ESP_LOGI("UART", "Received SELL command!");

        // Extract Price, RefId, and extra value (e.g., 0x20)
        char uartPrice[10], spring[10];
        if (sscanf(pkt, "*SELL,%19[^,],%19[^,#]#", uartPrice, spring) == 2) {
            ESP_LOGI("UART", "Price: %s, RefId: %s", price, refId);
            sprintf(payload,"*SELL-OK,%s,%s#", uartPrice, spring);
            if (UartDebugInfoRequired){
            uart_write_string_ln(payload);
            }
            // Process the SELL command as needed...
            if(IsMobivendApi)
            {
            char formatted_url[376];  // Adjust size if needed
            sprintf(formatted_url, 
             URL_CASHLESSVEND,
             SerialNumber,spring, price, refId);
            strcpy(API,"CashLessVend");
            start_http_get_task(formatted_url);
            }
        } else {
            ESP_LOGW("UART", "Invalid SELL command format!");
        }
    }
    else if (strncmp(pkt, "*VEND,", 6) == 0) {
        ESP_LOGI("UART", "Received VEND command!");

        // Extract Price, RefId, and extra value (e.g., 0x20)
        char uartPrice[10], spring[10];
        if (sscanf(pkt, "*VEND,%19[^,],%19[^,#]#", uartPrice, spring) == 2) {
            // ESP_LOGI("UART", "Price: %s, RefId: %s", price, refId);
            sprintf(payload,"*VEND-OK,%s,%s#", uartPrice, spring);
            if (UartDebugInfoRequired){
            uart_write_string_ln(payload);
            }
            // Process the SELL command as needed...
            if(IsMobivendApi)
            {
            char formatted_url[376];  // Adjust size if needed
            sprintf(formatted_url, 
            URL_CASHLESSSALE,
            SerialNumber,refId,price,spring);
            strcpy(API,"CashLessSale");
            start_http_get_task(formatted_url);
            }
        } else {
            ESP_LOGW("UART", "Invalid VEND command format!");
        }
    }

     // added on 070525
    else if (strncmp(pkt, "*TRXN,", 6) == 0) {
        ESP_LOGI("UART", "Received TRXN command!");
        if (UartDebugInfoRequired)
             uart_write_string_ln("Received TRXN command!");
        // Declare buffers for all 7 fields
      
    
        // Parse 7 fields
        int matched = sscanf(pkt, "*TRXN,%[^,],%[^,],%[^,],%[^,],%[^,],%[^,#]#", 
                             val1, val2, refId, val4, rawPrice, itemCode);
    
        if (matched == 6) {
            // Convert price from paise to rupees
            int priceInt = atoi(rawPrice);
            int p=priceInt/100;
    
            // Format spring (e.g., A01 → AX01)
            char spring[10];
            if (strlen(itemCode) >= 3) {
                snprintf(spring, sizeof(spring), "%cX%c%c", itemCode[0], itemCode[1], itemCode[2]);
            } else {
                ESP_LOGW("UART", "Invalid itemCode length");
                uart_write_string_ln("Invalid itemCode length");
                return;
            }
            // uart_write_string_ln("*SUCCESS#"); // 140525 removed and sent by Server
            DisplayItemVend();
            ESP_LOGI(TAG,"DISPLAYED ITEME VEND");
            if(MQTT_CONNEECTED && MQTTRequired)
            {
            mqtt_publish_msg(pkt);
            }

            // Send acknowledgment over UART
            char payload[204];
            // snprintf(payload, sizeof(payload), "*VEND-OK,%s,%s#", price, spring);
            // uart_write_string_ln(payload);
            if(IsMobivendApi)
            {
                sprintf(price,"%d", p);
            char formatted_url[476];  // Adjust size if needed
            snprintf(formatted_url, sizeof(formatted_url),
            URL_CASHLESSSALE,
            SerialNumber, refId, price , spring);  // price is int here
            strcpy(API,"CashLessSale");
            start_http_get_task(formatted_url);
            }

            vTaskDelay(10000/portTICK_PERIOD_MS);
            DisplayMode=ModeNone;
            dispayQR();
    
        } else {
            ESP_LOGW("UART", "Invalid TRXN format: expected 7 values.");
        }
    }

    // added on 080525
    else if(strncmp(pkt,"*REFUND,",8)==0)
    {
        // uart_write_string_ln("*SUCCESS#"); // 140525 removed and sent by Server
        mqtt_publish_msg(pkt);
    }
     // added on 080525
    else if(strncmp(pkt,"*SCANQR#",8)==0)
    {
        DisplayMode=ModeNone;
        dispayQR();
    }
     else if(strncmp(pkt,"*SENDID#",8)==0)
    {
       char response[6];   // 5 digits + null terminator
        int num = atoi(SerialNumber);   // Convert string to integer

        // Format into 5-digit, zero-padded number
        sprintf(response, "%05d", num);
        uart_write_string_ln(response);
    }
      else if(strncmp(pkt,"*REQUEST:",9)==0)
    {
        if (UartDebugInfoRequired)
            uart_write_string_ln("Display dummy QR Code");
        strcpy(QrString,"Waiting For QrCode");
        DisplayMode=ModeNone;
        dispayQR();
        if (UartDebugInfoRequired)
            uart_write_string_ln("Send Request to server");
        mqtt_publish_msg(pkt);
    }
    
    else if(strncmp(pkt,"*TSI#",5)==0)
    {
        DisplayStatusText();
    }

// added for decoding base 64
        else if((pkt[0])==0xff){
        int x = strlen(pkt);
         ESP_LOGI(TAG,"Packet Start Received");
        if (pkt[x-1] == 0xfe)    
        {
            ESP_LOGI(TAG,"Packet End Received");
            decode_base64_pkt(pkt);
        }
    }

  
    else{
        if (UartDebugInfoRequired)
             uart_write_string_ln(pkt);
     int l = strlen(pkt);
        char buff[l+1];
        /*   *---#  */

        if(extractSubstring(pkt, buff) == true){
            int l2 = strlen(buff);
            char b[l2+3];
            sprintf(b, "*%s#", buff);
            // tcp_ip_client_send_str(b);
            tx_event_pending = 1;
            // added in 120525
            UartDataReceivedShowQRCode();
         }
    }
}

void uart_write_number(uint8_t number){
    char str[5];
    if (number<10)
    {
         str[0] = number+'0';
         str[1] = 0x00;
    }   
    uart_write_bytes(EX_UART_NUM, str, 1);
}

void uart_write_string(const char * str){
    uart_write_bytes(EX_UART_NUM, str, strlen(str));
}

void uart_write_string_ln(const char * str){
    ESP_LOGI(TAG,"To UART : %s",str);
    uart_write_bytes(EX_UART_NUM, str, strlen(str));
    uart_write_string("\r\n");
}
