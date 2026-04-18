☕ Gaggia Classic Automation (ESP32-C3)

This project automates the espresso brewing process into three distinct phases: Pre-infusion, Blooming, and Extraction. By adding a dimmer and an ESP32-C3 to a classic machine like the Gaggia Classic, you can achieve professional pressure profiling based on your favorite recipes.

🛠 Hardware Requirements

Controller: ESP32-C3 SuperMini (chosen for its compact form factor)

Dimmer: Robotdyn AC Dimmer module (4A or higher model; controls pump power)

Sensor: AC 220V Optocoupler isolation module (detects the status of the extraction button)

Input: TTP223 Touch sensor (Mode toggle: Manual / Automatic Brew)

Display: 0.91" OLED (I2C)

Build: Perfboard (7x9 cm recommended to withstand pump vibration)

Case: 3D printed enclosure (EspressoBrewerCase.stl included)

Note: All components are affordable and easily available on AliExpress or Amazon. Refer to the Photos folder for visual aids.

📱 Software

Arduino Code (EspressoBrewer.ino): Should be compiled and uploaded to ESP32-C3 via Arduino IDE.

Mobile App (BrewMate.apk): Built using MIT App Inventor.

The associated source file (BrewMate.aia) is uploaded; you can modify and rebuild it if needed.

🔄 Workflow

Manual Mode (Default): On power-up, the machine starts in Manual mode. The pump operates at full capacity, allowing you to purge air or flush the group head.

Mode Toggle: Tap the touch sensor to switch between Manual and Brew modes.

Warm-up: In Brew mode, the system waits for a warmup period (Default: 5 min).

Note: Based on my Gaggia Classic with PID (XMT 7100), it takes about 5 minutes for the 105°C boiler temp to stabilize. You can adjust the warmupLimit variable in the Arduino file.

Pre-infusion: Turn the extraction switch ON. The pump runs at a user-specified power level for a set duration.

Blooming: At the end of pre-infusion, the pump will completely stop and pause for blooming during the user-specified bloom time.

Ramping Up: Pump power gradually increases to the user-specified maximum level.

Extraction: Maintains the specified max pump power until the extraction button is turned OFF manually.

Back-to-Back: After a shot, the machine warms up again and will be "Ready" after only 2 minutes.

Back Flushing: Switch to Manual mode via the touch sensor to perform flushing as needed.

⚙️ Setting Parameters (Mobile App)

You can adjust parameters and manage profiles using the BrewMate app:

PreTime: Pre-infusion duration

PrePower: Pump power during pre-infusion

Pause: Blooming duration

RampUp: Duration to reach max power

MxPower: Maximum pump power (set below 100% for "Turbo Shots")

Connection Details:

The app requests the local IP address assigned to the ESP32-C3 (e.g., 192.168.0.119) on first launch.

Configure your SSID, Password, and Static IP within the EspressoBrewer.ino file.

The app is designed to be intuitive and can be mastered in minutes without a manual.

🔌 Electrical Wiring Guide

This guide covers both standard machines and those with a PID controller (e.g., XMT 7100) installed.

💡 Wiring Schematic Flow

[ CASE 1: Standard Machine ]
AC Hot (L) ---------------------> [ Dimmer Input L ]
AC Neutral (N) ---------+-------> [ Dimmer N ]
                        |
                        +-------> [ Optocoupler N ]

[ CASE 2: With PID (Optional) ]
AC Hot (L) ----> [ PID Controller ] --(Output L)--> [ Dimmer Input L ]
AC Neutral (N) ---------+-------------------------> [ Dimmer N ]
                        |
                        +-------------------------> [ Optocoupler N ]

[ PUMP & SENSING ]
Dimmer Output L ---------------> [ Pump (L) ] ---+
                                                 |
                                                 +--> [ Optocoupler L ]
AC Neutral (N) ----------------------------------+


Pin Mappings: Detailed ESP32-C3 pin mappings for each module are documented in the EspressoBrewer.ino file.

📂 Files Included

EspressoBrewer.ino: Main Arduino source code.

BrewMate.apk: Android application for profile management.

BrewMate.aia: MIT App Inventor project file.

EspressoBrewerCase.stl: 3D printable case file.

Photos/: Hardware setup and wiring reference photos.
