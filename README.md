# tiny-bleKeyboard-Cardputer
**[`　日本語　`](README_jp.md)**

# Bluetooth Keyboard for Cardputer User Manual

## 0. Changelog

*   v101 2025-06-07
    *   Initial release
*   v102 2025-06-08
    *   Removed `Insert`, `PrintScreen` Keys from cursor movement mode
*   v103 2025-06-11
    *   M5Cardputer lib modifications

---

## 1. Introduction

This software (`tiny-bleKeyboard`) is for using the M5Stack Cardputer as a Bluetooth keyboard.
All printed keys are implemented. Other useful keys are also implemented, allowing you to use the Cardputer almost like a standard small Bluetooth keyboard.

Main Features:
*   Standard key input, sending modifier keys (Shift, Ctrl, Alt, Opt)
*   Special functions in combination with the Fn key
    *   CapsLock On/Off (Fn + 1)
    *   Cursor movement mode On/Off (Fn + 2)
    *   Auto Power Off (APO) time setting (Fn + 3)
*   Power saving features
    *   Screen brightness reduction after a period of inactivity
    *   Transition to deep sleep after APO warning when set time elapses
    *   Transition to deep sleep after warning display when battery is low
*   Status confirmation via screen display

## 2. Startup and Screen Display

### Startup
The software starts when the Cardputer is powered on.
If the 'a' key is pressed during startup, it will switch to the app selection menu for BIN files on the SD card (see "7. Switching Applications" for details).

### Screen Display Layout
The screen consists of 6 lines and displays the following information:

*   **L0 (1st line): Title and Bluetooth connection status**
    *   Example: `- tiny bleKeyboard -`
    *   The color of the word `ble` indicates the connection status (Blue: Connected, Red: Not connected)
*   **L1 (2nd line): Battery level**
    *   Example: `            bat. 76%`
*   **L2 (3rd line): Guide for Fn key functions**
    *   Example: `fn1:Cap 2:CurM 3:Apo`
*   **L3 (4th line): Current status of Fn key functions**
    *   CapsLock status (Example: `unlock` / `lock`)
    *   Cursor movement mode status (Example: `off` / `on`)
    *   APO set time (Example: `20min` / `off`)
*   **L4 (5th line): Modifier key status / Power saving warning**
    *   Pressed modifier keys (Example: `Shift Ctrl Alt Opt Fn`)
    *   APO warning: `     SLEEP TIME     ` (blinking)
    *   Low battery warning: `    LOW BATTERY !!  ` (blinking)
*   **L5 (6th line): Sent key information**
    *   Input character and HID code (Example: when `a` is pressed, `a :hid 0x04`)
    *   HID code only when special keys are sent (Example: when `ENTER` is pressed, `hid 0x28`)

## 3. Basic Usage

* All keys printed on the Cardputer are implemented.

Additionally, the following are implemented:
* The `Opt` key is assigned to the `Win`/`Cmd` key for PC keyboards.
* Editing keys such as `Home`, `End`, `PageUp`, `PageDown`, `PrintScreen`, `Insert` are additionally assigned.
* `F5` to `F10`, often used with Japanese IMEs, are additionally assigned.
* Key repeat when holding down a key has been confirmed to work.

### Bluetooth Pairing
1.  Turn on the Cardputer.
2.  Open the Bluetooth settings screen on the host device (PC, smartphone, etc.) you want to connect to, search for a device name like `ESP32 Keyboard`, and perform pairing.
3.  If pairing is successful, the color of the word `ble` on screen L0 will change from **red to blue**.

Once pairing is complete, the connection status is continuously monitored while powered on. If the connection is lost, it will attempt to reconnect. You can check the current connection status by the color of `ble`.

### Key Input and Host Device Drivers
Characters input from the Cardputer keyboard are sent to the paired host device (PC, smartphone, etc.).
Please use an `English layout keyboard driver` on the host device.

If using a `Japanese keyboard driver`, differences will occur for some characters.
Refer to "9. Links: Using an English Keyboard with a Japanese Driver" for how to handle this.

### Fn Key Functions

By pressing the Fn key and specific keys simultaneously, you can use the following special functions:

*   **CapsLock (Fn + 1)**
    *   Toggles CapsLock status On/Off.
    *   When turned on, the display on screen L3 changes from `unlock` to `lock` (yellow).
*   **Cursor Movement Mode (Fn + 2)**
    *   Toggles cursor movement mode On/Off.
    *   When turned on, the display on screen L3 changes from `off` to `on` (yellow).
    *   When cursor mode is on, specific keys function as arrow keys or navigation keys like Home/End (see "5. Key Mapping" for details).
    *   This setting is remembered even after deep sleep.

