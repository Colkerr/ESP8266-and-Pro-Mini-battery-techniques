# Summary of Low Power techniques with ESP*** and Pro Mini
Aiming to help anyone trying to run ESP*** from battery (and mention of Pro Mini also) with Alexa and Google Mini communication. There will be code examples in due course.
Not going to repeat in detail what is well described elsewhere but it took me 2 years to find and get working these 'tricks' so hoping this speeds it up for others.  
This will grow week by week as my code examples are all customised and embedded in large code systems now so I have to extract and test, but I should have kept notes so this will be my repository for remembering and reusing code sections.  
Will cover: ESP version, deepSleep, WiFi power use, OTA, AP, ESP_Now, variable persistence (RTC memory, LittleFS, CRC), state management, single button input method. 
I only consider cheap, easy to source and easy program using Arduino IDE ESP*** and Pro Mini development boards  
## Low power basics. 
**Pro Mini** has a lower power requirement than ESP*** and the dev board needs a little modification for low power. The LED and regulator have to be removed and there are good descriptions elsewhere for doing this.   
A huge advantage of the Pro Mini is its wide voltage supply requirement. I'm talking now about the 3.3v 8 MHz version and I'm able to run one from a supercapacitor and small solar panel because it runs happily from 5v down to 1.8v,. I did have to switch off brown-out-detection (BOD) though. To get as low as 1.8v it's supposed to be neccesary to reduce the clock speed but I haven't found this neccessary' perhaps because when the voltage gets that low I have it in deepSleep and only waking up to check the supercapacitor vlotage.  
The Pro Mini lacks WiFi so I use RF433 modules (allowable frequency is country dependant so you may have to use different) which have a decent range through walls. A 17cm straight wire is as good as or better than the helix or spring aerials you can buy. The RCSwitch works well and you can save energy by setting the lowest repeat message that is reliable enough. I use PulseLength 220. Protocol 10 and send 3x for non critical messages and it transmits in about 0.3s using 7mA seconds of energy at 3.3v; even less at lower voltage. The RF433 module continues to transmit below 2v by the way.  
The quiescent current of the RF433 modules is supposed to be a few uA but some of them have not broken out this option to the dev board so I power them from a GPIO pin; you'd have to use a pin anyway to put it into quiescent mode so it's not a loss. However some do use uA when not transmitting so it's not always necessary.  
Unfortunaely it's one way unless you include a receiver, but that would have to be on all the time and will drain the battery, unless you have it wake up periodically and transmit a request for any messages. However for saving sensor data when the odd missing value doesn't matter it's fine. For something urgent I have a high repetition number in RCSwitch.    
RCSwitch is also effective at recognising codes from remote socket controllers allowing to replicate and control sockets by code. This provides an easy way to use voice control via Alexa --> ESP*** --> RF433 module --> remote socket.  
Battery choice for Pro Mini is easy as it is happy on a wide voltage range and doesn't need to use much power. 

**ESP8266** and similar
If you need WiFi the WEMOS and NodeMCU type boards are easy to use but include power sapping peripherals. For low power an ESP12F is better but in order to flash the code you need either a Burner Board or homemade device. I've made a programmer from an ESP01 USB programmer and breadboard, with the ESP12Fs mounted on breakout boards so the pins match breadboard spacing but really it's worth getting a burner board which will also take the ESP01.
You can get the ESP12F to deepSleep at about 20uA but the big power drain is when you have to use the WiFi. Even without WiFi running there's still a fair current requirement. You can minimise this by sleeping a few seconds while a sensor warms up - my air pollution one needed 30s.
It's tricky measuring an average withvery low power consumption and occasional high short spikes so I use a 5F supercapacitor and measure the voltage drop over time. i = voltage drop x Farads/time. For example dropping 0.1v over 1 hour is i = 0.1v x 5F/(60*60) A = approx 30uA.    
Battery choice is important for ESP*** because they require 3 to 3.6v and short high current when WiFi is on. 2 * AA Lithium supply close to 3v until near the end of life and I've found the ESP still operates to below 2.9v. I'm expecting a year's life from these powering ESP12F and DHT22 sensor for 5 minute readings along with weekly data downloads over WiFi.   
Getting data into and out of the ESP12F is easily achieved using the WiFi and a mobile phone, which requires broadcasting an Access Point (AP). There are other WiFi modes you may need to work with on the same device, such as ESP-Now, internet connection, OTA and these can be tricky to get working on the one device. I've succeeded by powering up into different states depending on a flag in flash memory. 

### Fast WiFi start up  

### Booting into different WiFi modes   

### ESP_Now -> ESP_Now --> internet

### OTA + AP

### 







