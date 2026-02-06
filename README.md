# SmoBI the smart monitoring bin!
This is the code repository of the SmoBi project. 

This guide shows you how to **wire**, **install libraries**, **upload**, and **run** the provided Arduino sketch that:
- reads **two ultrasonic sensors** (HC-SR04-style),
- shows visual feeback for throw-in or block events: **NORMAL / GREEN (thank you) / RED (full)** screens on an **ILI9341 TFT**,
- plays a short **C–E–G “success/reward” sound** on a buzzer.

---

## 1) What you need

### Hardware
- 1× Arduino board (UNO / Nano / Mega compatible)
- 1× ILI9341 TFT display **with SPI pins** (commonly 2.4" / 2.8")
- 2× Ultrasonic sensors (HC-SR04 or compatible)
- 1× speaker
- Jumper wires
- Breadboard 

### Software
- Arduino IDE (v1.8.x or Arduino IDE 2.x)

---

## 2) Wiring (IMPORTANT)

tbd.

## 3) Install required libraries

The code uses:
- `Adafruit_GFX`
- `Adafruit_ILI9341`

### Steps (Arduino IDE 2.x or 1.8.x)
1. Open **Arduino IDE**
2. Go to **Library Manager**
3. Search and install:
   - **“Adafruit GFX Library”** (by Adafruit)
   - **“Adafruit ILI9341”** (by Adafruit)

> The IDE may also install dependencies automatically (like Adafruit BusIO). If prompted, install them.

---

## 4) Create the Arduino sketch

1. Open Arduino IDE
2. Go to **File → New**
3. Copy/paste your full code into the new sketch
4. Save it

---

## 5) Select the correct board + port

1. Plug your Arduino into your computer via USB
2. In Arduino IDE:
   - **Tools → Board →** select your board (e.g., **Arduino Uno**)
   - **Tools → Port →** select the port your Arduino is on

---

## 6) Upload the code

1. Click **Verify** (✓) to compile
2. Click **Upload** (→)

If the upload completes successfully, the board will reboot and start running the program.

---

## 7) What you should see (expected behavior)

### Screens / Modes
The program has 4 internal states, but only 3 main screens:

- **Normal (Navy background)**  
  Shows a happy face + “I have space!”

- **Green success screen (Green background)**  
  Happens when the sensors briefly detect something *then return to normal quickly*  
  It also plays the **C–E–G** tone sequence.

- **Red alarm screen (Red background)**  
  Happens when the sensors detect a “not normal” distance continuously long enough.  
  Displays “I am full!” and “next bin 200m” with an arrow.

### Timing logic 
- `NORMAL_CM = 17.0` and `TOLERANCE_CM = 2.0`  
  → A reading is considered normal if it’s between **15 cm and 19 cm**.
- A “brief transient” is < **1000 ms** (`TRANSIENT_MAX_MS`)  
  → transient triggers GREEN when it returns to normal quickly
- GREEN holds for **2000 ms** (`GREEN_HOLD_MS`)
- RED clears only after **2000 ms continuously normal** (`ALARM_CLEAR_MS`)

---

## 8) Calibration 

If your sensor mounting makes “normal distance” different than ~17 cm, change

const float NORMAL_CM = 17.0;
const float TOLERANCE_CM = 2.0;

You can use use the distance_measure.ino file for help with calibrating the sensors.