Utilizing cursor movement mode makes cursor movement and text editing easier.<br>
For example, **holding down the Shift key while moving the cursor allows you to select characters or lines**, after which you can perform cut, copy, and paste (Ctrl-x/Ctrl-c/Ctrl-v) just like with a normal keyboard.

*   **APO Setting (Fn + 3)**
    *   Sets the Auto Power Off (APO) time. Each press cycles through the settings in the following order:
        *   1 min → 2 min → 3 min → 5 min → 10 min → 15 min → 20 min → 30 min → Off
    *   The set time is displayed on screen L3 (Example: `20min`, `off`).
    *   This setting is saved to NVS (Non-Volatile Storage) and will be retained on the next startup.

## 4. How to Use

### 4.1. Basic Key Input
Pressing a key on the Cardputer sends the corresponding character or HID code via Bluetooth.

### 4.2. Modifier Keys
-   `Ctrl`, `Shift`, `Alt`, `Opt` keys function as modifier keys when pressed simultaneously with other keys.
-   The `Fn` key is used for special functions or navigation key input.

### 4.3. Combinations with Fn Key

-   Pressing the `Fn` key while pressing the following keys sends specific HID codes.
-   These include keys printed in orange above the corresponding key and newly assigned keys.

| Fn + Key | Function         | HID Code      |
| :------- | :--------------- | :------------ |
| `Fn` + `` ` ``  | ESC              | `0x29`        |
| `Fn` + `BACK`| DELETE           | `0x4C`        |
| `Fn` + `5`   | F5               | `0x3E`        |
| `Fn` + `6`   | F6               | `0x3F`        |
| `Fn` + `7`   | F7               | `0x40`        |
| `Fn` + `8`   | F8               | `0x41`        |
| `Fn` + `9`   | F9               | `0x42`        |
| `Fn` + `0`   | F10              | `0x43`        |
| `Fn` + `\`   | Insert           | `0x49`        |
| `Fn` + `'`   | Print Screen     | `0x46`        |
| `Fn` + `;`   | ↑ (Up Arrow)     | `0x52`        |
| `Fn` + `.`   | ↓ (Down Arrow)   | `0x51`        |
| `Fn` + `,`   | ← (Left Arrow)   | `0x50`        |
| `Fn` + `/`   | → (Right Arrow)  | `0x4F`        |
| `Fn` + `-`   | Home             | `0x4A`        |
| `Fn` + `[`   | End              | `0x4D`        |
| `Fn` + `=`   | Page Up          | `0x4B`        |
| `Fn` + `]`   | Page Down        | `0x4E`        |

![Newly Assigned KEYS](images/03a-cardputer-gazo.jpg)<br>
*Newly assigned keys (`fn` + KEY)*

### 4.4. Caps Lock
-   Pressing `Fn` + `1` toggles the Caps Lock state.
-   Screen L3 displays `lock` (enabled) or `unlock` (disabled).

### 4.5. Cursor Movement Mode
-   Pressing `Fn` + `2` toggles the cursor movement mode state.
-   When this mode is enabled (`on`), the following keys function as navigation keys even without pressing the `Fn` key:
    -   `;` -> ↑ (Up Arrow)
    -   `.` -> ↓ (Down Arrow)
    -   `,` -> ← (Left Arrow)
    -   `/` -> → (Right Arrow)
    -   `-` -> Home
    -   `[` -> End
    -   `=` -> Page Up
    -   `]` -> Page Down
-   Screen L3 displays `on` (enabled) or `off` (disabled).

![Assigned keys in cursor movement mode](images/04a-cardputer-gazo.jpg)<br>
*Assigned keys in cursor movement mode*

## 5. Building and Flashing Firmware

There are two ways to flash the firmware:

### 5.1. M5Burner
Using the `M5Burner` software, you can flash firmware from online data via PC.
Install and launch the `M5Burner` software, select `CARDPUTER` on the left side for the device model, choose `tiny-bleKeyboard`, and follow the instructions to flash the firmware.

### 5.2. vsCode + PlatformIO
1.  Set up a development environment with vsCode + PlatformIO on your PC.
2.  Obtain this software from Github and build it.
3.  Connect the Cardputer to your PC and flash the firmware.

The Cardputer may need to be put into **firmware flashing mode** by "pressing and holding the `BtnG0` button while connecting the USB cable, then releasing the `BtnG0` button." Please be aware of this.

## 6. Power Saving Features

### Brightness Reduction
The screen brightness decreases after about 40 seconds of inactivity. Pressing any key restores normal brightness.

