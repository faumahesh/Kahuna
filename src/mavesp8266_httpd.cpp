/****************************************************************************
 *
 * Copyright (c) 2015, 2016 Gus Grubba. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

/**
 * @file mavesp8266_httpd.cpp
 * ESP8266 Wifi AP, MavLink UART/UDP Bridge
 *
 * @author Gus Grubba <mavlink@grubba.com>
 */

#if defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266WebServer.h>
#include <Updater.h>
#else
#include <WebServer.h>
#include <Update.h>
#endif
#include "mavesp8266.h"
#include "mavesp8266_httpd.h"
#include "mavesp8266_parameters.h"
#include "mavesp8266_gcs.h"
#include "mavesp8266_vehicle.h"

const char PROGMEM kTEXTPLAIN[] = "text/plain";
const char PROGMEM kTEXTHTML[] = "text/html";
const char PROGMEM kBADARG[] = "BAD ARGS";
const char PROGMEM kAPPJSON[] = "application/json";
const char PROGMEM kACCESSCTL[] = "Access-Control-Allow-Origin";
const char PROGMEM kDEFAULTBTN[] = "<button type=button onclick=\"window.location.href='/setdefaults'\">Reset to Defaults</button></div></div></body></html>";
const char PROGMEM kUPLOADFORM[] = "<div class=p_content><div class='formbox'><p>Upload new firmware</p><form method='POST' action='/upload' enctype='multipart/form-data'><input type='file' accept='.bin' name='update'><br></div><input type='submit' value='Update'></form></div></body></html>";
const char PROGMEM kUPLOADSUCCESS[] = "<div class=p_content><div class='formbox'><p>Firmware Update in progress, waiting for the Kahuna to come back online...</p></div><script>function checkServer(){fetch('/').then(response=>{if(response.ok){window.location.href='/';}else{throw new Error('Server not ready');}}).catch(()=>{setTimeout(checkServer,5000);});}setTimeout(checkServer,10000);</script><div/></body></html>";

const char PROGMEM kHEADER2[] = "<a href=javascript:void(0); class=icon onclick=changeTopnav()><div class=container><div class=bar1></div><div class=bar2></div><div class=bar3></div></div></a></div>";

