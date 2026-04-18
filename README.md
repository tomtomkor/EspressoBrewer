# ☕ Brew Process Automation (ESP32-C3)

This project automates the espresso brewing process into three distinct phases: **Pre-infusion**, **Blooming**, and **Extraction**. By adding a dimmer and an ESP32-C3 to a classic machine like the Gaggia Classic, you can achieve professional pressure profiling based on your favorite recipes.

---

## 🛠 Hardware Requirements

* **Controller:** ESP32-C3 SuperMini (chosen for its compact form factor)
* **Dimmer:** Robotdyn AC Dimmer module (4A or higher model; controls pump power)
* **Sensor:** AC 220V Optocoupler isolation module (detects the status of the extraction button)
* **Input:** TTP223 Touch sensor (Mode toggle: Manual / Automatic Brew)
* **Display:** 0.91" OLED (I2C)
* **Build:** Perfboard (7x9 cm recommended to withstand pump vibration)
* **Case:** 3D printed enclosure (`EspressoBrewerCase.stl` included)

> **Note:** All components are affordable and easily available on AliExpress or Amazon. Refer to the `Photos` folder for visual aids.

---

## 📱 Software

* **Arduino Code (`EspressoBrewer.ino`):** Should be compiled and uploaded to ESP32-C3 via Arduino IDE.
* **Mobile App (`BrewMate.apk`):** Built using [MIT App Inventor](https://appinventor.mit.edu).
  * The associated source file (`BrewMate.aia`) is uploaded; you can modify and rebuild it if needed.

---

## 🔄 Workflow

1. **Manual Mode (Default):** On power-up, the machine starts in Manual mode. The pump operates at full capacity, allowing you to purge air or flush the group head.
2. **Mode Toggle:** Tap the touch sensor to switch between **Manual** and **Brew** modes.
3. **Warm-up:** In Brew mode, the system waits for a warmup period (Default: 5 min). 
   * *Note: Based on my Gaggia Classic with PID (XMT 7100), it takes about 5 minutes for the 105°C boiler temp to stabilize. You can adjust the `warmupLimit` variable in the Arduino file.*
4. **Pre-infusion:** Turn the extraction switch ON. The pump runs at a user-specified power level for a set duration(0~20 seconds).
5. **Blooming:** The pump stops completely for a "pause" or blooming phase(0~60s).
6. **Ramping Up:** Pump power gradually increases from 0% to the user-specified max level over 0~15 seconds .
7. **Extraction:** Maintains the specified max pump power until the extraction button is turned OFF manually.
8. **Back-to-Back:** After a shot, the machine reheats and returns to "Ready" status in 2 minutes. (This duration is also adjustable in the CoffeeBrewer.ino file.)
9. **Back Flushing:** Switch to Manual mode via the touch sensor to perform flushing as needed.

---

## ⚙️ Setting Parameters (Mobile App)

You can adjust parameters and manage profiles using the **BrewMate** app:

* **PreTime:** Pre-infusion duration
* **PrePower:** Pump power during pre-infusion
* **Pause:** Blooming duration
* **RampUp:** Duration to reach max power
* **MxPower:** Maximum pump power (set below 100% for "Turbo Shots")

**Connection Details:**

* The app requests the local IP address assigned to the ESP32-C3 (e.g., `192.168.0.119`) on first launch.
* Configure your **SSID**, **Password**, and **Static IP** within the `EspressoBrewer.ino` file.
* The app is designed to be intuitive and can be mastered in minutes without a manual.

---

## 🔌 Wiring

* **Power (L) to Pump** ───→ Optocoupler (L)
* **Dimmer (Output L)** ───→ Pump
* ***Main Power (or PID*) L*** ───→ Dimmer (Input L)
* ***Main Power (or PID*) N*** ───→ Dimmer (N)
* ***Main Power (or PID)*** **N*** ───→ Optocoupler (N)
  *\*If a PID is installed.*

> **Pin Mappings:** Detailed ESP32-C3 pin mappings for each module are documented in the `EspressoBrewer.ino` file.

---

## 📂 Files Included

* `EspressoBrewer.ino`: Main Arduino source code.
* `BrewMate.apk`: Android application for profile management.
* `BrewMate.aia`: MIT App Inventor project file.
* `EspressoBrewerCase.stl`: 3D printable case file.
* `Photos/`: Hardware setup and wiring reference photos.
