## 241225
  strcpy(CDTColor[3],CDTColorTable[(decoded_data[24] & 0x70) >> 4]);
    ESP_LOGI (TAG, "CDT Details are %s - %d%s:%d%s:%d%s:%d%s", Command,CDTime[0],CDTColor[0],CDTime[1],CDTColor[1],CDTime[2],CDTColor[2],CDTime[3],CDTColor[3]);
    char payload[200];
    sprintf(payload,"*TL,%s,%s,%d%s,%d%s,%d%s,%d%s#",SerialNumber,Command,CDTime[0],CDTColor[0],CDTime[1],CDTColor[1],CDTime[2],CDTColor[2],CDTime[3],CDTColor[3]);
    send(sock, payload, strlen(payload), 0);

    use there paratmeters to display on screen
    chnage time every second.
    this command send time once when things change. reduce time in code and update seconds
    
## 231225
-- P366-TFT_CA ported to CDT Manager
-- added base64 decoding and display values in analysePacketUart. 
-- next step is to add display

## 181025
-- BlankImage error solved .ESP-TOUCH tested- woking
-- in RestartDevice if edges and pulse stop delay is not zero, wait for 3 seconds and then reset
-- this avoid any issue of extra pulse due to noise
-- do not display ESP TOUCH DONE image
--RestartDevice removed after ESP-DONE

## 111025
-- changes made fo solve bug in ShowQrCode
-- *VEND command remove from mqttRoutines

## 101025
-- ShowQr Function changes made for resolev errors
-- displayImage function made changes for crash error of displayingImag

## 091025
-- showQrCode function added in main file for display QrCOde
-- Cash Received Displayed from V command in MQTT.

## 190925
-- hARd coding removed polarity = 0 removed 
-- set PULSEOO as per polarity
-- set initial level as SignalPolarity in hardware.c
-- set polarity when CA command is sent
-- SDA is also used as PULSEO but in reverse polarity

## 290825
remove MQTT for RP till we find solution
add *SENDID# command
reply with 5 digit serial number lik 00123
change serial number to numeric only and do not add any GVC-CA etc..

when cash inserted (*RP:1:1#) - show a screen of CASH ENTERED for X seconds and then back to last screen QR Code or NO STOCK or No WiFi


## 310725
1. Test FOTA
2. Start ESP Touch when switch pressed and then released at power on -workingf

3. Initiate ESP Touch from UART - SIdDDHI 10:51 - 11:00
*START_ESPTOUCH# command added in Uart to Start Esp Touch.-ABORT

4. Display Images/Text - REMOVE JUMPER , RUN ESP TOUCH APP, SENSING WIFI, ESP TOUCH DONE, RESETTING

## 300725
if CINHI is low, display OUT OF STOCK image
when it is high, display QR code again
Displayed No Stock Image on Poweron on also if INHInputValue==0

when there is no WiFi, make CINH0 as high
SetInho function added to Set INHO value

Reset Not Working.
Wifi Not connecting after restarting.

Three vending mode added
vendingMobivend = edges = cash* 100 so reduce by 100
vendingArcade = generate only one pulse

14:45 - on dashboard, add Coin_Acceptor_type in database as 0/1/2/....
if 0 any payment - 
if 1 any paymenyt - send command *V:TID:1:1# also send VEND command
if 2 any payment  - send command *V:TID:1:X# where x is amount
if 3 any paymeny - use fpr future use

Right now we hard code as 2 for P366 and 0 for mobivend 
option - do not add any database , send VEND as well as V command (CA Type 2)

## 290725
copied from KWIKPAT MOBIVEND TFT and made it working by making hardware related changes
git push

add PULSEI and PULSEO
set PULSO as 0
Pulse as per V command 

*V:TID:1:x#
gpio bug solved


## 050525
-- use static QR code
-- set QR code using *QR:QRstring# command
-- interface with Mobivend APIs
-- interface with GVC Vending Board (via UART)
-- set Uart baud rate to 9600
-- command to UART when getting payment *VEND,PAYTM,TID,Amount,GUID#
-- send *ARD+ESP# when device powered on before trying for wifi
-- send z after some delay
-- send *CONFIGOK# after some delay
-- send *OKNET# when wifi and mqtt connected
- when UART sends *TRXN,POSID,TID,SID,PAYTM,Price,SprialNumber# - this means this item has been vend, send API to mobivend server.
-- send Serial Number via *PID,xxxx# to UART
-- serial number is without NA-1507-
-- MS? & WS? command added in uart

#060525
-- MS? & WS? command added in uart
-- replay as MSx and WSx
-- send *WiFi,SSID# when wifi connected
-- send *WS1# when wifi connected and *WS0# when wifi disconnected
-- send *MS1# when MQTT connected and *MS0# when mqtt disconnected
-- send *Dddmmyyhhmmss# when wifi connected
-- remove unnecessary UART command
-- when *65121,PAYTM,AmountReceived#  received, reply with *SUCCESS# to UART
-- do not send this or any UART packet to TCP

## 070525
-- change QR code to thaat of Mobivend (done 12:30 - 12:45 )
-- ensure that QR code is changed when new value is set (pending)
-- removed ESP_LOGI (for MQTT HBT) (12:45 - 12:50)
-- stop calling mobivend APIs (done)
-- Send SUCCESS after TRXN received (done)
-- Send MS1 and WS1 and not MS,1 and WS,1
-- do not send any thing to UART for diagnostics and MQTT copy data (done)
-- add ITEMS VENDED screen on display whem TRXN receievd(done)  changed in /main/src/analysePacketUart.c
-- wait for 30 seconds and display QR code again(done)  changed in /main/src/analysePacketUart.c
-- send MQTT *TRXN,.. as it is #(done)  changed in /main/src/analysePacketUart.c
-- check when there is no WIFI or MQTT do not display QR code and display some message (done)

## 080525
-- when *SCANQR# received display QR code if not already on display(done)  changed in /main/src/analysePacketUart.c
-- Send SUCCESS after REFUND received(done)  changed in /main/src/analysePacketUart.c


## 120525

-- QR code displayed when WS? command recived from uart. /main/src/analysePacketUart.c
-- till then displayed wating for cashless device image. /main/src/wifi.c

## 130525
-- removed UART data of FW and Mac and IP Address
-- all display images logic changed. It is in single task DisplayImageTask / main.c


## 140525
-- display QR code when setting --(done) /main/src/mqttRoutines.c , displayimage.c, main.c
-- send SUCCESS from backend when you get *Amount Received". "TRXN" and "REFUND" and remove from esp-idf - analysepacketuart at 3 places
-- esp-idf will forward SUCCESS message to UART--(done) /main/src/mqttRoutines.c


-- display Item Code and Price is Item Vended (as well as remaining price if any) src/displayImage.c
-- refund
-- backend data save all info (vend, refund, transaction)

