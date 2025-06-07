// --------------------------------------------------------
//  *** tiny bleKeyboard ***     by NoRi
//  bluetooth keyboard software for Cardputer
//   2025-06-08  v102
// https://github.com/NoRi-230401/tiny-bleKeyboard-Cardputer
//  MIT License
// --------------------------------------------------------
#include <Arduino.h>
#include <SD.h>
#include <nvs.h>
#include <BleKeyboard.h>
#include <M5Cardputer.h>
#include <M5StackUpdater.h>
#include <esp_sleep.h>     // Added for Deep Sleep
#include <driver/gpio.h>   // Added for gpio_pullup_en
#include <driver/adc.h>    // Added for adc_power_off
#include <WiFi.h>          // Added for WiFi.mode(WIFI_OFF)
#include <driver/rtc_io.h> // Added for rtc_gpio_pullup_en
#include <algorithm>
#include <string> // Added for std::string
#include <map>

void setup();
void loop();
bool checkInput(m5::Keyboard_Class::KeysState &current_keys_state);
bool specialFnMode(const m5::Keyboard_Class::KeysState &current_keys);
void bleSend(const m5::Keyboard_Class::KeysState &current_keys);
void notifyBleConnect();
void powerSaveAndDisp();
void dispLx(uint8_t Lx, const char *msg);
void dispModsKeys(const char *msg);
void dispModsCls();
void dispSendKey(const char *msg);
void dispSendKey2(const char *msg);
void dispFnState();
void dispBleState();
void dispInit();
void m5stack_begin();
void SDU_lobby();
bool SD_begin();
void goDeepSleep();
void fnStateInit();
bool wrtNVS(const char *title, uint8_t data);
bool rdNVS(const char *title, uint8_t &data);
void dispBatteryLevel();

// ----- Cardputer Specific disp paramaters -----------
const int32_t N_COLS = 20; // columns
const int32_t N_ROWS = 6;  // rows
//---- caluculated in m5stackc_begin() ----------------
static int32_t X_WIDTH, Y_HEIGHT;
static int32_t W_CHR, H_CHR;
static int32_t LINE0, LINE1, LINE2, LINE3, LINE4, LINE5;

//-------------------------------------
BleKeyboard bleKey;
KeyReport bleKeyReport;
SPIClass SPI2;
static bool SD_ENABLE;
static bool capsLock; // fn + 1  : Cpas Lock On/Off
static bool cursMode; // fn + 2  : cursor movement mode On/Off
static bool bleConnect = false;

// -- Auto Power Off(APO) --- (fn + 3)  ----
nvs_handle_t nvs;
const char *NVS_SETTING = "setting";
const char *APO_TITLE = "apo";
const char *CURM_TITLE = "curm";
const uint8_t CUSRM_ON = 1;
const uint8_t CURSM_OFF = 0;

// Define a structure to hold APO time and its string representation
struct ApoSetting
{
  int timeMinutes;
  const char *timeString;
};
const ApoSetting apoSettings[] = {
    {1, " 1min"}, {2, " 2min"}, {3, " 3min"}, {5, " 5min"}, {10, "10min"}, {15, "15min"}, {20, "20min"}, {30, "30min"}, {7 * 24 * 60, " off "}};
// if Apo is "off", set 7days time data : that is enough long time

const uint8_t apoTmMax = (sizeof(apoSettings) / sizeof(apoSettings[0])) - 1;
const uint8_t apoTmDefault = 6; // Default APO time: 20min
uint8_t apoTmIndex = apoTmDefault;
unsigned long apoTmout; // ms: auto powerOff(APO) timeout
esp_sleep_wakeup_cause_t wakeup_reason;

// --- control APO and LOW BATTERY  -----------
static unsigned long prev_btlvl_disp_tm = 0L;
static unsigned long prev_warn_disp_tm = 0L;
static bool warnDispFlag = true;
static uint8_t consecutiveLowBatteryCount = 0;
static unsigned long lowBatteryWarnStartTimeMs = 0;
const uint8_t LOW_BATTERY_CONSECUTIVE_READINGS = 3;
const uint8_t LOW_BATTERY_LEVEL_THRESHOLD = 10; // % : define LOW BATTERY lvl

// --- hid key-code define ----
const uint8_t HID_UPARROW = 0x52;
const uint8_t HID_DOWNARROW = 0x51;
const uint8_t HID_LEFTARROW = 0x50;
const uint8_t HID_RIGHTARROW = 0x4F;
const uint8_t HID_ESC = 0x29;
const uint8_t HID_DELETE = 0x4C;
const uint8_t HID_HOME = 0x4A;
const uint8_t HID_END = 0x4D;
const uint8_t HID_PAGEUP = 0x4B;
const uint8_t HID_PAGEDOWN = 0x4E;
const uint8_t HID_INS = 0x49;
const uint8_t HID_PRINTSC = 0x46;
const uint8_t HID_F5 = 0x3E;
const uint8_t HID_F6 = 0x3F;
const uint8_t HID_F7 = 0x40;
const uint8_t HID_F8 = 0x41;
const uint8_t HID_F9 = 0x42;
const uint8_t HID_F10 = 0x43;

