//Demonstrate fast WiFi reconnection after deepSleep. Also Reset reason, and deepSleep.
#include <WiFiManager.h>  //https://github.com/tzapu/WiFiManager 

void setup() {
  Serial.begin(9600);       // start up serial port
  Serial.println();
  Serial.print( __FILE__); Serial.print(__TIME__); Serial.println(__DATE__); 
  Serial.println(ESP.getResetReason().c_str());    // this should show power up here when turned on
  WiFiTasks();
}

void loop() {
    Serial.println(millis()); Serial.println("    when connected");
    ESP.deepSleep(10e6);   
}

void WiFiTasks() {
  int counter=0;
  while (WiFi.status() != WL_CONNECTED) {
     delay(10);      // use small delays, NOT 500ms
     if (++counter > 1000) launchSlowConnect();     // if timed-out, connect the slow-way
  }
  Serial.print(" fast tries = ");Serial.println(counter);
  //
}

// this is accessed when the initial run fails to connect because no (or old) credentials
void launchSlowConnect() {
  int counter = 0;
  Serial.println("slow connect");
  enableWiFiAtBootTime(); //for releases >=3
  // persistent and autoconnect should be true by default, but lets make sure.
  WiFi.mode(WIFI_STA);
  if (!WiFi.getAutoConnect()) WiFi.setAutoConnect(true);  // autoconnect from saved credentials
  if (!WiFi.getPersistent()) WiFi.persistent(true);     // save the wifi credentials to flash
 
  WiFiManager wifiManager; // wifiManager.resetSettings();
  wifiManager.autoConnect("WiFiManager");
  // now wait for good connection, or reset
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (++counter > 20) {        // allow up to 10-sec to connect to wifi
        Serial.println("wifi timed-out. Rebooting..");
        delay(10);  // so the serial message has time to get sent
        ESP.restart();
    }
  }
  WiFi.persistent(false);     // already saved above so don't bother launchSlowConnect() needed.
  Serial.print("connected, given IP address: "); String IP_Address = WiFi.localIP().toString();
  Serial.println(IP_Address); 
  Serial.println("WiFi connected and credentials saved");
}