const char PROGMEM kHEADER1_C1[] = "<!doctype html><meta content='text/html; charset=windows-1252'http-equiv=Content-Type><style>body { margin: 0; font-family: Arial, Helvetica, sans-serif; } .topnav { display: table; display: flex; overflow: hidden; background-color: #333; } .topnav a { float: left; color: #f2f2f2; text-align: center; padding: 14px 16px; text-decoration: none; font-size: 20px; order: 1; } .topnav a:hover { background-color: #ddd; color: black; } .topnav a.active { background-color: rgb(97, 32, 110); color: white; } .topnav .icon { display: none; } .formbox { margin-top: 20px; padding: 20px; border: 1px solid #ccc; border-radius: 5px; background-color: #f2f2f2; } input[type='file'] { -webkit-text-size-adjust: 300%; } #logo { display: block; float: left; height: 40px; } #title { display: block; float: left; margin-top: auto; margin-bottom: auto; padding: 0; font-size: 30px; color: #61206e; }";
const char PROGMEM kHEADER1_C2[] = " #logocontainer { display: flex; margin: 0; padding: 0; background-color: #f2f2f2; padding: 5px; } #FW { margin-top: auto; margin-bottom: 5px; margin-left: auto; margin-right: 5px; padding: 0; font-size: 10px; } input[type=text], select { font-size: 1em; width: 100%; padding: 12px; border: 1px solid #ccc; border-radius: 4px; box-sizing: border-box; resize: vertical; background-color: white; } label { padding: 12px 12px 12px 0; display: inline-block; } input[type=submit] { font-size: 1em; margin-top: 20px; background-color: #61206e; color: white; padding: 12px 20px; border: none; border-radius: 4px; cursor: pointer; float: left; } button { background-color: gray; margin-left: 20px; font-size: 1em; margin-top: 20px; color: white; padding: 12px 20px; border: none; border-radius: 4px; cursor: pointer; float: left; } .p_content { padding-left: 20px; padding-right: 20px; max-width: 650px; } .col-l { float: left; width: 35%; margin-top: 6px; } .col-r { float: left; width: 65%; margin-top: 6px; } .row:after { content: ''; display: table; clear: both; } table { table-layout: fixed; width: 100%; } ";
const char PROGMEM kHEADER1_C3[] = "@media screen and (max-device-width: 600px) { .topnav a:not(.active) { display: none; } .topnav a.active { order: 0; display: table-cell; width: 100%; text-align: left; } .topnav a.icon { display: block; margin-left: auto; margin-right: 0px; padding: none; } .topnav.responsive { position: relative; flex-direction: column; } .topnav.responsive .icon { display: table-cell; position: absolute; margin-left: 0px; right: 0; top: 0; padding-top: auto; padding-bottom: auto; } .topnav.responsive a { float: none; display: block; display: flex; text-align: left; } .topnav a { font-size: 200%; } body { font-size: 250%; } .bar1, .bar2, .bar3 { width: 60px; height: 10px; background-color: white; margin: 8px 0; } .container { display: inline-block; margin-top: auto; margin-bottom: auto; } .formbox { max-width: 100%; } input[type='radio'] { height: 35px; width: 35px; } #logo { height: 100px; } #title { font-size: 80px; } #FW { font-size: 25px; } .col-l, .col-r { width: 100%; margin-top: 0; } .p_content { max-width: 100%; } ";
const char PROGMEM kHEADER1_C4[] = "input[type='submit'] { width: 100%; margin: 20px 0px 0px; } button { width: 100%; margin: 20px 0px 20px; } td { -webkit-text-size-adjust: none; } .left { width: 70%; } .right { width: 30%; } }</style><script>function changeTopnav(){var a=document.getElementById('BRtopnav');'topnav'===a.className?a.className+=' responsive':a.className='topnav'}</script><title>BR Kahuna</title><div id=logocontainer><svg id=logo version=1.1 viewBox='0 0 230 100'xmlns=http://www.w3.org/2000/svg><g transform='translate(3.62 20.395)'><g transform='translate(-.62316 -4.3941)'><g transform='matrix(1.848 0 0 1.848 -9.7077 -3.7351)'fill=none stroke=#61206e><path d='m12.448 35.432 18.022-17.616'stroke-width=2 /><g transform='matrix(.26458 0 0 .26458 3.6432 -4.4745)'stroke-width=7.5591><path d='m33.279 150.83v-113.39h37.795s20.651-1.4003 32.601 15.978c11.95 17.378-2.2818 30.829-2.2818 30.829l-68.114 66.579'/><path d='m71.074 37.442 43.626-0.08317s20.651-1.4003 32.601 15.978-2.2818 30.829-2.2818 30.829l-28.347 27.142'/>";
const char PROGMEM kHEADER1_C5[] = "<path d='m33.279 150.83 122.28-0.08317-30.222-47.271'/></g><path d='m11.448 35.428 10.87 0.0039s10.549 0.22731 12.214-6.5893c1.6643-6.8166-5.7271-9.3846-5.7271-9.3846'stroke-width=2 stroke-miterlimit=3.1 /></g><text dominant-baseline=auto fill=#61206e font-family='&#39;Times New Roman&#39;'font-size=136.06px stop-color=#000000 style=font-feature-settings:normal;font-variant-alternates:normal;font-variant-caps:normal;font-variant-east-asian:normal;font-variant-ligatures:normal;font-variant-numeric:normal;font-variant-position:normal;font-variation-settings:normal;inline-size:0;line-height:1.25;shape-margin:0;shape-padding:0;text-decoration-color:#000;text-decoration-line:none;text-decoration-style:solid;text-indent:0;text-orientation:mixed;text-transform:none;white-space:normal transform='matrix(.26458 0 0 .26458 48.793 14.503)'x=125.12414 xml:space=preserve y=57.077274><tspan font-family='&#39;Space Mono&#39;'font-size=136.06px x=125.12414 y=57.077274 fill=#61206e>Beyond</tspan></text>";
const char PROGMEM kHEADER1_C6[] = "<text dominant-baseline=auto fill=#000000 font-family='&#39;Times New Roman&#39;'font-size=136.06px stop-color=#000000 style=font-feature-settings:normal;font-variant-alternates:normal;font-variant-caps:normal;font-variant-east-asian:normal;font-variant-ligatures:normal;font-variant-numeric:normal;font-variant-position:normal;font-variation-settings:normal;inline-size:0;line-height:1.25;shape-margin:0;shape-padding:0;text-decoration-color:#000;text-decoration-line:none;text-decoration-style:solid;text-indent:0;text-orientation:mixed;text-transform:none;white-space:normal transform='matrix(.26458 0 0 .26458 -120.67 50.186)'x=761.78986 xml:space=preserve y=48.768002><tspan font-family='&#39;Space Mono&#39;'font-size=136.06px x=761.78986 y=48.768002>Robotix</tspan></text></g></g></svg><h1 id=title>Kahuna</h1><p id=FW>Version: ";

const uint32_t PROGMEM baudRates[] = {9600, 19200, 38400, 57600, 111100, 115200, 230400, 256000, 460800, 500000, 921600};
const uint8_t PROGMEM numBaudRates = 11;
char buffer[2048];

const char *kBAUD = "baud";
const char *kPWD = "pwd";
const char *kSSID = "ssid";
const char *kPWDSTA = "pwdsta";
const char *kSSIDSTA = "ssidsta";
const char *kIPSTA = "ipsta";
const char *kGATESTA = "gatewaysta";
const char *kSUBSTA = "subnetsta";
const char *kCPORT = "cport";
const char *kHPORT = "hport";
const char *kCHANNEL = "channel";
const char *kDEBUG = "debug";
const char *kREBOOT = "reboot";
const char *kPOSITION = "position";
const char *kMODE = "mode";