// ----- Key Mappings for bleSend() -----
// fnKeyExclusiveMappings: Dedicated mappings evaluated with priority when Fn key is pressed
const std::map<uint8_t, uint8_t> fnKeyExclusiveMappings = {
    {0x35, HID_ESC},    // '`' -> ESC
    {0x2A, HID_DELETE}, // 'BACK' -> DELETE
    {0x22, HID_F5},     // '5' -> F5
    {0x23, HID_F6},     // '6' -> F6
    {0x24, HID_F7},     // '7' -> F7
    {0x25, HID_F8},     // '8' -> F8
    {0x26, HID_F9},     // '9' -> F9
    {0x27, HID_F10},    // '0' -> F10
    {0x31, HID_INS},    // '\' -> Insert
    {0x34, HID_PRINTSC} // ''' -> Print Screen
};

// generalNavigationMappings: Mappings used in combination with Fn key (if not in exclusive) or in cursor mode
const std::map<uint8_t, uint8_t> generalNavigationMappings = {
    {0x33, HID_UPARROW},    // ';' -> Up Arrow
    {0x37, HID_DOWNARROW},  // '.' -> Down Arrow
    {0x36, HID_LEFTARROW},  // ',' -> Left Arrow
    {0x38, HID_RIGHTARROW}, // '/' -> Right Arrow
    {0x2d, HID_HOME},       // '-' -> Home
    {0x2f, HID_END},        // '[' -> End
    {0x2e, HID_PAGEUP},     // '=' -> Page Up
    {0x30, HID_PAGEDOWN}    // ']' -> Page Down
};
// --------------------------------------

unsigned long lastKeyInput = 0;          // last key input time
const uint8_t BRIGHT_NORMAL = 70;        // LCD normal bright level
const uint8_t BRIGHT_LOW = 20;           // LCD low bright level
const uint8_t MAX_SIMULTANEOUS_KEYS = 6; // Max number of keys in HID report (excluding modifier keys)
const int COL_CAPSLOCK = 1;              // "Cap" display start position
const int COL_CURSORMODE = 10;           // "CurM" display start position
const int COL_APO = 15;                  // "Apo" display start position
const int COL_BATVAL = 16;               // Battery value display start position
const int WIDTH_BATVAL_LEN = 3;          // Battery value display length

void setup()
{
  m5stack_begin();

  if (SD_ENABLE)
  { // M5stack-SD-Updater lobby
    SDU_lobby();
    SD.end();
  }

  bleKey.begin();
  fnStateInit(); // function state initialize
  dispInit();    // display initialize
  lastKeyInput = millis();
}

void loop()
{
  M5Cardputer.update(); // update Cardputer key input

  m5::Keyboard_Class::KeysState current_keys_state;
  if (checkInput(current_keys_state))       // check Cardputer key input and get state
  {                                         // if keys input
    if (!specialFnMode(current_keys_state)) // special function mode check
      bleSend(current_keys_state);          // send data via bluetooth
  }

  notifyBleConnect(); // check bluetooth connection
  powerSaveAndDisp(); // bat.status disp and power save
  delay(5);
}

bool checkInput(m5::Keyboard_Class::KeysState &current_keys_state)
{ // check Cardputer key input
  if (M5Cardputer.Keyboard.isChange())
  {
    lastKeyInput = millis();
    if (M5Cardputer.Keyboard.isPressed())
    {
      current_keys_state = M5Cardputer.Keyboard.keysState();
      return true;
    }
    else // All keys are physically released
    {
      bleKey.releaseAll();
      dispModsCls();
    }
  }

  if (M5Cardputer.BtnA.wasPressed())
  { // BtnG0 : instead name of Cardputer BtnA
    lastKeyInput = millis();
  }
  return false;
}

bool specialFnMode(const m5::Keyboard_Class::KeysState &current_keys)
{ // **** [fn] special function mode (High Priority) **********
  // return true: special function executed , false: not exectuted

  //  --- input keys status setup  -------------------------
  uint8_t mods = current_keys.modifiers;
  bool existWord = !current_keys.word.empty();
  uint8_t keyWord = 0;
  if (existWord)
    keyWord = current_keys.word[0];

  if (current_keys.fn && existWord)
  {
    bool specialFnProcessed = false;
    switch (keyWord)
    {
    case '1':
    case '!': // Caps Lock Toggle (fn + '1')
      specialFnProcessed = true;
      capsLock = !capsLock;
      bleKey.write(KEY_CAPS_LOCK);
      break;
    case '2':
    case '@': // Cursor movement Mode Toggle (fn + '2')
      specialFnProcessed = true;
      cursMode = !cursMode;
      break;
    case '3':
    case '#': // APO(Auto PowerOff) time setting (fn + '3')
      specialFnProcessed = true;
      apoTmIndex = (apoTmIndex < apoTmMax) ? apoTmIndex + 1 : 0;
      apoTmout = static_cast<unsigned long>(apoSettings[apoTmIndex].timeMinutes) * 60 * 1000L;
      wrtNVS(APO_TITLE, apoTmIndex);
#ifdef DEBUG
      Serial.print("autoPowerOff: ");
      Serial.println(apoSettings[apoTmIndex].timeString);
      Serial.print("apoTmout: ");
      Serial.println(apoTmout, DEC);
#endif
      break;
    }

    if (specialFnProcessed)
    {
      dispFnState();
      bleKey.releaseAll(); // Release all keys/modifiers on the host side
      dispModsCls();       // Clear modifier key display on the Cardputer side
      return true;
    }
  }
  return false;
}

