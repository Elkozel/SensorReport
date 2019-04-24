#include <Wire.h>

// Adafruit_MCP9808_Library-1.1.0 - Version: Latest 
#include <Adafruit_MCP9808.h>

// BME280-Release_Version_2.3.0 - Version: Latest
#include <BME280.h>
#include <BME280I2C.h>
#include <BME280I2C_BRZO.h>
#include <EnvironmentCalculations.h>

// MCP9250-1.0.1 - Version: Latest
#include <MPU9250.h>

// I2C Definitions
#define I2C_SD_Pin A4
#define I2C_SK_Pin A5
#define I2C_BMP280_Addr 0x76
#define I2C_MCP9808_Addr 0x18
#define I2C_MPU9250_Addr 0x68

// Declare Globals
Adafruit_MCP9808 tempsensor = Adafruit_MCP9808();
BME280I2C barsensor;
MPU9250 IMU(Wire, I2C_MPU9250_Addr);
struct t_report{
    uint8_t BMP280_flag = 0;
    float BMP280_temp = 0;
    float BMP280_pres = 0;
    uint16_t BMP280_samples = 0;
    uint8_t MCP9808_flag = 0;
    float MCP9808_temp = 0;
    uint16_t MCP9808_samples = 0;
    uint8_t MPU9250_flag = 0;
    float MPU9250_accel[3];
    float MPU9250_gyro[3];
    float MPU9250_mag[3];
    float MPU9250_temp = 0;
    uint16_t MPU9250_samples = 0;
};

void barRoutine(unsigned long), tempRoutine(unsigned long), IMURoutine(unsigned long), setTempResolution(int), submitReport(), reportRoutine();

// MCP9808 Globals
unsigned int MCP9808_Interval;
unsigned long MCP9808_Timestamp;

// BME280 Globals
unsigned int BMP280_Interval;
unsigned long BMP280_Timestamp;

// MPU9250 Globals
unsigned int MPU9250_Interval;
unsigned long MPU9250_Timestamp;

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

  Serial.println(F("Setting IMU"));
  int status = 0;
  if((status = IMU.begin())<0){
    Serial.print(F("MPU9250 Setup failed (Status: "));
    Serial.print(status);
    Serial.println(F(")"));
    while(1);
  }
  IMU.setAccelRange(MPU9250::ACCEL_RANGE_8G);
  IMU.setGyroRange(MPU9250::GYRO_RANGE_500DPS);
  IMU.setDlpfBandwidth(MPU9250::DLPF_BANDWIDTH_20HZ);
  IMU.setSrd(19);
  Serial.println(F("IMU sensor set successfully"));
  BMP280_Interval = 20;
  BMP280_Timestamp = millis();

  Serial.println(F("Setup complete"));
}

void loop() {
  unsigned long currentMillis = millis();
  tempRoutine(currentMillis);
  barRoutine(currentMillis);
  IMURoutine(currentMillis);
  reportRoutine();
}

void reportRoutine() {
  if(c_report.BMP280_flag == 1 && c_report.MCP9808_flag == 1 && c_report.MPU9250_flag == 1){
    submitReport();
  }
}

void submitReport() {
  p_report = c_report;
  t_report n_report;
  c_report = n_report;
  
  Serial.print((char) 1); // Begin transmission
  Serial.print((char) 1); // Set sensor BMP280/MCP9808
  Serial.write((char *)&p_report.BMP280_temp, 4);
  Serial.write((char *)&p_report.BMP280_pres, 4);
  Serial.write((char *)&p_report.BMP280_samples, 2);
  Serial.write((char *)&p_report.MCP9808_temp, 4);
  Serial.write((char *)&p_report.MCP9808_samples, 2);
  Serial.write((char *)&p_report.MPU9250_accel, 12);
  Serial.write((char *)&p_report.MPU9250_gyro, 12);
  Serial.write((char *)&p_report.MPU9250_mag, 12);
  Serial.write((char *)&p_report.MPU9250_temp, 4);
  Serial.write((char *)&p_report.MPU9250_samples, 2);
  Serial.print((char) 4); // End transmission
}

void statusMsg(const char * msg){
  Serial.print((char) 1); // Begin transmission
  Serial.print((char) 0); // Set status
  Serial.print(msg);
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

void IMURoutine(unsigned long clock) {
  if (MPU9250_Timestamp < clock){
    IMU.readSensor();
    if(c_report.MPU9250_flag > 0){
      c_report.MPU9250_accel[0] += IMU.getAccelX_mss();
      c_report.MPU9250_accel[0] /= 2;
      c_report.MPU9250_accel[1] += IMU.getAccelY_mss();
      c_report.MPU9250_accel[1] /= 2;
      c_report.MPU9250_accel[2] += IMU.getAccelZ_mss();
      c_report.MPU9250_accel[2] /= 2;
      c_report.MPU9250_gyro[0] += IMU.getGyroX_rads();
      c_report.MPU9250_gyro[0] /= 2;
      c_report.MPU9250_gyro[1] += IMU.getGyroY_rads();
      c_report.MPU9250_gyro[1] /= 2;
      c_report.MPU9250_gyro[2] += IMU.getGyroZ_rads();
      c_report.MPU9250_gyro[2] /= 2;
      c_report.MPU9250_mag[0] += IMU.getMagX_uT();
      c_report.MPU9250_mag[0] /= 2;
      c_report.MPU9250_mag[1] += IMU.getMagY_uT();
      c_report.MPU9250_mag[1] /= 2;
      c_report.MPU9250_mag[2] += IMU.getMagZ_uT();
      c_report.MPU9250_mag[2] /= 2;
      c_report.MPU9250_temp += IMU.getTemperature_C();
      c_report.MPU9250_temp /= 2;
      c_report.MPU9250_samples++;
    }
    else{
      c_report.MPU9250_accel[0] = IMU.getAccelX_mss();
      c_report.MPU9250_accel[1] = IMU.getAccelY_mss();
      c_report.MPU9250_accel[2] = IMU.getAccelZ_mss();
      c_report.MPU9250_gyro[0] = IMU.getGyroX_rads();
      c_report.MPU9250_gyro[1] = IMU.getGyroY_rads();
      c_report.MPU9250_gyro[2] = IMU.getGyroZ_rads();
      c_report.MPU9250_mag[0] = IMU.getMagX_uT();
      c_report.MPU9250_mag[1] = IMU.getMagY_uT();
      c_report.MPU9250_mag[2] = IMU.getMagZ_uT();
      c_report.MPU9250_temp = IMU.getTemperature_C();
      c_report.MPU9250_flag = 1;
      c_report.MPU9250_samples++;
    }
    MPU9250_Timestamp = clock + MPU9250_Interval;
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
