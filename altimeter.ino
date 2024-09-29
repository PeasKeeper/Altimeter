#include <Bonezegei_LCD1602_I2C.h>
#include <Adafruit_BMP280.h>
#include <EventManager.h>
#include <Encoder.h>
#include <EEPROM.h>

Bonezegei_LCD1602_I2C lcd(0x27);
Encoder myEnc(2, 3);

#define BTN_ENC (4)
#define BTN_ZERO (5)
#define BMP_SCK  (13)
#define BMP_MISO (12)
#define BMP_MOSI (11)
#define BMP_CS   (10)
#define UPDATE_READING (1)
#define BUTTON_PRESSED (2)
#define ZERO (3)
#define EEPROM_ATMOSPHERE (0)
#define EEPROM_ZEROMODE (sizeof(int))
#define EEPROM_ZEROALTITUDE (sizeof(int) + sizeof(bool))

Adafruit_BMP280 bmp(BMP_CS, BMP_MOSI, BMP_MISO,  BMP_SCK);

EventManager manager;

int oldPosition = 0;
int oldBtn = 1;
int oldBtnZero = 1;
bool editMode = false;
bool zeroMode = false;
int atmosphere;
float currentAltitude = 0;
float zeroAltitude;

void setup() {
  Serial.begin(9600);
  //bmp280
  bmp.begin();
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,
                Adafruit_BMP280::SAMPLING_X2,
                Adafruit_BMP280::SAMPLING_X16,
                Adafruit_BMP280::FILTER_X16,
                Adafruit_BMP280::STANDBY_MS_500);
  //encoder
  pinMode (BTN_ENC, INPUT_PULLUP);
  //button
  pinMode (BTN_ZERO, INPUT_PULLUP);
  //taskmanager
  manager.pushEvent(UPDATE_READING, 0);
  //lcd
  lcd.begin();
  lcd.print("Altimeter  mk.1");
  
  EEPROM.get(EEPROM_ATMOSPHERE, atmosphere);
  EEPROM.get(EEPROM_ZEROMODE, zeroMode);
  EEPROM.get(EEPROM_ZEROALTITUDE, zeroAltitude);
  //for first initialization
  if (atmosphere > 850 || atmosphere < 600) {
    atmosphere = 750;
  }
}

void loop() {
  String data;
  int newPosition, pressure, atmosphericPressure;
  int eventCode = manager.popEvent(false);
  int currentBtn, currentBtnZero;

  currentBtn = digitalRead(BTN_ENC);
  if (currentBtn != oldBtn) {
    oldBtn = currentBtn;
    manager.pushEvent(BUTTON_PRESSED, 100);
  }

  currentBtnZero = digitalRead(BTN_ZERO);
  if (currentBtnZero != oldBtnZero) {
    oldBtnZero = currentBtnZero;
    manager.pushEvent(ZERO, 100);
  }

  switch (eventCode) {

    case UPDATE_READING:
      lcd.setPosition(0, 1);
      data = "*C";
      data += bmp.readTemperature();
      data += " ";
      lcd.print(data.c_str());
      data = "m";
      if (!zeroMode) {
        zeroAltitude = currentAltitude;
        currentAltitude = bmp.readAltitude((float)(atmosphere + oldPosition / 4) * 1.33317);
        data += currentAltitude;
      }
      else {
        data += "Z";
        Serial.println(zeroAltitude);
        currentAltitude = bmp.readAltitude((float)(atmosphere + oldPosition / 4) * 1.33317);
        currentAltitude -= zeroAltitude;
        data += currentAltitude;
      }
      data += "     ";
      lcd.print(data.c_str());
      manager.pushEvent(UPDATE_READING, 500);
      break;

    case BUTTON_PRESSED:
      currentBtn = digitalRead(BTN_ENC);
      if (currentBtn == 0) {
        if (!editMode) {
          editMode = true;
          lcd.clear();
          manager.removeEvents(int UPDATE_READING);
          data = "";
          data += atmosphere + oldPosition / 4;
          lcd.print(data.c_str());
        }
        else {
          editMode = false;
          EEPROM.put(EEPROM_ATMOSPHERE, atmosphere + oldPosition / 4);
          manager.pushEvent(UPDATE_READING, 0);
          lcd.clear();
          lcd.print("Altimeter  mk.1");
        }
      }
      break;

    case ZERO:
      currentBtnZero = digitalRead(BTN_ZERO);
      if (currentBtnZero == 0) {
        if (!zeroMode) {
          zeroMode = true;
          EEPROM.put(EEPROM_ZEROMODE, zeroMode);
        }
        else {
          zeroMode = false;
          EEPROM.put(EEPROM_ZEROMODE, zeroMode);
          EEPROM.put(EEPROM_ZEROALTITUDE, zeroAltitude);
        }
      }
  }
  if (editMode) {
    newPosition = myEnc.read();
    if (newPosition != oldPosition) {
      oldPosition = newPosition;
      pressure = newPosition / 4;
      atmosphericPressure = atmosphere + pressure;
      if (!(atmosphericPressure > 850) && !(atmosphericPressure < 600)) {
        data = "";
        data += atmosphericPressure;
      }
      lcd.clear();
      lcd.print(data.c_str());
    }
  }
}