void bleSend(const m5::Keyboard_Class::KeysState &current_keys)
{
  bleKeyReport = {0};
  String modsStr = "";
  uint8_t modifier = 0;
  String localSendWord = ""; // Use local variable for constructing HID code string

  // ** modifiers keys(ctrl,shift,alt) and fn **
  //    these keys are used with other key
  if (current_keys.ctrl)
  {
    modifier |= 0x01;
    modsStr += "Ctrl ";
  }
  if (current_keys.shift)
  {
    modifier |= 0x02;
    modsStr += "Shift ";
  }
  if (current_keys.alt)
  {
    modifier |= 0x04;
    modsStr += "Alt ";
  }
  if (current_keys.opt)
  {
    modifier |= 0x08;
    modsStr += "Opt ";
  }
  if (current_keys.fn)
    modsStr += "Fn ";

  bleKeyReport.modifiers = modifier;
  dispModsKeys(modsStr.c_str());

  // *****  Regular Character Keys *****
  int count = 0;
  for (auto hidCode : current_keys.hid_keys)
  {
    char tempBuf[8]; // For " 0xYY" format
    if (count < MAX_SIMULTANEOUS_KEYS)
    {
      uint8_t finalHidCode = hidCode;

      if (current_keys.fn)
      {
        auto it_exclusive = fnKeyExclusiveMappings.find(hidCode);
        if (it_exclusive != fnKeyExclusiveMappings.end())
        {
          finalHidCode = it_exclusive->second; // Apply Fn-specific mapping
        }
        else
        {
          // If Fn key is pressed but no dedicated mapping is found, try general navigation mapping
          auto it_general = generalNavigationMappings.find(hidCode);
          if (it_general != generalNavigationMappings.end())
          {
            finalHidCode = it_general->second;
          }
        }
      }
      else if (cursMode) // If Fn key is not pressed and cursor mode is enabled, try general navigation mapping
      {
        auto it_general = generalNavigationMappings.find(hidCode);
        if (it_general != generalNavigationMappings.end())
        {
          finalHidCode = it_general->second;
        }
      }
      bleKeyReport.keys[count] = finalHidCode;
      snprintf(tempBuf, sizeof(tempBuf), " 0x%02X", finalHidCode);
      localSendWord += tempBuf;
      count++;
    }
  }

  // Send keyReport via bluetooth
  bleKey.sendReport(&bleKeyReport);

  String keysDisplayString = "";
  if (!current_keys.word.empty())
  {
    keysDisplayString += " ";
    keysDisplayString += current_keys.word[0];
    keysDisplayString += " :hid";
  }
  else if (!localSendWord.isEmpty())
  {
    keysDisplayString += " hid";
  }
  keysDisplayString += localSendWord;

  if (!keysDisplayString.isEmpty())
  {
    dispSendKey(keysDisplayString.c_str());
  }
}

static unsigned long PREV_BLECHK_TM = 0;
void notifyBleConnect()
{
  const unsigned long BLE_CONNECT_CHECK_INTERVAL_MS = 1000UL;
  if (millis() < PREV_BLECHK_TM + BLE_CONNECT_CHECK_INTERVAL_MS)
    return;

  PREV_BLECHK_TM = millis();

  if (bleKey.isConnected() && !bleConnect)
  {
    bleConnect = true;
    dispBleState();
  }
  else if (!bleKey.isConnected() && bleConnect)
  {
    bleConnect = false;
    dispBleState();
  }
}

void dispLx(uint8_t Lx, const char *msg)
{
  //*********** Lx is (0 to N_ROWS - 1) *************************
  // -----01234567890123456789--------------------------
  // L0  "- tiny bleKeyborad -" : title and ble status
  // L1               bat.100%  : battery Status
  // L2   fn1:Cap 2:CurM 3:Apo  : fn
  // L3    unlock   off  30min  :
  // L4   Shift Ctrl Alt Opt    : modifiers keys
  // L5    X :hid 0x00          : sendKey and hidCode
  // -----01234567890123456789---------------------------
  // ****************************************************
  if (Lx >= N_ROWS)
    return;

  M5Cardputer.Display.fillRect(0, Lx * H_CHR, X_WIDTH, H_CHR, TFT_BLACK);
  M5Cardputer.Display.setCursor(0, Lx * H_CHR);
  M5Cardputer.Display.print(msg);
}

void dispModsKeys(const char *msg)
{ // line4 : modifiers keys(Shift/Ctrl/Alt/Opt/Fn)
  dispLx(4, msg);
}

void dispModsCls()
{ // line4 : modifiers keys disp clear
  // dispLx(4, "");
  M5Cardputer.Display.fillRect(0, LINE4, X_WIDTH, H_CHR, TFT_BLACK);
}

void dispSendKey(const char *msg)
{ // line5 : reguler character send info
  M5Cardputer.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  dispLx(5, msg);
#ifdef DEBUG
  Serial.println(msg); // msg is already const char*
#endif
}

void dispSendKey2(const char *msg)
{ // line5 : other keys (enter,tab,backspace, etc ...) send info
  M5Cardputer.Display.setTextColor(TFT_YELLOW, TFT_BLACK);
  char buffer[N_COLS + 1]; // Ensure buffer is large enough
  snprintf(buffer, sizeof(buffer), " %s", msg);
  dispLx(5, buffer);
#ifdef DEBUG
  Serial.println(msg); // msg is already const char*
#endif
}