### APO (Auto Power Off)
*   If the APO setting is not "off", a warning message `     SLEEP TIME     ` will blink in yellow on screen L4 when the APO set time minus 15 seconds has elapsed since the last key input.
*   If there is no key input for 15 seconds after the warning starts (when the APO set time is reached), the Cardputer will enter deep sleep mode.
*   Just before entering deep sleep, messages like "Entering Sleep" and "Press SPACE key to wakeup..." will be displayed on the screen for about 5 seconds.

### Low Battery Warning
*   If the battery level is 10% or less for three consecutive readings, a warning message `    LOW BATTERY !!  ` will blink in yellow on screen L4. This warning takes precedence over other power-saving processes.
*   If there is no key input for about 30 seconds after the warning starts, the Cardputer will enter deep sleep mode.
*   Just before entering deep sleep, messages like "Entering Sleep" and "Press SPACE key to wakeup..." will be displayed on the screen for about 5 seconds.

### Waking from Deep Sleep
To wake from deep sleep mode, press the `SPACE` key or `BtnG0` on the Cardputer.

## 7. Switching Applications
You can switch and use multiple BIN file applications on an SD card using Launcher software for Cardputer.
The BIN file for this software can be obtained from the BINS folder on GitHub. Operation has been confirmed with the following two types of Launcher software.

### (1) M5Stack-SD-Updater

1.  **Preparation**:
    *   Install M5Stack-SD-Updater compatible software (e.g., this software) on the Cardputer beforehand using M5Burner or vsCode.
    *   Prepare the files from the BINS folder (`menu.bin` and app `bleKeyboard.bin`, etc.).
    *   Copy the BIN files to the root directory of a microSD card.
2.  **Application Switching Procedure**:
    *   With the Cardputer powered off, insert the microSD card.
    *   Turn on the Cardputer while pressing the 'a' key.
    *   `menu.bin` on the SD card (M5Stack-SD-Updater menu screen) will launch. Follow the on-screen instructions to select a file.
    *   The new application will be loaded, and it will automatically restart upon completion.

### (2) M5Launcher Cardputer
Confirmed with M5Launcher Cardputer v2.3.10.

1.  **Preparation**:
    *   Install the `M5Launcher` firmware on the Cardputer beforehand using M5Burner.
    *   Prepare the BIN file from the BINS folder (`bleKeyboard.bin`).
    *   Copy the BIN file to a microSD card.

2.  **Application Switching Procedure**:
    *   With the Cardputer powered off, insert the microSD card.
    *   When M5Launcher starts, press the return key immediately, then select SD on the screen and choose the BIN file for the application.
    *   Select Install to install the new application, and it will automatically restart upon completion.

## 8. Others

### Saving Settings to NVS (Non-Volatile Storage)
The following settings are saved to the Cardputer's internal NVS and are retained even when powered off or entering deep sleep:
*   **APO Setting**: The APO time changed with Fn + 3 is automatically saved.
*   **Cursor Mode Setting**: The On/Off state of the cursor mode changed with Fn + 2 is saved when entering deep sleep and restored on the next startup (when waking from deep sleep). It always starts as off when powered on.

### Introduction to USB Version Software
There is also a USB version software `tiny-usbKeyboard` with the same key layout as this software. Both can be quickly switched and used with Launcher software, which is convenient.

・tiny-usbKeyboard-Cardputer: USB Version Keyboard

## 9. Links

・[tiny-bleKeyboard-Cardputer: GitHub for this software](https://github.com/NoRi-230401/tiny-bleKeyboard-Cardputer)


・[Turning M5Stack Cardputer into a BLE Keyboard: by ji6czd](https://github.com/NoRi-230401/tiny-bleKeyboard-Cardputer)


I was able to complete this software thanks to the answer in ji6czd's article "Can't connect via Bluetooth in the first place?".

・[M5 Cardputer as a USB Keyboard: by Shikarunochi](https://shikarunochi.matrix.jp/?p=5254)

・[Switching programs on M5Cardputer standalone: by Shikarunochi](https://shikarunochi.matrix.jp/?p=5268)

I gained a lot of knowledge about Cardputer from Shikarunochi's articles.

・[Using an English Keyboard with a Japanese Driver: by MAST DESIGN](https://mastdesign.me/20240107-jiskeyboard-uskeyboard/)

Initially, I tried to make it work with both Japanese and English keyboard drivers but gave up. I should have used this from the beginning.

・[HID Key Codes: Onakasuita Wiki](https://wiki.onakasuita.org/pukiwiki/?HID%2F%E3%82%AD%E3%83%BC%E3%82%B3%E3%83%BC%E3%83%89)


・[M5Stack-SD-Updater: by tobozo](https://github.com/tobozo/M5Stack-SD-Updater/)





