#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <ESP8266WiFi.h>

Adafruit_MPU6050 mpuMain;
Adafruit_MPU6050 mpuMiddle;

WiFiClient client;

char* ssid = "Achyut FF";
char* password = "bhadri9019*";
char* serverIP = "10.15.40.103"; 
int port = 8080; 

float accAngleX, accAngleY, gyroAngleX, gyroAngleY, gyroAngleZ;
float roll, pitch, yaw;
float AccErrorX, AccErrorY, GyroErrorX, GyroErrorY, GyroErrorZ;
float elapsedTime, currentTime, previousTime;
String uploadPRY;

void wifiConnect()
{
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi");
}

void mpu6050Init()
{
  // Initialize main chip with address 0x68
  if (!mpuMain.begin(0x68, &Wire)) {
    Serial.println("Failed to find MPU6050 main Chip");
    while (1) {
      delay(10);
    }
  }
  Serial.println("Main chip initialized");

  // Initialize thumb chip with address 0x69
  if (!mpuMiddle.begin(0x69, &Wire)) {
    Serial.println("Failed to find MPU6050 Middle Chip");
    while (1) {
      delay(10);
    }
  }
  Serial.println("Middle chip initialized");
}

void mpu6050Setup(Adafruit_MPU6050 &mpu)
{
  Serial.println("Setup Start");
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  Serial.print("Accelerometer range set to: ");
  switch (mpu.getAccelerometerRange()) {
  case MPU6050_RANGE_2_G:
    Serial.println("+-2G");
    break;
  case MPU6050_RANGE_4_G:
    Serial.println("+-4G");
    break;
  case MPU6050_RANGE_8_G:
    Serial.println("+-8G");
    break;
  case MPU6050_RANGE_16_G:
    Serial.println("+-16G");
    break;
  }

  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  Serial.print("Gyro range set to: ");
  switch (mpu.getGyroRange()) {
  case MPU6050_RANGE_250_DEG:
    Serial.println("+- 250 deg/s");
    break;
  case MPU6050_RANGE_500_DEG:
    Serial.println("+- 500 deg/s");
    break;
  case MPU6050_RANGE_1000_DEG:
    Serial.println("+- 1000 deg/s");
    break;
  case MPU6050_RANGE_2000_DEG:
    Serial.println("+- 2000 deg/s");
    break;
  }

  mpu.setFilterBandwidth(MPU6050_BAND_5_HZ);
  Serial.print("Filter bandwidth set to: ");
  switch (mpu.getFilterBandwidth()) {
  case MPU6050_BAND_260_HZ:
    Serial.println("260 Hz");
    break;
  case MPU6050_BAND_184_HZ:
    Serial.println("184 Hz");
    break;
  case MPU6050_BAND_94_HZ:
    Serial.println("94 Hz");
    break;
  case MPU6050_BAND_44_HZ:
    Serial.println("44 Hz");
    break;
  case MPU6050_BAND_21_HZ:
    Serial.println("21 Hz");
    break;
  case MPU6050_BAND_10_HZ:
    Serial.println("10 Hz");
    break;
  case MPU6050_BAND_5_HZ:
    Serial.println("5 Hz");
    break;
  }
  Serial.println("");
}

void getValues(Adafruit_MPU6050 &mpu, int n)
{
  if(n == 1)
  {
    Serial.println("Main sensor");
  }
  else if(n == 2)
  {
    Serial.println("Thumb sensor");
  }
  else if(n == 3)
  {
    Serial.println("Point sensor");
  }
  else
  {
    Serial.println("Middle sensor");
  }
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  float ax = a.acceleration.x;
  float ay = a.acceleration.y;
  float az = a.acceleration.z;
  float rx = g.gyro.x;
  float ry = g.gyro.y;
  float rz = g.gyro.z;

  /* Print out the values */
//  Serial.print("Acceleration X: ");
//  Serial.print(ax);
//  Serial.print(", Y: ");
//  Serial.print(ay);
//  Serial.print(", Z: ");
//  Serial.print(az);
//  Serial.println(" m/s^2");
//
//  Serial.print("Rotation X: ");
//  Serial.print(g.gyro.x);
//  Serial.print(", Y: ");
//  Serial.print(g.gyro.y);
//  Serial.print(", Z: ");
//  Serial.print(g.gyro.z);
//  Serial.println(" rad/s");
//
//  Serial.print("Temperature: ");
//  Serial.print(temp.temperature);
//  Serial.println(" degC");
//  Serial.println("");
  accToDisp(ax, ay, az, rx, ry, rz, n);
}