void dispFnState()
{ // Line3 : fn1 to fn3 state display
  //-------- 01234567890123456789---
  //_____L2 "fn1:Cap 2:CurM 3:Apo"__
  //_____L3_" unlock   off  30min"__
  //-------- 01234567890123456789---
  const char *StCaps[] = {"unlock", " lock"};
  const char *StEditMode[] = {"off", " on"};

  M5Cardputer.Display.fillRect(0, LINE3, X_WIDTH, H_CHR, TFT_BLACK);

  // capsLock state
  M5Cardputer.Display.setCursor(W_CHR * COL_CAPSLOCK, LINE3);
  if (capsLock)
  {
    M5Cardputer.Display.setTextColor(TFT_YELLOW, TFT_BLACK);
    M5Cardputer.Display.print(StCaps[1]);
  }
  else
  {
    M5Cardputer.Display.setTextColor(TFT_WHITE, TFT_BLACK);
    M5Cardputer.Display.print(StCaps[0]);
  }

  // Cursor movement mode state
  M5Cardputer.Display.setCursor(W_CHR * COL_CURSORMODE, LINE3);
  if (cursMode)
  {
    M5Cardputer.Display.setTextColor(TFT_YELLOW, TFT_BLACK);
    M5Cardputer.Display.print(StEditMode[1]);
  }
  else
  {
    M5Cardputer.Display.setTextColor(TFT_WHITE, TFT_BLACK);
    M5Cardputer.Display.print(StEditMode[0]);
  }

  // Auto PowerOff time
  M5Cardputer.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5Cardputer.Display.setCursor(W_CHR * COL_APO, LINE3);
  M5Cardputer.Display.print(apoSettings[apoTmIndex].timeString);
}

void dispBleState()
{ // line0: software name and BLE connect information
  //----01234567890123456789--
  // L0_"- tiny bleKeyboard -"-
  //----01234567890123456789--

  M5Cardputer.Display.fillRect(0, LINE0, X_WIDTH, H_CHR, TFT_BLACK);
  M5Cardputer.Display.setCursor(0, LINE0);
  M5Cardputer.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5Cardputer.Display.print("- tiny ");

  if (bleConnect)
    M5Cardputer.Display.setTextColor(TFT_BLUE, TFT_BLACK);
  else
    M5Cardputer.Display.setTextColor(TFT_RED, TFT_BLACK);
  M5Cardputer.Display.print("ble");

  M5Cardputer.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5Cardputer.Display.print("Keyboard -");
}

void dispInit()
{
  M5Cardputer.Display.fillScreen(TFT_BLACK);
  M5Cardputer.Display.setBrightness(BRIGHT_NORMAL);
  M5Cardputer.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5Cardputer.Display.setCursor(0, 0);

  //  L0 : software title and BLE connect inf -----------------
  dispBleState();

  //  L1 :Battery Level ---------------------------------------
  //-------------------"01234567890123456789"------------------;
  const char *L1Str = "            bat.   %";
  //-------------------"01234567890123456789"------------------;
  M5Cardputer.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  dispLx(1, L1Str); // L1Str is now const char*
  dispBatteryLevel();

  //  L2 : Fn1 to Fn3 title -----------------------------------
  //-------------------"01234567890123456789"------------------;
  //           L2Str = "fn1:Cap 2:CurM 3:Apo";
  //-------------------"01234567890123456789"------------------;
  M5Cardputer.Display.setCursor(0, LINE2);
  M5Cardputer.Display.setTextColor(TFT_ORANGE, TFT_BLACK);
  M5Cardputer.Display.print("fn1:");
  M5Cardputer.Display.setTextColor(TFT_GREEN, TFT_BLACK);
  M5Cardputer.Display.print("Cap ");

  M5Cardputer.Display.setTextColor(TFT_ORANGE, TFT_BLACK);
  M5Cardputer.Display.print("2:");
  M5Cardputer.Display.setTextColor(TFT_GREEN, TFT_BLACK);
  M5Cardputer.Display.print("CurM ");

  M5Cardputer.Display.setTextColor(TFT_ORANGE, TFT_BLACK);
  M5Cardputer.Display.print("3:");
  M5Cardputer.Display.setTextColor(TFT_GREEN, TFT_BLACK);
  M5Cardputer.Display.print("Apo");

  // L3 : Fn1 to Fn3 state disp --------------------------------
  dispFnState();
}

