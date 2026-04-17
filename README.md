Espresso brewing process consists of 3 phases: preinfustion, blooming, and extracting.  
You can do that easily on Gaggia machines if you have a dimmer and connect it to the pump. 
I wanted it to be done automatically and repeatedly based on my favorate recipes.

Hardware requirements:
- ESP32C3 supermini board (because of its small form factor)
- Robotdyn AC dimmer module (4A or higher model is OK; controls the pump power)
- AC 220V optocoupler isolation module (detects on/off status of the extraction button)
- TTP223 touch button (mode toggle: manual/automatic(brew))

* All are cheap and available on Aliexpress or Amazon (look in Photos folder)

There are two program files: an Arduino code file for ESP32C3(.ino) & a mobile app(.apk)
- The Arduino file should be compiled and uplodaded to ESP32C3 via Arduino IDE or the like.
- The mobile app was built using 'MIT App Inventor'(https://appinventor.mit.edu).
  I have uploaded the associated file(.aia) here and so you can download and modify it for 
  rebuling the apk file if you want.