const char *kFlashMaps[7] = {
    "512KB (256/256)",
    "256KB",
    "1MB (512/512)",
    "2MB (512/512)",
    "4MB (512/512)",
    "2MB (1024/1024)",
    "4MB (1024/1024)"};

static uint32_t flash = 0;
static char paramCRC[12] = {""};

#if defined(ARDUINO_ARCH_ESP8266)
ESP8266WebServer webServer(80);
#else
WebServer webServer(80);
#endif
MavESP8266Update *updateCB = NULL;
bool started = false;

// Send the header content to the web server for various pages
static void sendHeaderContent()
{
    webServer.setContentLength(CONTENT_LENGTH_UNKNOWN);
    webServer.send_P(200, kTEXTHTML, kHEADER1_C1);
    webServer.sendContent_P(kHEADER1_C2);
    webServer.sendContent_P(kHEADER1_C3);
    webServer.sendContent_P(kHEADER1_C4);
    webServer.sendContent_P(kHEADER1_C5);
    webServer.sendContent_P(kHEADER1_C6);

    const char *buildDateTime = __DATE__ " " __TIME__;
    snprintf(buffer, sizeof(buffer), "%u.%u.%u (Build: %s)</p></div><div class=topnav id=BRtopnav><a href=/>Setup</a><a href=/getstatus>Status</a><a href=/getparameters>Parameters</a><a href=/update>Firmware Update</a><a href=/reboot class=active>Reboot</a>", MAVESP8266_VERSION_MAJOR, MAVESP8266_VERSION_MINOR, MAVESP8266_VERSION_BUILD, buildDateTime);
    //snprintf(buffer, sizeof(buffer), "%u.%u.%u</p></div><div class=topnav id=BRtopnav><a href=/>Setup</a><a href=/getstatus>Status</a><a href=/getparameters>Parameters</a><a href=/update>Firmware Update</a><a href=/reboot class=active>Reboot</a>", MAVESP8266_VERSION_MAJOR, MAVESP8266_VERSION_MINOR, MAVESP8266_VERSION_BUILD);
    webServer.sendContent(buffer);
    webServer.sendContent_P(kHEADER2);
}


// String message;
//---------------------------------------------------------------------------------
void setNoCacheHeaders()
{
    webServer.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    webServer.sendHeader("Pragma", "no-cache");
    webServer.sendHeader("Expires", "-1");
}

//---------------------------------------------------------------------------------
void returnFail(String msg)
{
    webServer.send(500, FPSTR(kTEXTPLAIN), msg + "\r\n");
}

//---------------------------------------------------------------------------------
void respondOK()
{
    webServer.send(200, FPSTR(kTEXTPLAIN), "OK");
}

//---------------------------------------------------------------------------------
void handle_update()
{
    webServer.sendHeader("Connection", "close");
    webServer.sendHeader(FPSTR(kACCESSCTL), "*");
    sendHeaderContent();
    webServer.sendContent_P(kUPLOADFORM);
    webServer.sendContent("");
}

//---------------------------------------------------------------------------------
void handle_upload()
{
    webServer.sendHeader("Connection", "close");
    webServer.sendHeader(FPSTR(kACCESSCTL), "*");
    sendHeaderContent();
    webServer.sendContent_P(kUPLOADSUCCESS);

    webServer.sendContent("");

    if (updateCB)
    {
        updateCB->updateCompleted();
    }
    delay(5000);   // ensure the response is sent before rebooting
    ESP.restart(); // Restart the Kahuna
}

//---------------------------------------------------------------------------------
void handle_upload_status()
{
    bool success = true;
    if (!started)
    {
        started = true;
        if (updateCB)
        {
            updateCB->updateStarted();
        }
    }
    HTTPUpload &upload = webServer.upload();
    if (upload.status == UPLOAD_FILE_START)
    {
#ifdef DEBUG_SERIAL
        DEBUG_SERIAL.setDebugOutput(true);
#endif
#if defined(ARDUINO_ARCH_ESP8266)
        WiFiUDP::stopAll();
#else
        getWorld()->getGCS()->stopUdpForUpdate();
#endif
#ifdef DEBUG_SERIAL
        DEBUG_SERIAL.printf("Update: %s\n", upload.filename.c_str());
#endif
#if defined(ARDUINO_ARCH_ESP8266)
        uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
        if (!Update.begin(maxSketchSpace))
#else
        if (!Update.begin(UPDATE_SIZE_UNKNOWN))
#endif
        {
#ifdef DEBUG_SERIAL
            Update.printError(DEBUG_SERIAL);
#endif
            success = false;
        }
    }
    else if (upload.status == UPLOAD_FILE_WRITE)
    {
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize)
        {
#ifdef DEBUG_SERIAL
            Update.printError(DEBUG_SERIAL);
#endif
            success = false;
        }
    }
    else if (upload.status == UPLOAD_FILE_END)
    {
        if (Update.end(true))
        {
#ifdef DEBUG_SERIAL
            DEBUG_SERIAL.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
#endif
        }
        else
        {
#ifdef DEBUG_SERIAL
            Update.printError(DEBUG_SERIAL);
#endif
            success = false;
        }
#ifdef DEBUG_SERIAL
        DEBUG_SERIAL.setDebugOutput(false);
#endif
    }
    yield();
    if (!success)
    {
        if (updateCB)
        {
            updateCB->updateError();
        }
    }
}