void m5stack_begin()
{
  auto cfg = M5.config();
  cfg.serial_baudrate = 115200; // Serial communication speed
  cfg.internal_imu = false;     // Do not use IMU (accelerometer/gyroscope)
  cfg.internal_mic = false;     // Do not use microphone
  cfg.output_power = false;     // Disable Grove port power output
  cfg.led_brightness = 0;
  M5Cardputer.begin(cfg, true);
  M5Cardputer.Speaker.setVolume(0);
  WiFi.mode(WIFI_OFF); // Ensure Wi-Fi is off

  // --- Wakeup reason check ---
  wakeup_reason = esp_sleep_get_wakeup_cause();

#ifdef DEBUG
  // vsCode terminal cannot get serial data
  //  of cardputer before 5 sec ...!
  delay(5000);
  Serial.println("\n\n*** m5stack begin ***");
  // -----------------------------------------

  // ----- Wakeup reason ---------
  Serial.print("Wakeup cause: ");
  switch (wakeup_reason)
  {
  case ESP_SLEEP_WAKEUP_EXT0:
    Serial.println("External signal using RTC_IO (EXT0) - Likely BtnA (GPIO0)");
    break;
  case ESP_SLEEP_WAKEUP_EXT1:
    Serial.println("External signal using RTC_CNTL (EXT1) - Likely Keyboard Matrix Key");
    {
      uint64_t wakeup_pin_mask = esp_sleep_get_ext1_wakeup_status();
      Serial.printf("EXT1 wakeup triggered by pin mask: 0x%llX\n", wakeup_pin_mask);
      const int rtc_pins_for_wakeup[] = {13, 15, 3, 4, 5, 6, 7}; // Defined in goDeepSleep
      bool found_trigger_pin = false;
      for (int pin : rtc_pins_for_wakeup)
      {
        if ((wakeup_pin_mask >> pin) & 1)
        {
          Serial.printf("Wakeup detected on RTC GPIO %d\n", pin);
          found_trigger_pin = true;
        }
      }
      if (!found_trigger_pin)
        Serial.println("EXT1 wakeup, but no specific pin identified from the configured mask (check mask definition).");
    }
    break;
  case ESP_SLEEP_WAKEUP_TIMER:
    Serial.println("Timer");
    break;
  case ESP_SLEEP_WAKEUP_TOUCHPAD:
    Serial.println("Touchpad");
    break;
  case ESP_SLEEP_WAKEUP_ULP:
    Serial.println("ULP program");
    break;
  default:
    Serial.printf("Not caused by deep sleep: %d\n", wakeup_reason);
    break;
  }
  // --- End of wakeup reason ---
#endif

  // Reduce power consumption by lowering CPU frequency (e.g., 80MHz)
  if (setCpuFrequencyMhz(80))
  {
#ifdef DEBUG
    Serial.printf("CPU Freq set to: %d MHz\n", getCpuFrequencyMhz());
#endif
  }

  // Calculate Cardputer specific display scale parameters
  X_WIDTH = M5Cardputer.Display.width();
  Y_HEIGHT = M5Cardputer.Display.height();
  W_CHR = X_WIDTH / N_COLS;  // width of 1 character
  H_CHR = Y_HEIGHT / N_ROWS; // height of 1 character
  LINE0 = 0 * H_CHR;
  LINE1 = 1 * H_CHR;
  LINE2 = 2 * H_CHR;
  LINE3 = 3 * H_CHR;
  LINE4 = 4 * H_CHR;
  LINE5 = 5 * H_CHR;

  // display setup at startup
  M5Cardputer.Display.fillScreen(TFT_BLACK);
  M5Cardputer.Display.setBrightness(BRIGHT_NORMAL);
  M5Cardputer.Display.setRotation(1);
  M5Cardputer.Display.setFont(&fonts::Font0);
  M5Cardputer.Display.setTextSize(2);
  M5Cardputer.Display.setTextDatum(top_left); // character base position
  M5Cardputer.Display.setTextWrap(false);
  M5Cardputer.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5Cardputer.Display.setCursor(0, 0);

  // SPI setup for using SD card
  SPI2.begin(
      M5.getPin(m5::pin_name_t::sd_spi_sclk),
      M5.getPin(m5::pin_name_t::sd_spi_miso),
      M5.getPin(m5::pin_name_t::sd_spi_mosi),
      M5.getPin(m5::pin_name_t::sd_spi_ss));

  SD_ENABLE = SD_begin();
}

// ------------------------------------------------------------------------
// SDU_lobby :  lobby for SD-Updater
// ------------------------------------------------------------------------
// load '/menu.bin' on SD, if key'a' pressed at booting.
// 'menu.bin' for Cardputer is involved in BINS folder at this github site
// https://github.com/NoRi-230401/tiny-bleKeyboard-Cardputer
//
// ------------------------------------------------------------------------
// ** M5Stack-SD-Updater is launcher software for m5stak by tobozo **
// https://github.com/tobozo/M5Stack-SD-Updater/
// ------------------------------------------------------------------------
void SDU_lobby()
{
  M5Cardputer.update();
  if (M5Cardputer.Keyboard.isKeyPressed('a'))
  {
    updateFromFS(SD, "/menu.bin");
    ESP.restart();
    // *** NEVER RETURN ***
  }
}

bool SD_begin()
{
  int i = 0;
  while (!SD.begin(M5.getPin(m5::pin_name_t::sd_spi_ss), SPI2) && i < 10)
  {
    delay(500);
    i++;
  }
  if (i >= 10)
  {
    Serial.println("ERR: SD begin fail...");
    SD.end();
    return false;
  }
  return true;
}

