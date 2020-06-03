#include <WiFi.h>
#include <WebServer.h>
#include <AutoConnect.h>
#include <ESPmDNS.h>
#include <time.h> 

WebServer Server;
AutoConnect portal(Server);
AutoConnectConfig Config("", "");
AutoConnectAux    menuConfig("/config.htm", "Config");   // Step #1 as the above procedure
const char* mDnsHostName = "Teo";  
bool isWifiConnected = false;

uint32_t wifiRetryTimer = millis();
uint32_t wifiRetryTimout = 600000;
void sendRedirect(String uri) {
  WebServerClass& Server = portal.host();
  Server.sendHeader("Location", uri, true);
  Server.send(302, "text/plain", "");
  Server.client().stop();
}

bool atDetect(IPAddress softapIP) { 
  Serial.println("Captive portal started, SoftAP IP:" + softapIP.toString());
  return true;
}


//void deleteAllCredentials(void) {
//  AutoConnectCredential credential;
//  station_config_t config;
//  uint8_t ent = credential.entries();
//
//  while (ent--) {
//    uint8_t entry = 0;
//    credential.load(entry, &config);
//    credential.del((const char*)&config.ssid[0]);
//  }
//}

void setup()
{
   // connect at 115200 so we can read the GPS fast enough and echo without dropping chars
  // also spit it out
  Serial.begin(115200);

  struct tm tmstruct ;
  delay(2000);
  tmstruct.tm_year = 0;
  getLocalTime(&tmstruct, 5000);
  Serial.printf("\nNow is : %d-%02d-%02d %02d:%02d:%02d\n",(tmstruct.tm_year)+1900,( tmstruct.tm_mon)+1, tmstruct.tm_mday,tmstruct.tm_hour , tmstruct.tm_min, tmstruct.tm_sec);
  Serial.println("");
  
  Config.boundaryOffset = 256;
  Config.autoReconnect = true; 
  Config.ota=AC_OTA_BUILTIN;
  uint64_t chipid = ESP.getEfuseMac();  
  Config.apid = "TeoGPS_" + String((uint32_t)chipid, HEX);
  Config.apip = IPAddress(192,168,10,1);      // Sets SoftAP IP address
  Config.gateway = IPAddress(0,0,0,0);     // Sets WLAN router IP address
  Config.netmask = IPAddress(255,255,255,0);    // Sets WLAN scope
  Config.psk = "12345678";  // The lenght should be from 8 to up to 63.
  Config.ticker = true;  
  Config.tickerPort = LED_BUILTIN;
  Config.tickerOn = LOW;
  Config.homeUri = "/";
  Config.immediateStart = false;
  Config.retainPortal = true;
  Config.portalTimeout = 5000;  //Wait 30 Seconds to Connect to AP if Not found Start Captive, This will delay our Loop Start
  Config.bootUri = AC_ONBOOTURI_HOME;
  Config.autoReset=false;
  Config.autoRise = true;
  Config.minRSSI = -70;
  Config.principle = AC_PRINCIPLE_RSSI;
  
  //Config. menuItems = AC_MENUITEM_CONFIGNEW | AC_MENUITEM_OPENSSIDS | AC_MENUITEM_DISCONNECT | AC_MENUITEM_RESET | AC_MENUITEM_UPDATE | AC_MENUITEM_HOME;
  //Config.autoSave = AC_SAVECREDENTIAL_NEVER;
  portal.config(Config);
  portal.onDetect(atDetect); 
  portal.onNotFound(handleNotFound);
  //server.on("/config", handleConfig);
  portal.join({ menuConfig });
  Serial.println("Starting File System."); 
  setupFileSystem();
  delay(100);
  
  Serial.println("Starting GPS.");
  gpsSetup();
  delay(100);
  Serial.println("Starting GPS Http.");
  gpsHttpSetup();
  delay(100);
  Serial.println("Starting Http Server.");
  setupHttpServer();
  delay(100);
  Serial.println("MDNS Being.");
  MDNS.begin(mDnsHostName);
  MDNS.setInstanceName("teo's gps tracker");
  MDNS.addService("_http", "_tcp", 80);
  Serial.println("Portal Begin.");
  //deleteAllCredentials();
  isWifiConnected = portal.begin();
  if ( isWifiConnected ){
    Serial.println("Started, IP:" + WiFi.localIP().toString());
    //Serial.println("Portal Ending.");
    //portal.end();
  }else {  
    Serial.println("Connection failed.");
    
  }
}  

bool portalBegin = false;


void loop() // run over and over again
{
  portal.handleClient();
  gpsLoop();
  
  
  if (WiFi.status() != WL_CONNECTED) {
      if (wifiRetryTimer > millis()){
        Serial.println("wifiRetryTimer wrapped");
        wifiRetryTimer = millis();
      }
      // approximately every 10 seconds or so, print out the current stats
      if (millis() - wifiRetryTimer > wifiRetryTimout) {
        
        Serial.println("Retry Wifi Connection with Portal.begin");
        //isWifiConnected = portal.begin();
        if ( isWifiConnected ){
          Serial.println("Started, IP:" + WiFi.localIP().toString());
        }else {  
          Serial.println("Connection failed.");
          
        }
      
        wifiRetryTimer = millis();  // reset the gpsTimer
      }
  } 
}