//---------------------------------------------------------------------------------
void handle_getParameters() // This can be improved a lot
{
    setNoCacheHeaders();
    sendHeaderContent();
    webServer.sendContent("<div class=p_content><div class='formbox'><table><tr><td>Name</td><td>Value</td></tr>");

    for (int i = 0; i < MavESP8266Parameters::ID_COUNT; i++)
    {
        if (i == getWorld()->getParameters()->ID_FWVER)
        {
            snprintf(buffer, sizeof(buffer), "<tr><td>%s</td><td>%u.%u.%u</td></tr>", getWorld()->getParameters()->getAt(i)->id, MAVESP8266_VERSION_MAJOR, MAVESP8266_VERSION_MINOR, MAVESP8266_VERSION_BUILD);
            webServer.sendContent(buffer);
        }
        else if (i == getWorld()->getParameters()->ID_MODE)
        {
            snprintf(buffer, sizeof(buffer), "<tr><td>%s</td><td>%s</td></tr>", getWorld()->getParameters()->getAt(i)->id, getWorld()->getParameters()->getWifiMode() == KAHUNA_PARAM_WIFI_AP ? "AP" : "STA");
            webServer.sendContent(buffer);
        }
        else if (i == getWorld()->getParameters()->ID_IPADDRESS)
        {

            uint32_t local_ip = getWorld()->getParameters()->getLocalIPAddress();
            snprintf(buffer, sizeof(buffer), "<tr><td>%s</td><td>%d.%d.%d.%d</td></tr>", getWorld()->getParameters()->getAt(i)->id, (int)(local_ip & 0xFF), (int)(local_ip >> 8 & 0xFF), (int)(local_ip >> 16 & 0xFF), (int)(local_ip >> 24 & 0xFF));
            webServer.sendContent(buffer);
        }
        else if (i == getWorld()->getParameters()->ID_SSID1)
        {
            snprintf(buffer, sizeof(buffer), "<tr><td>%s</td><td>%s</td></tr>", getWorld()->getParameters()->getAt(i)->id, getWorld()->getParameters()->getWifiSsid());
            webServer.sendContent(buffer);
        }
        else if (i > getWorld()->getParameters()->ID_SSID1 && i <= getWorld()->getParameters()->ID_SSID4)
        {
        }
        else if (i == getWorld()->getParameters()->ID_PASS1)
        {
            snprintf(buffer, sizeof(buffer), "<tr><td>%s</td><td>%s</td></tr>", getWorld()->getParameters()->getAt(i)->id, getWorld()->getParameters()->getWifiPassword());
            webServer.sendContent(buffer);
        }
        else if (i > getWorld()->getParameters()->ID_PASS1 && i <= getWorld()->getParameters()->ID_PASS4)
        {
        }
        else if (i == getWorld()->getParameters()->ID_SSIDSTA1)
        {
            snprintf(buffer, sizeof(buffer), "<tr><td>%s</td><td>%s</td></tr>", getWorld()->getParameters()->getAt(i)->id, getWorld()->getParameters()->getWifiStaSsid());
            webServer.sendContent(buffer);
        }
        else if (i > getWorld()->getParameters()->ID_SSIDSTA1 && i <= getWorld()->getParameters()->ID_SSIDSTA4)
        {
        }
        else if (i == getWorld()->getParameters()->ID_PASSSTA1)
        {
            snprintf(buffer, sizeof(buffer), "<tr><td>%s</td><td>%s</td></tr>", getWorld()->getParameters()->getAt(i)->id, getWorld()->getParameters()->getWifiStaPassword());
            webServer.sendContent(buffer);
        }
        else if (i > getWorld()->getParameters()->ID_PASSSTA1 && i <= getWorld()->getParameters()->ID_PASSSTA4)
        {
        }
        else if (i == getWorld()->getParameters()->ID_IPSTA)
        {
            uint32_t wifi_sta_ip = getWorld()->getParameters()->getWifiStaIP();
            snprintf(buffer, sizeof(buffer), "<tr><td>%s</td><td>%d.%d.%d.%d</td></tr>", getWorld()->getParameters()->getAt(i)->id, (int)(wifi_sta_ip & 0xFF), (int)(wifi_sta_ip >> 8 & 0xFF), (int)(wifi_sta_ip >> 16 & 0xFF), (int)(wifi_sta_ip >> 24 & 0xFF));
            webServer.sendContent(buffer);
        }
        else if (i == getWorld()->getParameters()->ID_GATEWAYSTA)
        {
            uint32_t wifi_sta_gateway = getWorld()->getParameters()->getWifiStaGateway();
            snprintf(buffer, sizeof(buffer), "<tr><td>%s</td><td>%d.%d.%d.%d</td></tr>", getWorld()->getParameters()->getAt(i)->id, (int)(wifi_sta_gateway & 0xFF), (int)(wifi_sta_gateway >> 8 & 0xFF), (int)(wifi_sta_gateway >> 16 & 0xFF), (int)(wifi_sta_gateway >> 24 & 0xFF));
            webServer.sendContent(buffer);
        }
        else if (i == getWorld()->getParameters()->ID_SUBNETSTA)
        {
            uint32_t wifi_sta_subnet = getWorld()->getParameters()->getWifiStaSubnet();
            snprintf(buffer, sizeof(buffer), "<tr><td>%s</td><td>%d.%d.%d.%d</td></tr>", getWorld()->getParameters()->getAt(i)->id, (int)(wifi_sta_subnet & 0xFF), (int)(wifi_sta_subnet >> 8 & 0xFF), (int)(wifi_sta_subnet >> 16 & 0xFF), (int)(wifi_sta_subnet >> 24 & 0xFF));
            webServer.sendContent(buffer);
        }
        else // integer values
        {
            unsigned long val = 0;
            if (getWorld()->getParameters()->getAt(i)->type == MAV_PARAM_TYPE_UINT32)
                val = (unsigned long)*((uint32_t *)getWorld()->getParameters()->getAt(i)->value);
            else if (getWorld()->getParameters()->getAt(i)->type == MAV_PARAM_TYPE_UINT16)
                val = (unsigned long)*((uint16_t *)getWorld()->getParameters()->getAt(i)->value);
            else
                val = (unsigned long)*((int8_t *)getWorld()->getParameters()->getAt(i)->value);

            snprintf(buffer, sizeof(buffer), "<tr><td>%s</td><td>%lu</td></tr>", getWorld()->getParameters()->getAt(i)->id, val);
            webServer.sendContent(buffer);
        }
    }
    webServer.sendContent("</table></div></div></body></html>");
    webServer.sendContent("");
}