void goDeepSleep()
{
#ifdef DEBUG
  Serial.println(" *** Preparing for Deep Sleep ***");
#endif
  // disp message at LCD for 5sec befor entering deepSleep
  //--------------------------------------------------------------------
  // ---01234567890123456789--
  // L1:   Entering Sleep
  // L2:
  // L3: Press  SPACE  key
  // L4:     to wakeup...
  // ---01234567890123456789--
  M5Cardputer.Display.fillScreen(TFT_BLACK); // clear screen
  M5Cardputer.Display.setTextColor(TFT_RED, TFT_BLACK);
  M5Cardputer.Display.setCursor(W_CHR * 3, LINE1);
  M5Cardputer.Display.print("Entering Sleep");

  M5Cardputer.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5Cardputer.Display.setCursor(W_CHR * 1, LINE3);
  M5Cardputer.Display.print("Press");
  M5Cardputer.Display.setTextColor(TFT_YELLOW, TFT_BLACK);
  M5Cardputer.Display.print("  SPACE  ");
  M5Cardputer.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5Cardputer.Display.print("key");

  M5Cardputer.Display.setCursor(W_CHR * 5, LINE4);
  M5Cardputer.Display.print("to wakeup...");

#ifdef DEBUG
  Serial.println("Entering Deep Sleep now. Wake up by pressing any key.");
  Serial.flush();
#endif

  // Save current cursor mode and sleep status to NVS
  if (cursMode)
    wrtNVS(CURM_TITLE, CUSRM_ON);
  else
    wrtNVS(CURM_TITLE, CURSM_OFF);

  delay(5000); // Allow time for display messages and serial flush
               // -----------------------------------------------------------------------

  // Shutdown peripherals before sleep
  M5Cardputer.Display.setBrightness(0);
  M5Cardputer.Display.powerSaveOn();
  M5Cardputer.Speaker.end();

  // Configure 74HC138 select pins (A0, A1, A2) - these are non-RTC GPIOs
  const int output_pins_to_configure[] = {8, 9, 11};
#ifdef DEBUG
  Serial.println("Configuring 74HC138 select pins (GPIO 8, 9, 11) to OUTPUT LOW for sleep...");
#endif
  for (int pin : output_pins_to_configure)
  {
    gpio_set_direction((gpio_num_t)pin, GPIO_MODE_OUTPUT);
    gpio_set_level((gpio_num_t)pin, 0); // Set A0, A1, A2 to LOW -> Y0 output of 74HC138 will be active LOW
#ifdef DEBUG
    Serial.printf("GPIO %d (74HC138 select) set to OUTPUT LOW. Y0 will be active LOW.\n", pin);
#endif
  }

  // Keep power on for VDD_SDIO (for non-RTC GPIOs 8,9,11) and RTC_PERIPH (for EXT1 wakeup) during Deep Sleep
  esp_err_t pd_err_vddsdio = esp_sleep_pd_config(ESP_PD_DOMAIN_VDDSDIO, ESP_PD_OPTION_ON);
  esp_err_t pd_err_rtc_periph = esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);
#ifdef DEBUG
  Serial.printf("Power domain VDD_SDIO set to ON: %s\n", esp_err_to_name(pd_err_vddsdio));
  Serial.printf("Power domain RTC_PERIPH set to ON: %s\n", esp_err_to_name(pd_err_rtc_periph));
#endif

  // Configure EXT1 wakeup using specified RTC GPIO pins
  // These pins are connected to the keyboard matrix rows (via 74HC138 outputs)
  const uint64_t ext1_wakeup_pin_mask = (1ULL << 13) | (1ULL << 15) | (1ULL << 3) |
                                        (1ULL << 4) | (1ULL << 5) | (1ULL << 6) |
                                        (1ULL << 7);

#ifdef DEBUG
  Serial.printf("Enabling ext1 wakeup with mask: 0x%llX\n", ext1_wakeup_pin_mask);
#endif

  // Configure each RTC GPIO pin for EXT1 wakeup:
  // - Enable internal pull-up resistor.
  // - Disable internal pull-down resistor.
  // - Disable hold feature (to allow pin state to change and trigger wakeup).
  const int rtc_pins_to_configure[] = {13, 15, 3, 4, 5, 6, 7};
  for (int pin : rtc_pins_to_configure) // These are the keyboard matrix row sense lines (Y0-Y6 of 74HC138)
  {
    if (rtc_gpio_is_valid_gpio((gpio_num_t)pin))
    {
      rtc_gpio_pullup_en((gpio_num_t)pin);
      rtc_gpio_pulldown_dis((gpio_num_t)pin);
      rtc_gpio_hold_dis((gpio_num_t)pin);
#ifdef DEBUG
      Serial.printf("RTC GPIO %d pull-up enabled, hold disabled for deep sleep.\n", pin);
#endif
    }
    else
    {
      ;
#ifdef DEBUG
      Serial.printf("Warning: GPIO %d is not a valid RTC GPIO, skipping configuration for sleep.\n", pin);
#endif
    }
  }

#ifdef DEBUG
  // Final check of RTC GPIO levels for EXT1 wakeup (just before esp_deep_sleep_start)
  Serial.println("Final check of RTC GPIO levels before sleep:");
  for (int pin : rtc_pins_to_configure)
  {
    if (rtc_gpio_is_valid_gpio((gpio_num_t)pin))
    {
      int level = gpio_get_level((gpio_num_t)pin);

      Serial.printf("Final RTC GPIO %d level: %d (1=HIGH, 0=LOW)\n", pin, level);
      if (level == 0)
      {
        Serial.printf("WARNING: RTC GPIO %d is LOW just before sleep!\n", pin);
      }
    }
  }
#endif

  // Enable EXT1 wakeup. Wake up when any of the selected RTC GPIOs go LOW.
  esp_err_t err_ext1 = esp_sleep_enable_ext1_wakeup(ext1_wakeup_pin_mask, ESP_EXT1_WAKEUP_ANY_LOW);

