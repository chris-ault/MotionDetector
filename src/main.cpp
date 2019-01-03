#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <driver/adc.h>

/*
MotionDetector
=====================================
Wiring
======
MPU6050    GY-521(MPU6050)
3.3V       VCC ( GY-521 has onboard regulator 5v tolerant)
GND        GND
21         SDA
22         SCL
15         INT - Note below on allowed RTC IO Pins
*/

const char* ssid = "ssid";
const char* password = "password";
String deviceId = "pushingbox_vDeviceId_here";
const char* logServer = "api.pushingbox.com";

/*
Current Usage
=====================================
// 5.37mA with GY-521 LED attached
// 3.96mA with Led Broken/removed, Temp and Gyro still active, high speed parsing
// 0.71mA with Broken LED, Gyro's disabled
// 0.65mA with Broken LED, Gyro's disabled, Temp sensor disabled
*/

/*
Deep Sleep with External Wake Up
=====================================
NOTE:
======
Only RTC IO can be used as a source for external wake
source. They are pins: 0,2,4,12-15,25-27,32-39.
*/

// MPU registers
#define SIGNAL_PATH_RESET  0x68
#define I2C_SLV0_ADDR      0x37
#define ACCEL_CONFIG       0x1C
#define MOT_THR            0x1F  // Motion detection threshold bits [7:0]
#define MOT_DUR            0x20  // This seems wrong // Duration counter threshold for motion interrupt generation, 1 kHz rate, LSB = 1 ms
#define MOT_DETECT_CTRL    0x69
#define INT_ENABLE         0x38
#define PWR_MGMT           0x6B //SLEEPY TIME
#define INT_STATUS 0x3A
#define MPU6050_ADDRESS 0x68 //AD0 is 0

/*
Set your Static IP address
=====================================
// Settled on not using this however it could be beneficial
======
IPAddress local_IP(192, 168, 1, 229);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 0, 0);
*/

/*    Example for using write byte
      Configure the accelerometer for self-test
      writeByte(MPU6050_ADDRESS, ACCEL_CONFIG, 0xF0); // Enable self test on all three axes and set accelerometer range to +/- 8 g */
void writeByte(uint8_t address, uint8_t subAddress, uint8_t data)
{
  Wire.begin();
  Wire.beginTransmission(address);  // Initialize the Tx buffer
  Wire.write(subAddress);           // Put slave register address in Tx buffer
  Wire.write(data);                 // Put data in Tx buffer
  Wire.endTransmission();           // Send the Tx buffer
}

//example showing using readbytev   ----    readByte(MPU6050_ADDRESS, GYRO_CONFIG);
uint8_t readByte(uint8_t address, uint8_t subAddress)
{
  uint8_t data;                            // `data` will store the register data
  Wire.beginTransmission(address);         // Initialize the Tx buffer
  Wire.write(subAddress);                  // Put slave register address in Tx buffer
  Wire.endTransmission(false);             // Send the Tx buffer, but send a restart to keep connection alive
  Wire.requestFrom(address, (uint8_t) 1);  // Read one byte from slave register address
  data = Wire.read();                      // Fill Rx buffer with result
  return data;                             // Return data read from slave register
}

// sens argument configures wake up sensitivity
void configureMPU(int sens){
  writeByte( MPU6050_ADDRESS, 0x6B, 0x00);
  writeByte( MPU6050_ADDRESS, SIGNAL_PATH_RESET, 0x07);//Reset all internal signal paths in the MPU-6050 by writing 0x07 to register 0x68;
  // writeByte( MPU6050_ADDRESS, I2C_SLV0_ADDR, 0x20);//write register 0x37 to select how to use the interrupt pin. For an active high, push-pull signal that stays until register (decimal) 58 is read, write 0x20.
  writeByte( MPU6050_ADDRESS, ACCEL_CONFIG, 0x01);//Write register 28 (==0x1C) to set the Digital High Pass Filter, bits 3:0. For example set it to 0x01 for 5Hz. (These 3 bits are grey in the data sheet, but they are used! Leaving them 0 means the filter always outputs 0.)
  writeByte( MPU6050_ADDRESS, MOT_THR, sens);  //Write the desired Motion threshold to register 0x1F (For example, write decimal 20).
  writeByte( MPU6050_ADDRESS, MOT_DUR, 40 );  //Set motion detect duration to 1  ms; LSB is 1 ms @ 1 kHz rate
  writeByte( MPU6050_ADDRESS, MOT_DETECT_CTRL, 0x15); //to register 0x69, write the motion detection decrement and a few other settings (for example write 0x15 to set both free-fall and motion decrements to 1 and accelerometer start-up delay to 5ms total by adding 1ms. )
  writeByte( MPU6050_ADDRESS, 0x37, 140 ); // now INT pin is active low
  writeByte( MPU6050_ADDRESS, INT_ENABLE, 0x40 ); //write register 0x38, bit 6 (0x40), to enable motion detection interrupt.
  writeByte( MPU6050_ADDRESS, PWR_MGMT, 8 ); // 101000 - Cycle & disable TEMP SENSOR
  writeByte( MPU6050_ADDRESS, 0x6C, 7); // Disable Gyros
}

void sendNotification(String message){
  Serial.println("- connecting to Home Router SID: " + String(ssid));
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  WiFiClient client;
  if (client.connect(logServer, 80)) {
    delay(5000); // This delay allows time for connection to server to complete
    Serial.println("- succesfully connected to push server");
    String postStr = "devid=";
    postStr += String(deviceId);
    postStr += "&message_parameter=";
    postStr += String(message);
    postStr += "\r\n\r\n";
    Serial.println("- sending data...");
    client.print("POST /pushingbox HTTP/1.1\n");
    client.print("Host: api.pushingbox.com\n");
    client.print("Connection: close\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(postStr.length());
    client.print("\n\n");
    client.print(postStr);
    client.print("\n\n");
    delay(500);
  }
  client.stop();
  Serial.println("- stopping the client");
  Serial.println("Stopping wifi");
  WiFi.mode(WIFI_OFF);
  btStop();
}

void setup(){
  int start = esp_timer_get_time();
  Serial.begin(115200);
  sendNotification("Motion Detected");
  configureMPU(1); // Setup MPU for interrupt, power down gyros & temp sensor
  int end = esp_timer_get_time();
  int total = end - start;
  Serial.println(total);
  Serial.println("Sleeping");
  adc_power_off();  // adc power off disables wifi entirely, upstream bug
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_15,0); delay(1500);//1 = High, 0 = Low
  adc_power_off();
  esp_deep_sleep_start();
}

void loop(){
}