//---------------------------------------------------------------------------------
static void handle_setup()
{
    setNoCacheHeaders();
    sendHeaderContent();
    
    snprintf(buffer, sizeof(buffer),
             "<div class=p_content><form action=/setparameters method=post>"
             "<div class=formbox>"
             "<div class=row>"
             "<div class=col-l><label>WiFi Mode</label></div>"
             "<div class=col-r>"
             "<input value=0 name=mode type=radio%s><label for=0>Access Point</label> "
             "<input value=1 name=mode type=radio%s><label for=1>Station</label>"
             "</div>"
             "</div>"
             "<div class=row>"
             "<div class=col-l><label>Baud Rate</label></div>"
             "<div class=col-r><select name='baud'>",
             getWorld()->getParameters()->getWifiMode() == KAHUNA_PARAM_WIFI_AP ? " checked" : "",
             getWorld()->getParameters()->getWifiMode() == KAHUNA_PARAM_WIFI_STA ? " checked" : "");
    webServer.sendContent(buffer);

    // Build the baud rate dropdown options into the buffer (we loop through so we can select the current baud rate)
    size_t offset = 0;
    uint32_t currentBaud = getWorld()->getParameters()->getUartBaudRate();
    for (uint8_t i = 0; i < numBaudRates; i++)
    {
        uint32_t rate = pgm_read_dword(&baudRates[i]);
        offset += snprintf(buffer + offset, sizeof(buffer) - offset,
                           "<option value='%u'%s>%u</option>",
                           rate,
                           (rate == currentBaud) ? " selected" : "",
                           rate);
    }
    webServer.sendContent(buffer);

    snprintf(buffer, sizeof(buffer),
             "</select></div></div>"
             "<div class=row>"
             "<div class=col-l><label>Host Port</label></div>"
             "<div class=col-r><input value=%u name=hport  type=text></div>"
             "</div>"
             "<div class=row>"
             "<div class=col-l><label>Client Port</label></div>"
             "<div class=col-r><input value=%u name=cport  type=text></div>"
             "</div>"
             "</div>"
             "<div class=formbox>"
             "<div class=row>"
             "<div class=col-l><label>AP SSID</label></div>"
             "<div class=col-r><input value='%s' name=ssid type=text  maxlength='15' minlength='1' required></div>"
             "</div>"
             "<div class=row>"
             "<div class=col-l><label>AP Password (min len 8)</label></div>"
             "<div class=col-r><input value='%s' name=pwd type=text  maxlength='15' minlength='8' required></div>"
             "</div>"
             "<div class=row>"
             "<div class=col-l><label>WiFi Channel</label></div>"
             "<div class=col-r><select name='channel'>",
             getWorld()->getParameters()->getWifiUdpHport(),
             getWorld()->getParameters()->getWifiUdpCport(),
             getWorld()->getParameters()->getWifiSsid(),
             getWorld()->getParameters()->getWifiPassword());
    webServer.sendContent(buffer);

    // Build the WiFi channel dropdown options into the buffer (we loop through so we can select the current channel)
    offset = 0;
    uint8_t currentChannel = getWorld()->getParameters()->getWifiChannel();
    for (uint8_t channel = 1; channel <= 11; channel++)
    {
        offset += snprintf(buffer + offset, sizeof(buffer) - offset,
                           "<option value='%u'%s>%u</option>",
                           channel,
                           (channel == currentChannel) ? " selected" : "",
                           channel);
    }
    webServer.sendContent(buffer);

    snprintf(buffer, sizeof(buffer),
             "</select></div></div></div>"
             "<div class=formbox>"
             "<div class=row>"
             "<div class=col-l><label>Station SSID</label></div>"
             "<div class=col-r><input value='%s' name=ssidsta type=text  maxlength='15'></div>"
             "</div>"
             "<div class=row>"
             "<div class=col-l><label>Station Password</label></div>"
             "<div class=col-r><input value='%s' name=pwdsta type=text  maxlength='15'></div>"
             "</div>",
             getWorld()->getParameters()->getWifiStaSsid(),
             getWorld()->getParameters()->getWifiStaPassword());
    webServer.sendContent(buffer);

    IPAddress StaIP = getWorld()->getParameters()->getWifiStaIP();
    IPAddress StaGateway = getWorld()->getParameters()->getWifiStaGateway();
    IPAddress StaSubnet = getWorld()->getParameters()->getWifiStaSubnet();
    snprintf(buffer, sizeof(buffer),
             "<div class=row>"
             "<div class=col-l><label>Station IP</label></div>"
             "<div class=col-r><input value='%s' name=ipsta type=text></div>"
             "</div>"
             "<div class=row>"
             "<div class=col-l><label>Station Gateway</label></div>"
             "<div class=col-r><input value='%s' name=gatewaysta type=text></div>"
             "</div>"
             "<div class=row>"
             "<div class=col-l><label>Station Subnet</label></div>"
             "<div class=col-r><input value='%s' name=subnetsta type=text></div>"
             "</div></div>"
             "<div class=row><input value=Save type=submit></form>",
             StaIP.toString().c_str(),
             StaGateway.toString().c_str(),
             StaSubnet.toString().c_str());
    webServer.sendContent(buffer);

    webServer.sendContent_P(kDEFAULTBTN);
    webServer.sendContent("");
}

