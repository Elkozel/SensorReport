#include <Wire.h>

// Adafruit_MCP9808_Library-1.1.0 - Version: Latest 
#include <Adafruit_MCP9808.h>

// BME280-Release_Version_2.3.0 - Version: Latest
#include <BME280.h>
#include <BME280I2C.h>
#include <BME280I2C_BRZO.h>
#include <EnvironmentCalculations.h>

// I2C Definitions
#define I2C_CS_Pin
#define I2C_CK_Pin
#define I2C_BMP280_Addr 
#define I2C_MCP9808_Addr 0x18
#define I2C_IMU_Addr

// Declare Globals
Adafruit_MCP9808 tempsensor = Adafruit_MCP9808();
BME280I2C barsensor;

void barRoutine(unsigned long), tempRoutine(unsigned long), setTempResolution(int);

// MCP9808 Globals
unsigned int MCP9808_Interval;
unsigned long MCP9808_Timestamp;

// BME280 Globals
unsigned int BMP280_Interval;
unsigned long BMP280_Timestamp;

// Global Flags
unsigned char tempFlag = 0;
unsigned char barFlag = 0;

void setup() {
  Wire.begin();
  Serial.begin(9600);
  while(!Serial) {}

  Serial.println(F("Starting setup"));
  Serial.println(F("Setting up temprature sensor"));
  if (!tempsensor.begin(I2C_MCP9808_Addr)) {
    Serial.println(F("Couldn't find MCP9808! Check your connections and verify the address is correct."));
    while (1);
  }
  setTempResolution(3);
  tempsensor.wake();
  Serial.println(F("Temperature sensor set successfully"));

  Serial.println(F("Setting up barometric sensor"));
  if (!barsensor.begin()) {
    Serial.println(F("Couldn't find BME280!"));
    while(1);
  }
  BMP280_Interval = 50;
  BMP280_Timestamp = millis();
  Serial.println(F("Barometric sensor set successfully"));
}

void loop() {
  unsigned long currentMillis = millis();
  tempRoutine(currentMillis);
  barRoutine(currentMillis);
}

void barRoutine(unsigned long clock) {
  if (BMP280_Timestamp < clock){
    float temp;
    float hum;
    float pres;
    BME280::TempUnit tempUnit(BME280::TempUnit_Celsius);
    BME280::PresUnit presUnit(BME280::PresUnit_Pa);
    barsensor.read(pres, temp, hum, tempUnit, presUnit);
    barFlag = 1; // Raise temperature flag
    Serial.println("BME280: ");
    Serial.print("Temperature: ");
    Serial.println(temp);
    Serial.print("Pressure: ");
    Serial.println(pres);
    Serial.print("Humidity: ");
    Serial.println(hum);
    Serial.println();
    BMP280_Timestamp = clock + BMP280_Interval;
  }
}

void tempRoutine(unsigned long clock) {
  if (MCP9808_Timestamp < clock){
    float temp = tempsensor.readTempC();
    tempFlag = 1; // Raise temperature flag
    Serial.println("MCP9808: ");
    Serial.print("Temperature: ");
    Serial.println(temp);
    Serial.println();
    MCP9808_Timestamp = clock + MCP9808_Interval;
  }
}

void setTempResolution(int level) {
    // Mode Resolution SampleTime
    //  0    0.5°C       30 ms
    //  1    0.25°C      65 ms
    //  2    0.125°C     130 ms
    //  3    0.0625°C    250 ms
    switch (level)
    {
        case 0:
            MCP9808_Interval = 30;
            break;
        case 1:
            MCP9808_Interval = 65;
            break;
        case 2:
            MCP9808_Interval = 130;
            break;
        case 3:
            MCP9808_Interval = 250;
            break;
        default:
            return;
    }
    tempsensor.setResolution(level);
    MCP9808_Timestamp = millis() + MCP9808_Interval;
}