#include <Wire.h>
#include <SPI.h>
#include <SD.h>

// BME280-Release_Version_2.3.0 - Version: Latest
#include <BME280.h>
#include <BME280I2C.h>

// MCP9250-1.0.1 - Version: Latest
#include <MPU9250.h>

// Pin Definitions
#define SD_MOSI 11
#define SD_MISO 12
#define SD_CLK 13
#define SD_CS 10

// I2C Definitions
#define I2C_SD_Pin A4
#define I2C_SK_Pin A5
#define I2C_BMP280_Addr 0x76
#define I2C_MPU9250_Addr 0x68

// Define intervals
#define filename "report.txt"
#define MPU9250_Interval 65L
#define BMP280_Interval 50L

// Declare Globals
BME280I2C barsensor;
MPU9250 IMU(Wire, I2C_MPU9250_Addr);
File logger;
struct t_report{
    uint8_t BMP280_flag = 0;
    float BMP280_temp = 0;
    float BMP280_pres = 0;
    uint8_t BMP280_samples = 0;
    uint8_t MPU9250_flag = 0;
    float MPU9250_accel[3];
    float MPU9250_gyro[3];
    float MPU9250_mag[3];
    uint8_t MPU9250_samples = 0;
};

void barRoutine(unsigned long), IMURoutine(unsigned long), setTempResolution(int), submitReport(), reportRoutine();

// BME280 Globals
unsigned long BMP280_Timestamp = 0;

// MPU9250 Globals
unsigned long MPU9250_Timestamp = 0;

// Global Report
t_report c_report;

void setup() {
  Wire.begin();
  Serial.begin(19200);
  while(!Serial) {}

  Serial.println(F("Starting setup"));
//  Serial.println(F("Setting up SD card"));
//  if(!SD.begin(SD_CS)) { // If card could not be accessed
//    Serial.println(F("Could not initialize card"));
//    while(1);
//  }
//  if(!SD.exists("reports/")) // if folder reports exists
//    SD.mkdir("reports/");
//  logger = SD.open(filename);
//  Serial.println(F("Card initialized sucessfully"));
  
//  Serial.println(F("Setting up temprature sensor"));
//  if (!tempsensor.begin(I2C_MCP9808_Addr)) {
//    Serial.println(F("Couldn't find MCP9808! Check your connections and verify the address is correct."));
//    while (1);
//  }
//  tempsensor.setResolution(1);
//  MCP9808_Timestamp = millis() + MCP9808_Interval;
//  tempsensor.wake();
//  Serial.println(F("Temperature sensor set successfully"));

  Serial.println(F("Setting up barometric sensor"));
  if (!barsensor.begin()) {
    Serial.println(F("Couldn't find BME280!"));
    while(1);
  }
  BMP280_Timestamp = millis() + BMP280_Interval;
  Serial.println(F("Barometric sensor set successfully"));

  Serial.println(F("Setting IMU"));
  if(IMU.begin()<0){
    Serial.print(F("MPU9250 failed"));
    while(1);
  }
  IMU.setAccelRange(MPU9250::ACCEL_RANGE_8G);
  IMU.setGyroRange(MPU9250::GYRO_RANGE_500DPS);
  IMU.setDlpfBandwidth(MPU9250::DLPF_BANDWIDTH_20HZ);
  IMU.setSrd(19);
  Serial.println(F("IMU set successfully"));
  MPU9250_Timestamp = millis() + MPU9250_Interval;

  Serial.println(F("Setup complete"));
}

void loop() {
  unsigned long currentMillis = millis();
  barRoutine(currentMillis);
  IMURoutine(currentMillis);
  reportRoutine();
}

void reportRoutine() {
  if(c_report.BMP280_flag == 1 && c_report.MPU9250_flag == 1){
    submitReport();
    c_report.BMP280_flag = 0;
    c_report.MPU9250_flag = 0;
  }
}

void submitReport() {
  saveReport();
  Serial.print((char) 1); // Begin transmission
  Serial.print((char) 1); // Set sensor BMP280/MCP9808
  Serial.write((char *)&c_report.BMP280_temp, 4);
  Serial.write((char *)&c_report.BMP280_pres, 4);
  Serial.write((char *)&c_report.BMP280_samples, 2);
  Serial.write((char *)&c_report.MPU9250_accel, 12);
  Serial.write((char *)&c_report.MPU9250_gyro, 12);
  Serial.write((char *)&c_report.MPU9250_mag, 12);
  Serial.write((char *)&c_report.MPU9250_samples, 2);
  Serial.print((char) 4); // End transmission
}

void saveReport(){
  logger.write(millis());
  logger.write((char *)&c_report.BMP280_temp, 4);
  logger.write((char *)&c_report.BMP280_pres, 4);
  logger.write((char *)&c_report.BMP280_samples, 2);
  logger.write((char *)&c_report.MPU9250_accel, 12);
  logger.write((char *)&c_report.MPU9250_gyro, 12);
  logger.write((char *)&c_report.MPU9250_mag, 12);
  logger.write((char *)&c_report.MPU9250_samples, 2);
  logger.write("\n");
  logger.flush();
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
      c_report.MPU9250_flag = 1;
      c_report.MPU9250_samples = 1;
    }
    MPU9250_Timestamp = clock + MPU9250_Interval;
  }
}
