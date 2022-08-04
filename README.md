# Summary of Low Power techniques with ESP*** and Pro Mini
Aiming to help anyone trying to run ESP*** from battery (and mention of Pro Mini also) with Alexa and Google Mini communication. There will be code examples in due course.
Not going to repeat in detail what is well described elsewhere but it took me 2 years to find and get working these 'tricks' so hoping this speeds it up for others.  
This will grow week by week as my code examples are all customised and embedded in large code systems now so I have to extract and test, but I should have kept notes so this will be my repository for remembering and reusing code sections.  
Will cover: ESP version, deepSleep, WiFi power use, OTA, AP, ESP_Now, variable persistence (RTC memory, LittleFS, CRC), state management, single button input method. 
I only consider ESP*** and Pro Mini development boards as they are cheap, easy to source and easy to program with Arduino IDE. 
## Low power basics. 
**Pro Mini** has a lower power requirement than ESP*** but the dev board needs a little modification for low power. The LED and regulator have to be removed and there are good descriptions elsewhere for doing this.   
A huge advantage of the Pro Mini is its wide voltage supply requirement. I'm talking now about the 3.3v 8 MHz version and I'm able to run one from a supercapacitor and small solar panel because it runs happily from 5v down to 1.8v,. I did have to switch off brown-out-detection (BOD) though. To get as low as 1.8v it's supposed to be neccesary to reduce the clock speed but I haven't found this neccessary' perhaps because when the voltage gets that low I have it in deepSleep and only waking up to check the supercapacitor vlotage.  
The Pro Mini lacks WiFi so I use RF433 modules (allowable frequency is country dependant so you may have to use different) which have a decent range through walls. A 16.5cm straight wire is as good as or better than the helix or spring aerials you can buy. The RCSwitch works well and you can save energy by setting the lowest repeat message that is reliable enough. I use PulseLength 220. Protocol 10 and send 3x for non critical messages and it transmits in about 0.3s using 7mA seconds of energy at 3.3v; even less at lower voltage. The RF433 module continues to transmit below 2v by the way.  
The quiescent current of the RF433 modules is supposed to be a few uA but some of them have not broken out this option to the dev board so I power them from a GPIO pin; you'd have to use a pin anyway to put it into quiescent mode so it's not a loss. However some do use uA when not transmitting so it's not always necessary.  
Unfortunaely it's one way unless you include a receiver, but that would have to be on all the time and will drain the battery, unless you have it wake up periodically and transmit a request for any messages. However for saving sensor data when the odd missing value doesn't matter it's fine. For something urgent I have a high repetition number in RCSwitch.    
RCSwitch is also effective at recognising codes from remote socket controllers allowing to replicate and control sockets by code. This provides an easy way to use voice control via Alexa --> ESP*** --> RF433 module --> remote socket.  
Battery choice for Pro Mini is easy as it is happy on a wide voltage range and doesn't need to use much power. 

**ESP8266** and similar   
If you need WiFi the WEMOS and NodeMCU type boards are easy to use but include power sapping peripherals. For low power an ESP12F is better but in order to flash the code you need either a Burner Board or homemade device. I've made a programmer from an ESP01 USB programmer and breadboard, with the ESP12Fs mounted on breakout boards so the pins match breadboard spacing but really it's worth getting a burner board which will also take the ESP01.
You can get the ESP12F to deepSleep at about 20uA but the big power drain is when you have to use the WiFi. Even without WiFi running there's still a fair current requirement. You can minimise this by sleeping a few seconds while a sensor warms up - my air pollution one needed 30s.
It's tricky measuring an average withvery low power consumption and occasional high short spikes so I use a 5F supercapacitor and measure the voltage drop over time. i = voltage drop x Farads/time. For example dropping 0.1v over 1 hour is i = 0.1v x 5F/(60*60) A = approx 30uA.    
Battery choice is important for ESP*** because they require 3 to 3.6v and short high current when WiFi is on. 2 * AA Lithium supply close to 3v until near the end of life and I've found the ESP still operates below 3v, though some sensors may not. I'm expecting a year's life from these powering ESP12F and DHT22 sensor for 5 minute readings along with weekly data downloads over WiFi.   
Getting data into and out of the ESP12F is easily achieved using the WiFi and a mobile phone, which requires broadcasting an Access Point (AP). There are other WiFi modes you may need to work with on the same device, such as ESP-Now, internet connection, OTA and these can be tricky to get working on the one device. I've succeeded by powering up into different states depending on a flag in flash memory. 

### Fast WiFi start up   
Code example ESP_fast_boot.ino          
Briefly, I have  WiFiTasks(); in startup() and it attempts to connect while (WiFi.status() != WL_CONNECTED) {....} for 1000 tries with delay(10); in between. 
I like to Serial.print the number of tries but just now it's taking around 30 and connecting in under 400ms though I've seen it much higher. I guess it depends on how busy the router is. 
On failure to connect in 1000 attempts it calls launchSlowConnect() which includes the WiFiManager commands. It will obviously get here the very first time it doesn't see a known SSID. 

### Booting into different WiFi modes      
Code example OTA_with_Battery.ino   
Over-The-Air updating code is very useful with battery devices because once you have built the device, changing code can be impossible without dismantling. So I now always include OTA, which uses WiFi, which is power hungry. 
The solution I use is to check a flag at Power On and this indicates which mode is required. Normal mode is an Access Point (AP) to allow input of any parameters or to set options and one option is to require OTA at next Power On. The AP has to time out to avoid some unintended reboot leaving the device stuck. 
I used existing code as the basis of the example and left a few extras in. 
There's code to read RTC and calculate a CRC value. You can use this as an alternative way of identifying Power On as the CRC will not match after reboot. 
To prove it's working I'm saving a counter which increments each sleep cycle.  
Note that RST must be connected to GPIO16 for the ESP to wake up after deep sleep. 
I removed my sensor value transmission code but left in the check digit calculation I use to append to the transmission to help spot corruption during transmission. 
It's possible there is the odd unnecessary line on account of me cutting back an existing larger program ! 

To use the code:   
On Power On there should be an SSID = OTA appear in your WiFi Network list. Connect to it and if you included a password you'll be asked for it.  
After a few seconds wait (sometimes many seconds but should be under a minute or you're stuffed) open 192.168.4.1 in your browser. Now you got an extra minute though it's worth increasing extendWaitForSetUp for any real use. The only function I've got is to input the OTA credentials (your own router WiFi network sign in). However I usually have more than this, for example if there's a sensor I might set the period between data collection. You can also grab a timestamp from the host device though the ESP will drift an hour or two over a week. I also usually have a button that says "shut off the AP and start sensing/deep sleeping".  
Alternatively if you do nothing after a minute the device will commence business as usual.  
If you provided the OTA credentials and rebooted, the Arduino IDE should now have in menu Tools, menu Port, a Network Port available. Selecting this will then let you flash code over the air.  


### ESP_Now -> ESP_Now --> internet


### 


## Refences
https://arduino-esp8266.readthedocs.io/en/latest/esp8266wifi/generic-class.html  
https://github.com/tzapu/WiFiManager   

