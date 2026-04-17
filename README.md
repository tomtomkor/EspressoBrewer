Espresso brewing process consists of 3 phases: preinfustion, blooming, and extracting.  
You can do that easily on Gaggia machines if you have a dimmer and connect it to the pump. 
I wanted it to be done automatically and repeatedly based on my favorate recipes.

Hardware requirements:
- ESP32C3 supermini board (because of its small form factor)
- Robotdyn AC dimmer module (4A or higher model is OK; controls the pump power)
- AC 220V optocoupler isolation module (detects on/off status of the extraction button)
- TTP223 touch sensor (mode toggle: manual/automatic(brew))
- 0.91' OLED (I2C)
- Perfboard (7X9; optional, preferred due to pump vibration)

* All are cheap and available on Aliexpress or Amazon (look in Photos folder)

There are two program files: an Arduino code file for ESP32C3(.ino) & a mobile app(.apk)
- The Arduino file should be compiled and uplodaded to ESP32C3 via Arduino IDE or the like.
- The mobile app was built using 'MIT App Inventor'(https://appinventor.mit.edu).
  I have uploaded the associated file(.aia) here and so you can download and modify it for 
  rebuling the apk file if you want to.
  

< Workflow >

- When you power on your Gaggia it is set to 'Manual' mode so that you can purge off air
  along the pipe line by pulling water. In this mode the pump operates in its full capacity.

- Tap the touch sensor and it changes to 'Brew' mode. 

- Ready: Wait for 5 minutes of Warming up. The coffee machine will be 'Ready' to start brewing.
  (In my case, the PID(XMT 7100) installed on my Gaggia Classic indicates that the boiler
  temperature gets stabilized around the target 5 min. after it was turned on.
  This may differ according to situations. You can change this value(var: warmupLImit)
  in the arduino file.)

- Preinfusion: Turn the extraction switch of Gaggia on. Preinfusion phase will begin.
  This lasts at the user-set pump power level(percentage of the full pump capacity)
  during the user-set time.

- Blooming: At the end of preinfusion, the pump will stop and pause for blooming during
  the user-set bloom time.

- Ramping up: The pump power level gradually increases to the user-set max level(typically
  full power; 100%). You can try lower pressure extraction (e.g. Turbo shot) if you set the
  max level to be lower than 100%.

- Back-to-Back: Turn off the extraction switch off. The coffee machine will warm up again
  amd will be 'Ready' after 2 minutes. (This also can be changed in the arduino file.)

- Back Flushing: Tap the touch sensor. It changes to 'Manual' mode. Do flushing as you need.