void accToDisp(float ax, float ay, float az , float rx, float ry, float rz, int n){
  accAngleX = (atan(ay / sqrt(pow(ax, 2) + pow(az, 2))) * 180 / PI); //- 0.58; // AccErrorX ~(0.58) See the calculate_IMU_error()custom function for more details
  accAngleY = (atan(-1 * ax / sqrt(pow(ay, 2) + pow(az, 2))) * 180 / PI); // + 1.58; // AccErrorY ~(-1.58)

  // === Read gyroscope data === //
  previousTime = currentTime;        // Previous time is stored before the actual time read
  currentTime = millis();            // Current time actual time read
  elapsedTime = (currentTime - previousTime) / 1000; 
  yaw = rz * elapsedTime;
  elapsedTime -= 0.25;
  // rx = rx + 0.56; // GyroErrorX ~(-0.56)
  // ry = ry - 2; // GyroErrorY ~(2)
  // rz = rz + 0.79; // GyroErrorZ ~ (-0.8)
  // Currently the raw values are in degrees per seconds, deg/s, so we need to multiply by sendonds (s) to get the angle in degrees
//  gyroAngleX = gyroAngleX + rx * elapsedTime; // deg/s * s = deg
//  gyroAngleY = gyroAngleY + ry * elapsedTime;
  gyroAngleX =  rx * elapsedTime; // deg/s * s = deg
  gyroAngleY =  ry * elapsedTime;

  
  // Complementary filter - combine acceleromter and gyro angle values
  roll = 0.96 * gyroAngleX + 0.04 * accAngleX;
  pitch = 0.96 * gyroAngleY + 0.04 * accAngleY;

  Serial.print("Pitch: ");
  Serial.println(pitch);
  Serial.print("Roll: ");
  Serial.println(roll);
  Serial.print("Yaw: ");
  Serial.println(yaw);

  if(n == 1)
  {
    makeString("Main", pitch, roll, yaw);
  }
  else if(n == 2)
  {
    makeString("Thumb", pitch, roll, yaw);
  }
  else if(n == 3)
  {
    makeString("Point", pitch, roll, yaw);
  }
  else
  {
    makeString("Middle", pitch, roll, yaw);
  }
}

void makeString(String sensor, float p, float r, float y)
{
  // Convert each float value to a string with two decimal places
  String pitchStr = String(p, 2);
  String rollStr = String(r, 2);
  String yawStr = String(y, 2);

  // Concatenate the strings with a delimiter (e.g., comma)
  uploadPRY = sensor + ":" + pitchStr + "," + rollStr + "," + yawStr;

  // Print the final string for debugging purposes
  Serial.print("Encoded String: ");
  Serial.println(uploadPRY);

  if (client.connect(serverIP, port)) {
    Serial.println("Connected to server");

    // Send the string to the server
    client.println(uploadPRY);
    Serial.println("Data sent to server: " + uploadPRY);

    // Close the connection
    client.stop();
  } 
  else {
    Serial.println("Failed to connect to server");
  }  
}

void setup() 
{
  Serial.begin(115200);
  wifiConnect();
  Wire.begin();
  Serial.println("Trying to initialize sensors");
  mpu6050Init(); //initialize both MPU 6050 chips
  //setup MPU 6050
  mpu6050Setup(mpuMain); 
  mpu6050Setup(mpuMiddle); 
}

void loop() 
{
  getValues(mpuMain, 1);
  delay(200); // Shorter delay
  getValues(mpuMiddle, 4);
  delay(200); // Shorter delay
}