//---------------------------------------------------------------------------------
static void handle_getStatus()
{
    if (!flash)
        flash = ESP.getFreeSketchSpace();
    if (!paramCRC[0])
    {
        snprintf(paramCRC, sizeof(paramCRC), "%08X", getWorld()->getParameters()->paramHashCheck());
    }
    linkStatus *gcsStatus = getWorld()->getGCS()->getStatus();
    linkStatus *vehicleStatus = getWorld()->getVehicle()->getStatus();

    setNoCacheHeaders();
    sendHeaderContent();

#if defined(ARDUINO_ARCH_ESP8266)
    uint32_t flashChipSize = ESP.getFlashChipRealSize();
#else
    uint32_t flashChipSize = ESP.getFlashChipSize();
#endif
    unsigned gcsLostPct = gcsStatus->packets_received > 0
                              ? (unsigned)((gcsStatus->packets_lost * 100) / gcsStatus->packets_received)
                              : 0;
    unsigned vehLostPct = vehicleStatus->packets_received > 0
                              ? (unsigned)((vehicleStatus->packets_lost * 100) / vehicleStatus->packets_received)
                              : 0;
    snprintf(buffer, sizeof(buffer), "<div class=p_content><div class=formbox><div class=row><div class=col-l>Refresh Rate</div><div class=col-r><select id=refreshRate onchange=setStatusRefreshRate()><option value='1'>1 second<option value='2'>2 seconds<option value='5' selected>5 seconds<option value='10'>10 seconds<option value='15'>15 seconds</select></div></div><p>Comm Status<table><tr><td class=left>Packets Received from GCS<td class=right>%u<tr><td>Packets Sent to GCS<td>%u<tr><td>GCS Packets Lost<td>%u (%u%%)<tr><td>Packets Received from Vehicle<td>%u<tr><td>Packets Sent to Vehicle<td>%u<tr><td>Vehicle Packets Lost<td>%u (%u%%)<tr><td>Radio Messages<td>%u</table><p>System Status<table><tr><td class=left>Flash Size<td class=right>%u<tr><td>Flash Available<td>%u<tr><td>RAM Left<td>%u<tr><td>Parameters CRC<td>'%s'</table></div></div><script>function setStatusRefreshRate(){var s=document.getElementById('refreshRate');if(s){localStorage.setItem('statusRefreshRate',s.value);window.location.reload();}}(function(){var s=document.getElementById('refreshRate');if(!s)return;var rate=localStorage.getItem('statusRefreshRate')||'5';s.value=rate;setTimeout(function(){window.location.reload();},parseInt(rate,10)*1000);})();</script></body></html>", gcsStatus->packets_received, gcsStatus->packets_sent, gcsStatus->packets_lost, gcsLostPct, vehicleStatus->packets_received, vehicleStatus->packets_sent, vehicleStatus->packets_lost, vehLostPct, gcsStatus->radio_status_sent, flashChipSize, flash, ESP.getFreeHeap(), paramCRC);

    webServer.sendContent(buffer);

    webServer.sendContent("");
}

