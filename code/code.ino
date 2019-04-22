#include <Wire.h>

// Adafruit_MCP9808_Library-1.1.0 - Version: Latest 
#include <Adafruit_MCP9808.h>

// BME280-Release_Version_2.3.0 - Version: Latest
#include <BME280.h>
#include <BME280I2C.h>
#include <BME280I2C_BRZO.h>
#include <EnvironmentCalculations.h>

// I2C Definitions
#define I2C_SD_Pin A4
#define I2C_SK_Pin A5
#define I2C_BMP280_Addr
#define I2C_MCP9808_Addr 0x18
#define I2C_IMU_Addr

// Declare Globals
Adafruit_MCP9808 tempsensor = Adafruit_MCP9808();
BME280I2C barsensor;
struct t_report{
    uint8_t BMP280_flag = 0;
    float BMP280_temp = 0;
    float BMP280_pres = 0;
    uint16_t BMP280_samples = 0;
    uint8_t MCP9808_flag = 0;
    float MCP9808_temp = 0;
    uint16_t MCP9808_samples = 0;
};

void barRoutine(unsigned long), tempRoutine(unsigned long), setTempResolution(int), submitReport(), reportRoutine();

// MCP9808 Globals
unsigned int MCP9808_Interval;
unsigned long MCP9808_Timestamp;

// BME280 Globals
unsigned int BMP280_Interval;
unsigned long BMP280_Timestamp;

// Global Flags
unsigned char tempFlag = 0;
unsigned char barFlag = 0;

// Global Report
t_report c_report;
t_report p_report;

void setup() {
  Wire.begin();
  Serial.begin(19200);
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
  reportRoutine();
}

void reportRoutine() {
  if(c_report.BMP280_flag == 1 && c_report.MCP9808_flag == 1){
    submitReport();
  }
}

void submitReport() {
  p_report = c_report;
  t_report n_report;
  c_report = n_report;
  Serial.print((char) 1); // Begin transmission
  Serial.print((char) 0); // Status of message
  char *temp = (char *) &p_report.BMP280_temp;
  for(int s=0; s<4; s++){
    Serial.print(*(temp+s));
  }
  temp = (char *) &p_report.BMP280_pres;
  for(int s=0; s<4; s++){
    Serial.print(*(temp+s));
  }
  temp = (char *) &p_report.BMP280_samples;
  for(int s=0; s<2; s++){
    Serial.print(*(temp+s));
  }
  temp = (char *) &p_report.MCP9808_temp;
  for(int s=0; s<4; s++){
    Serial.print(*(temp+s));
  }
  temp = (char *) &p_report.MCP9808_samples;
  for(int s=0; s<2; s++){
    Serial.print(*(temp+s));
  }
  Serial.print((char) 4); // End transmission
}

void barRoutine(unsigned long clock) {
  if (BMP280_Timestamp < clock){
    float temp = barsensor.temp();
    float pres = barsensor.pres();
    if(c_report.BMP280_flag == 1){
      c_report.BMP280_temp += temp;
      c_report.BMP280_temp /= 2;
      c_report.BMP280_pres += pres;
      c_report.BMP280_pres /= 2;
      c_report.BMP280_samples += 1;
    }
    else{
      c_report.BMP280_temp = temp;
      c_report.BMP280_pres = pres;
      c_report.BMP280_samples = 1;
      c_report.BMP280_flag = 1; // Raise barometer flag
    }
    BMP280_Timestamp = clock + BMP280_Interval;
  }
}

void tempRoutine(unsigned long clock) {
  if (MCP9808_Timestamp < clock){
    float temp = tempsensor.readTempC();
    
    if(c_report.MCP9808_flag == 1){
      c_report.MCP9808_temp += temp;
      c_report.MCP9808_temp /= 2;
      c_report.MCP9808_samples += 1;
    }
    else{
      c_report.MCP9808_temp = temp;
      c_report.MCP9808_samples = 1;
      c_report.MCP9808_flag = 1; // Raise temperature flag
    }
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