#ifdef DEBUG
  if (err_ext1 != ESP_OK)
  {
    Serial.printf("Failed to enable ext1 wakeup: %s\n", esp_err_to_name(err_ext1));
  }
  else
  {
    Serial.println("ext1 wakeup enabled on keyboard input pins (ANY_LOW).");
  }

  // --- Configure EXT0 wakeup for BtnA (GPIO0) ---
  Serial.println("Configuring BtnA (GPIO0) for EXT0 wakeup (LOW level)...");
#endif

  if (rtc_gpio_is_valid_gpio(GPIO_NUM_0)) // Check if GPIO0 is an RTC GPIO
  {
    rtc_gpio_pullup_en(GPIO_NUM_0);
    rtc_gpio_pulldown_dis(GPIO_NUM_0);
    rtc_gpio_hold_dis(GPIO_NUM_0);                                    // Also disable hold
    esp_err_t err_ext0 = esp_sleep_enable_ext0_wakeup(GPIO_NUM_0, 0); // 0 = Wake up on LOW level

#ifdef DEBUG
    if (err_ext0 != ESP_OK)
    {
      Serial.printf("Failed to enable ext0 wakeup: %s\n", esp_err_to_name(err_ext0));
    }
    else
    {
      Serial.println("EXT0 wakeup enabled on BtnA (GPIO0).");
    }
#endif
  }

#ifdef DEBUG
  else
  {
    Serial.println("Error: GPIO0 is not an RTC GPIO, cannot use for EXT0 wakeup in this mode.");
  }

  Serial.println("Starting deep sleep now...");
  Serial.flush(); // Ensure all serial messages are sent
  delay(100);     // Short delay for serial transmission completion
#endif

  // *** Enter deep sleep ***
  esp_deep_sleep_start();
  // **** NEVER RETURN *****
}

void fnStateInit()
{
  // capsLock state is always 'false' start
  capsLock = false;

  // cursor movement mode state is recovered if waking up from DeepSleep
  cursMode = false;
  if (wakeup_reason == ESP_SLEEP_WAKEUP_EXT1 || wakeup_reason == ESP_SLEEP_WAKEUP_EXT0)
  { // Read NVS data if waking up from DeepSleep
#ifdef DEBUG
    Serial.println("Wakeup from DeepSleep");
#endif
    uint8_t nvmData;
    if (rdNVS(CURM_TITLE, nvmData))
    {
      if (nvmData == CUSRM_ON)
        cursMode = true;
    }
  }
  else // PowerOn: did not wake up from DeepSleep
  {
#ifdef DEBUG
    Serial.println("POWER ON");
#endif
    ;
  }
  // -----------------------------------------------------

  // Auto PowerOff state is always restored from NVS at start
  apoTmIndex = apoTmDefault;
  uint8_t nvmData;
  if (rdNVS(APO_TITLE, nvmData))
    apoTmIndex = nvmData;
  if (apoTmIndex > apoTmMax)
    apoTmIndex = apoTmDefault;

  apoTmout = static_cast<unsigned long>(apoSettings[apoTmIndex].timeMinutes) * 60 * 1000L;
  wrtNVS(APO_TITLE, apoTmIndex);
#ifdef DEBUG
  Serial.print("autoPowerOff: ");
  Serial.println(apoSettings[apoTmIndex].timeString);
  Serial.print("apoTmout: ");
  Serial.println(apoTmout, DEC);
#endif
}

bool wrtNVS(const char *title, uint8_t data)
{
  if (ESP_OK == nvs_open(NVS_SETTING, NVS_READWRITE, &nvs))
  {
    nvs_set_u8(nvs, title, data); // Use title directly
    nvs_close(nvs);
    return true;
  }
  return false;
}

bool rdNVS(const char *title, uint8_t &data)
{
  if (ESP_OK == nvs_open(NVS_SETTING, NVS_READONLY, &nvs))
  {
    nvs_get_u8(nvs, title, &data); // Use title directly
    nvs_close(nvs);
    return true;
  }
  return false;
}

enum PowerSaveFSM
{
  PS_NORMAL,
  PS_LOW_BRIGHT,
  PS_WARN_APO,
  PS_LOW_BATTERY_WARN
};
static PowerSaveFSM psState = PS_NORMAL;