//---------------------------------------------------------------------------------
void handle_getJLog()
{
    uint32_t position = 0, len;
    if (webServer.hasArg(kPOSITION))
    {
        position = webServer.arg(kPOSITION).toInt();
    }
    String logText = getWorld()->getLogger()->getLog(&position, &len);
    char jStart[128];
    snprintf(jStart, 128, "{\"len\":%d, \"start\":%d, \"text\": \"", len, position);
    String payLoad = jStart;
    payLoad += logText;
    payLoad += "\"}";
    webServer.send(200, FPSTR(kAPPJSON), payLoad);
}

//---------------------------------------------------------------------------------
void handle_getJSysInfo()
{
    if (!flash)
        flash = ESP.getFreeSketchSpace();
    if (!paramCRC[0])
    {
        snprintf(paramCRC, sizeof(paramCRC), "%08X", getWorld()->getParameters()->paramHashCheck());
    }
#if defined(ARDUINO_ARCH_ESP8266)
    uint32_t fid = spi_flash_get_id();
    const char *flashSizeLabel = kFlashMaps[system_get_flash_size_map()];
#else
    uint64_t mac = ESP.getEfuseMac();
    uint32_t fid = (uint32_t)(mac ^ (mac >> 32));
    static char flashSizeBuf[32];
    snprintf(flashSizeBuf, sizeof(flashSizeBuf), "%u KB", (unsigned)(ESP.getFlashChipSize() / 1024));
    const char *flashSizeLabel = flashSizeBuf;
#endif
    char message[512];
    snprintf(message, 512,
             "{ "
             "\"size\": \"%s\", "
             "\"id\": \"0x%02lX 0x%04lX\", "
             "\"flashfree\": \"%u\", "
             "\"heapfree\": \"%u\", "
             "\"logsize\": \"%u\", "
             "\"paramcrc\": \"%s\""
             " }",
             flashSizeLabel,
             (long unsigned int)(fid & 0xff), (long unsigned int)((fid & 0xff00) | ((fid >> 16) & 0xff)),
             flash,
             ESP.getFreeHeap(),
             getWorld()->getLogger()->getPosition(),
             paramCRC);
    webServer.send(200, "application/json", message);
}

//---------------------------------------------------------------------------------
void handle_getJSysStatus()
{
    bool reset = false;
    if (webServer.hasArg("r"))
    {
        reset = webServer.arg("r").toInt() != 0;
    }
    linkStatus *gcsStatus = getWorld()->getGCS()->getStatus();
    linkStatus *vehicleStatus = getWorld()->getVehicle()->getStatus();
    if (reset)
    {
        memset(gcsStatus, 0, sizeof(linkStatus));
        memset(vehicleStatus, 0, sizeof(linkStatus));
    }
    char message[512];
    snprintf(message, 512,
             "{ "
             "\"gpackets\": \"%u\", "
             "\"gsent\": \"%u\", "
             "\"glost\": \"%u\", "
             "\"vpackets\": \"%u\", "
             "\"vsent\": \"%u\", "
             "\"vlost\": \"%u\", "
             "\"radio\": \"%u\", "
             "\"buffer\": \"%u\""
             " }",
             gcsStatus->packets_received,
             gcsStatus->packets_sent,
             gcsStatus->packets_lost,
             vehicleStatus->packets_received,
             vehicleStatus->packets_sent,
             vehicleStatus->packets_lost,
             gcsStatus->radio_status_sent,
             vehicleStatus->queue_status);
    webServer.send(200, "application/json", message);
}

