//OTA with battery power. Requires booting into different modes.
//At boot up, reads littleFS OTA.txt to determine whether to connect to WiFi for OTA update, or open AP 192.168.4.1 for user input

#include <ESP8266WiFi.h>   //OTA + AP
#include <ESP8266mDNS.h>   //OTA
#include <ArduinoOTA.h>    //OTA
#include <LittleFS.h>
#include <ESP8266WebServer.h>  //AP
#include <WiFiUdp.h>       //OTA
ESP8266WebServer server(80);    // Create a webserver object that listens for HTTP request on port 80

#define User "OTA"                 //For Access Point. Change these as you wish
#define Password "password"         //optional
             
extern "C" {          // this is for the RTC memory read/write functions
  #include "user_interface.h" 
}

#define RTCMEMORYSTART 65
typedef struct {      // this is for the RTC memory read/write functions
  int iCount;
} rtcSubStore;

typedef struct {
  uint32_t crc32;
  rtcSubStore rtcData;
} rtcStore;

rtcStore rtcMem;

bool bOTAmode;
String sOTA_PW, sOTA_SSID;
long waitForSetUp = 1 * 60 * 1000, extendWaitForSetUp = 2 * 60 * 1000; //extend if web page requested

void setup() {
  Serial.begin(9600);  
  Serial.print(__FILE__); Serial.print(__TIME__); Serial.println(__DATE__); 
  Serial.println();
  Serial.print("Initialise start up reason =  ");
  Serial.println(ESP.getResetReason() );

  readFromRTCMemory();
  if (LittleFS.begin()) Serial.println("LittleFS begin OK");

  if ( ESP.getResetReason() != "Power On" ) {      // as opposed to Deep-Sleep Wake
    //do stuff, maybe read sensor and store in Flash memory so survuves Power On.
    //we will just increment a counter in RTC memory and prove it's working
    rtcMem.rtcData.iCount++;
    Serial.print("Our counter = "); Serial.println(rtcMem.rtcData.iCount);
    writeToRTCMemory();  //store anything only needed for next awake period
    //### RST must be connected to GPIO16 to wake up after deep sleep ###
    ESP.deepSleep(60e6, WAKE_RF_DISABLED );   //wake after 60s
  } else {
     if ( OTAmode() ) {
        bOTAmode = true;
        OTA_disable();  //so next time normal mode
        OTA_routine();
     } else {      
      //>>>>>>>>>>>>>serve web page<<<<<<<<<<<<<  
      WiFi.begin();
      Serial.println(WiFi.softAP(User, Password) ? "Ready" : "Failed!");
      Serial.print("Soft-AP IP address = ");
      Serial.println(WiFi.softAPIP());
      server.on("/prepare_OTA_boot", prepare_OTA_boot); 
      server.on("/", input_OTA_creds);
      server.onNotFound([]() {                              // If the client requests any URI
       // if (!handleFileRead(server.uri()))                // send it if it exists
          server.send(404, "text/plain", "404: Not Found"); // otherwise, respond with a 404 (Not Found) error
      });
      server.begin();                           
      Serial.println("HTTP server started");
      delay(10);
    } 
    Serial.println("Set iCount = 0 ");
    rtcMem.rtcData.iCount = 0;
  }
}

void loop() {   //waiting for client then timeout and sleep   
  if ( bOTAmode ) {
    Serial.println(" OTA mode ");
    ArduinoOTA.handle();
  } else {
    if ( millis() < waitForSetUp ) {   //this extended using extendWaitForSetUp if request received.
      server.handleClient();
      writeToRTCMemory();
      delay(10);
    } else {
      ESP.deepSleep(10e6, WAKE_RF_DISABLED );   //wake after 10s
    }
  }
}

void readFromRTCMemory() {
  system_rtc_mem_read(RTCMEMORYSTART, &rtcMem, sizeof(rtcMem));
  yield();
}

void writeToRTCMemory() { 
  uint32_t crc32Calc = calculateCRC32((uint8_t*) &rtcMem.rtcData, sizeof(rtcMem.rtcData));
  rtcMem.crc32 = crc32Calc;
  system_rtc_mem_write(RTCMEMORYSTART, &rtcMem, sizeof(rtcMem));
  yield();
}

uint32_t calculateCRC32(const uint8_t *data, size_t length) {
  uint32_t crc = 0xffffffff;
  while (length--) {
    uint8_t c = *data++;
    for (uint32_t i = 0x80; i > 0; i >>= 1) {
      bool bit = crc & 0x80000000;
      if (c & i) {
        bit = !bit;
      }
      crc <<= 1;
      if (bit) {
        crc ^= 0x04c11db7;
      }
    }
  }
  return crc;
}

int checkDigit(unsigned long ul) {
  int iAccum = 0;
  while ( ul > 0 ) {
    iAccum = iAccum + ul%10;
    ul = ul/10;
  }
  return iAccum%10;
} 

bool OTAmode() {
  if (!LittleFS.exists("/OTA.txt" ) ) {
    return false;
  } else {
    File f0 = LittleFS.open("/OTA.txt","r") ;
    if ( f0.size() < 5 ) return false; 
    Serial.println("OTA.txt looks good lets try and use OTA");
    sOTA_SSID = f0.readStringUntil('/');
    sOTA_PW = f0.readStringUntil('/');
    WiFi.begin(sOTA_SSID, sOTA_PW);
    while (WiFi.waitForConnectResult() != WL_CONNECTED) {
      Serial.println(" Could not connect to WiFi for OTA ");
      return false;
    }
    return true;
  }
}

void OTA_disable() {
  File f0 = LittleFS.open("/OTA.txt", "w");  
  f0.print("");
  f0.close();
}

void prepare_OTA_boot() { 
  sOTA_SSID = server.arg(0);
  sOTA_PW = server.arg(1);
  sOTA_SSID.trim();
  sOTA_PW.trim();
  Serial.print("ota ................ ");Serial.print(sOTA_PW);Serial.println(sOTA_SSID);
  File f0 = LittleFS.open("/OTA.txt", "w");  
  f0.print(sOTA_SSID + "/" + sOTA_PW + "/");
  f0.close();
  Serial.println("whats in ota.txt ");
  f0 = LittleFS.open("/OTA.txt","r"); Serial.println(f0.readStringUntil('/'));Serial.println(f0.readStringUntil('/'));f0.close();
  Serial.println(" end of prepare ota ");
  delay(2000);
  ESP.restart();
}


//OTA stuff
void input_OTA_creds() {
   waitForSetUp = extendWaitForSetUp + millis(); //must be intended restart so allow longer 
   server.send(200, "text/html", "<h1>OTA Credentials: </h1>"
      "<form method='post' name='frm' action='/prepare_OTA_boot'> "
       " Input SSID and PASSWORD &nbsp; &nbsp; "       
         "<input type='text'  name='ssid' value = '' ><br><br>"
         "<input type='text'  name='pw'   value = '' ><br><br>"
         "<p><input type='submit' value='OTA on reboot' &nbsp;&nbsp; &nbsp; />"   
         "</form> <br> "
   );
}

void OTA_routine() {
  delay(2000);
  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  // ArduinoOTA.setHostname("myesp8266");

  // No authentication by default
  // ArduinoOTA.setPassword((const char *)"123");

  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}