void powerSaveAndDisp()
{
  const unsigned long BATLVL_DISP_INTERVAL_MS = 5 * 1000UL;
  const unsigned long LOW_BATTERY_WARN_BLINK_DURATION_MS = 30 * 1000UL;

  const unsigned long LOW_BRIGHT_TIMEOUT_MS = 40 * 1000UL;
  const unsigned long APO_WARN_PERIOD_MS = 15 * 1000UL;
  const unsigned long WARN_BLINK_INTERVAL_MS = 500UL;
  unsigned long currentTime = millis(); // Get current time once
  unsigned long timeSinceLastInput = currentTime - lastKeyInput;

  if (currentTime > prev_btlvl_disp_tm + BATLVL_DISP_INTERVAL_MS)
  {
    prev_btlvl_disp_tm = currentTime;
    dispBatteryLevel(); // This will update consecutiveLowBatteryCount
  }

  // Check for Low Battery condition first (highest priority)
  if (consecutiveLowBatteryCount >= LOW_BATTERY_CONSECUTIVE_READINGS)
  {
    if (psState != PS_LOW_BATTERY_WARN)
    {
      psState = PS_LOW_BATTERY_WARN;
      lowBatteryWarnStartTimeMs = currentTime;
      warnDispFlag = true;                              // To show message first
      M5Cardputer.Display.setBrightness(BRIGHT_NORMAL); // Ensure warning is visible
      M5Cardputer.Display.setTextColor(TFT_YELLOW, TFT_BLACK);
      // -------"01234567890123456789"--
      dispLx(4, "    LOW BATTERY !!  ");
      // -------"01234567890123456789"--
      prev_warn_disp_tm = currentTime; // Reset blink timer for this new warning
    }

    // Handle blinking and eventual deep sleep for low battery
    if (currentTime >= lowBatteryWarnStartTimeMs + LOW_BATTERY_WARN_BLINK_DURATION_MS)
    {
      goDeepSleep(); // Enter deep sleep after warning duration
    }
    else
    {
      // Blinking logic for "LOW BATTERY !!"
      if (currentTime >= prev_warn_disp_tm + WARN_BLINK_INTERVAL_MS)
      {
        prev_warn_disp_tm = currentTime;
        warnDispFlag = !warnDispFlag;
        M5Cardputer.Display.setTextColor(TFT_YELLOW, TFT_BLACK);
        dispLx(4, warnDispFlag ? "    LOW BATTERY !!  " : "");
      }
    }
    return; // Low battery handling takes precedence over other power saving modes
  }

  if (timeSinceLastInput < LOW_BRIGHT_TIMEOUT_MS)
  { // Still within normal active period
    if (psState != PS_NORMAL)
    {
      // If returning to normal from any other state (low bright, APO warn)
      psState = PS_NORMAL;
      dispInit(); // Restore full display, normal brightness, and clear any previous warnings
    }
    return;
  }

  // LOW_BRIGHT_TIMEOUT_MS has passed
  if (apoTmIndex == apoTmMax) // APO is "off"
  {
    // --- If APO is "off",  APO is disable  ---
    // no power save function, not need dimming,warining,sleeping
    return;
  }

  // APO is enabled, and LOW_BRIGHT_TIMEOUT_MS has passed.
  // Proceed with dimming, warning, and sleeping logic.
  if (psState == PS_NORMAL)
  {
    // Transition from Normal to Low Bright as LOW_BRIGHT_TIMEOUT_MS has passed and APO is on
    psState = PS_LOW_BRIGHT;
    M5Cardputer.Display.setBrightness(BRIGHT_LOW);
  }

  if (psState == PS_LOW_BRIGHT)
  {
    // Check if it's time to transition to warning state
    if (timeSinceLastInput >= (apoTmout - APO_WARN_PERIOD_MS))
    {
      psState = PS_WARN_APO;
      prev_warn_disp_tm = currentTime;                  // Initialize for blinking
      warnDispFlag = true;                              // Show warning message first
      M5Cardputer.Display.setBrightness(BRIGHT_NORMAL); // Restore brightness for warning
      // Display initial warning message
      M5Cardputer.Display.setTextColor(TFT_YELLOW, TFT_BLACK);
      dispLx(4, "     SLEEP TIME     ");
    }
  }

  if (psState == PS_WARN_APO)
  {
    if (timeSinceLastInput >= apoTmout)
    { // APO timeout reached
      goDeepSleep();
      // esp_deep_sleep_start() does not return
    }
    else
    {
      // Still in warning period, blink the message
      if (currentTime >= prev_warn_disp_tm + WARN_BLINK_INTERVAL_MS)
      {
        prev_warn_disp_tm = currentTime;
        warnDispFlag = !warnDispFlag;
        M5Cardputer.Display.setTextColor(TFT_YELLOW, TFT_BLACK);
        dispLx(4, warnDispFlag ? "     SLEEP TIME     " : "");
      }
    }
  }
}

void dispBatteryLevel()
{

  // Line1 : battery level display
  //---- 01234567890123456789---
  // L1_"            bat.100%"--
  //---- 01234567890123456789---
  M5Cardputer.Display.fillRect(W_CHR * COL_BATVAL, LINE1, W_CHR * WIDTH_BATVAL_LEN, H_CHR, TFT_BLACK);
  M5Cardputer.Display.setCursor(W_CHR * COL_BATVAL, LINE1);
  M5Cardputer.Display.setTextColor(TFT_WHITE, TFT_BLACK);

  uint8_t batLvl = (uint8_t)M5.Power.getBatteryLevel();  // Get battery level
  char batLvlBuf[4];                                     // Buffer for "XXX" + null terminator
  snprintf(batLvlBuf, sizeof(batLvlBuf), "%3u", batLvl); // %3u pads with spaces if less than 3 digits
  M5Cardputer.Display.print(batLvlBuf);

  bleKey.setBatteryLevel(batLvl); // send battery level via Bluetooth

  // Update consecutive low battery count
  if (batLvl <= LOW_BATTERY_LEVEL_THRESHOLD)
  {
    if (consecutiveLowBatteryCount < LOW_BATTERY_CONSECUTIVE_READINGS)
    { // Avoid overflow if already at max
      consecutiveLowBatteryCount++;
    }
  }
  else
  {
    consecutiveLowBatteryCount = 0; // Reset if battery level is acceptable
  }
}