//---------------------------------------------------------------------------------
void handle_setParameters()
{
    if (webServer.args() == 0)
    {
        returnFail(kBADARG);
        return;
    }
    bool ok = false;
    bool reboot = false;
    if (webServer.hasArg(kBAUD))
    {
        ok = true;
        getWorld()->getParameters()->setUartBaudRate(webServer.arg(kBAUD).toInt());
    }
    if (webServer.hasArg(kPWD))
    {
        // Ensure password is between 8 characters and 16 characters
        if (webServer.arg(kPWD).length() >= 8 && webServer.arg(kPWD).length() <= 16)
        {
            ok = true;
            getWorld()->getParameters()->setWifiPassword(webServer.arg(kPWD).c_str());
        }
    }
    if (webServer.hasArg(kSSID))
    {
        // Ensure SSID is less than or equal to 16 characters
        if (webServer.arg(kSSID).length() <= 16)
        {
            ok = true;
            getWorld()->getParameters()->setWifiSsid(webServer.arg(kSSID).c_str());
        }
    }
    if (webServer.hasArg(kPWDSTA))
    {
        ok = true;
        getWorld()->getParameters()->setWifiStaPassword(webServer.arg(kPWDSTA).c_str());
    }
    if (webServer.hasArg(kSSIDSTA))
    {
        ok = true;
        getWorld()->getParameters()->setWifiStaSsid(webServer.arg(kSSIDSTA).c_str());
    }
    if (webServer.hasArg(kIPSTA))
    {
        IPAddress ip;
        ip.fromString(webServer.arg(kIPSTA).c_str());
        getWorld()->getParameters()->setWifiStaIP(ip);
    }
    if (webServer.hasArg(kGATESTA))
    {
        IPAddress ip;
        ip.fromString(webServer.arg(kGATESTA).c_str());
        getWorld()->getParameters()->setWifiStaGateway(ip);
    }
    if (webServer.hasArg(kSUBSTA))
    {
        IPAddress ip;
        ip.fromString(webServer.arg(kSUBSTA).c_str());
        getWorld()->getParameters()->setWifiStaSubnet(ip);
    }
    if (webServer.hasArg(kCPORT))
    {
        ok = true;
        getWorld()->getParameters()->setWifiUdpCport(webServer.arg(kCPORT).toInt());
    }
    if (webServer.hasArg(kHPORT))
    {
        ok = true;
        getWorld()->getParameters()->setWifiUdpHport(webServer.arg(kHPORT).toInt());
    }
    if (webServer.hasArg(kCHANNEL))
    {
        ok = true;
        getWorld()->getParameters()->setWifiChannel(webServer.arg(kCHANNEL).toInt());
    }
    if (webServer.hasArg(kDEBUG))
    {
        ok = true;
        getWorld()->getParameters()->setDebugEnabled(webServer.arg(kDEBUG).toInt());
    }
    if (webServer.hasArg(kMODE))
    {
        ok = true;
        getWorld()->getParameters()->setWifiMode(webServer.arg(kMODE).toInt());
    }
    if (webServer.hasArg(kREBOOT))
    {
        ok = true;
        reboot = webServer.arg(kREBOOT) == "1";
    }
    if (ok)
    {
        getWorld()->getParameters()->saveAllToEeprom();
        //-- Send new parameters back
        handle_getParameters();
        if (reboot)
        {
            delay(100);
            ESP.restart();
        }
    }
    else
        returnFail(kBADARG);
}

void handle_setDefaults()
{
    getWorld()->getParameters()->resetToDefaults();
    getWorld()->getParameters()->saveAllToEeprom();
    handle_getParameters();
}

//---------------------------------------------------------------------------------
static void handle_reboot()
{
    setNoCacheHeaders();

    sendHeaderContent();
    webServer.sendContent("<div class=p_content><div class='formbox'>rebooting ...</div></div></body></html>");
    webServer.sendContent("");
    delay(500);
    ESP.restart();
}


//---------------------------------------------------------------------------------
//-- 404
void handle_notFound()
{
    String message = "File Not Found\n\n";
    message += "URI: ";
    message += webServer.uri();
    message += "\nMethod: ";
    message += (webServer.method() == HTTP_GET) ? "GET" : "POST";
    message += "\nArguments: ";
    message += webServer.args();
    message += "\n";
    for (uint8_t i = 0; i < webServer.args(); i++)
    {
        message += " " + webServer.argName(i) + ": " + webServer.arg(i) + "\n";
    }
    webServer.send(404, FPSTR(kTEXTPLAIN), message);
}

//---------------------------------------------------------------------------------
MavESP8266Httpd::MavESP8266Httpd()
{
}

//---------------------------------------------------------------------------------
//-- Initialize
void MavESP8266Httpd::begin(MavESP8266Update *updateCB_)
{
    updateCB = updateCB_;
    webServer.on("/", handle_setup);
    webServer.on("/setparameters", handle_setParameters);
    webServer.on("/getparameters", handle_getParameters);
    webServer.on("/setdefaults", handle_setDefaults);
    webServer.on("/getstatus", handle_getStatus);
    webServer.on("/reboot", handle_reboot);
    webServer.on("/info.json", handle_getJSysInfo);
    webServer.on("/status.json", handle_getJSysStatus);
    webServer.on("/log.json", handle_getJLog);
    webServer.on("/update", handle_update);
    webServer.on("/upload", HTTP_POST, handle_upload, handle_upload_status);
    webServer.onNotFound(handle_notFound);
    webServer.begin();
}

//---------------------------------------------------------------------------------
//-- Initialize
void MavESP8266Httpd::checkUpdates()
{
    webServer.handleClient();
}
