Espresso brewing process consists of 3 phases: preinfustion, blooming, and extracting.  
You can do that easily on old espresso machines like my Gaggia Classic if you have a dimmer and connect it to the pump. 

I wanted it to be done automatically based on my favorate recipes.

// Hardware requirements:
- ESP32C3 supermini board (because of its small form factor)
- Robotdyn AC dimmer module (4A or higher model is OK; controls the pump power)
- AC 220V optocoupler isolation module (detects on/off status of the extraction button)
- TTP223 touch sensor (mode toggle: manual/automatic(brew))
- 0.91' OLED (I2C)
- Perfboard (7X9; optional, preferred considering pump vibration)

  * All are cheap and available on Aliexpress or Amazon (look in Photos folder)

There are two program files: an Arduino code file for ESP32C3(CoffeeBrewer.ino) & a mobile app(BrewMate.apk)
- The Arduino file should be compiled and uplodaded to ESP32C3 via Arduino IDE or the like.
- The mobile app was built using 'MIT App Inventor'(https://appinventor.mit.edu).
  I have uploaded the associated file(BrewMate.aia) and you can download and modify it for rebuling an apk file if you want to.

// Workflow

- When you power on your machine it is set to 'Manual' mode so that you can purge off air along the pipe line by pulling water. In this mode the pump operates in its full capacity.
- Tap the touch sensor and it changes to 'Brew' mode. 

- Ready: Wait for 5 minutes of Warming up. The coffee machine will be 'Ready' to start brewing. (In my case, the PID(XMT 7100) installed on my Gaggia Classic indicates that the boiler temperature gets stabilized around the target 5 min. after it was turned on. This may differ according to situations and you can change this value(var: warmupLImit) in the arduino file.)

- Preinfusion: Turn the extraction switch of the machine on. Preinfusion phase will begin. This lasts at the user-set pump power level(percentage of the full pump capacity) during the user-set time.

- Blooming: At the end of preinfusion, the pump will stop and pause for blooming during the user-set bloom time.

- Ramping up: The pump power level gradually increases to the user-set max level(typically full power; 100%). You can try lower pressure extractions (e.g. Turbo shot) if you set the max level to be lower than 100%.

- Extraction: User-set max pump power is applied until you turn the extraction switch off.

- Back-to-Back: The coffee machine will warm up again. It will be 'Ready' after 2(not 5) minutes. This also can be changed in the arduino file.

- Back Flushing: Tap the touch sensor. It changes to 'Manual' mode. Do flushing as you need.

// Setting parameters for brewing (Mobile app)

- You can set and adjust parameter values using the mobile app 'BrewMate'.
- The parameters are: preinfusion duration(PreTime), preinfusion pump power(PrePower), blooming duration(Pause), maximum pump power for extraction(MxPower), and ramp-up duration(RampUp)

- Those 5 parameter values with a name consists a 'Profile'

- You can save multiple profiles on the phone and manage them (add/delete)

- Retrieve one of them by name and send it to ESP32C3 via http protocol

- Once ESP32C3 gets the profile you sent from the phone, the received parameter values are applied to brewing.

  * When launched for the first time, the app requests the local IP address assigned to ESP32C3(e.g.: 192.168.0.100).
  * In advance, local IP address should be provided in the CoffeeBrewer.ino together with your SSID and password of your router.
  * 'BrewMate' is so simple that you can learn it in a few minutes without a manual.

// Wiring

- Pump Power(L)      ───→ Optocoupler L
- Dimmer(output L)   ───→ Pump
- Main power(or PID*) L  ───→ Dimmer (Input L)

- Main power(or PID)  N 
                               ├──→ Dimmer N
                               └──→ Optocoupler N

  * If you have a PID installed.
