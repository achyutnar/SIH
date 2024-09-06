#include <Wire.h>
#include <Adafruit_MPU6050.h>

const int MPU6050_ADDR_1 = 0x68;  // Default I2C address of MPU6050
Adafruit_MPU6050 gyro1;
Adafruit_MPU6050 gyro2;
Adafruit_MPU6050 gyro3;

float accAngleX, accAngleY, gyroAngleX, gyroAngleY, gyroAngleZ;
float roll, pitch, yaw;
float AccErrorX, AccErrorY, GyroErrorX, GyroErrorY, GyroErrorZ;
float elapsedTime, currentTime, previousTime;
String uploadPRY;

void setup() {
  Serial.begin(115200);
  Wire.begin();
  pinMode(15, OUTPUT);
  pinMode(13, OUTPUT);
  pinMode(12, OUTPUT);

  gyro1.begin();
  gyro2.begin();
  gyro3.begin();

  digitalWrite(15, HIGH);
  digitalWrite(13, HIGH);
  digitalWrite(12, HIGH);
}

void loop() {
  digitalWrite(15, LOW);
  getValues(gyro1, 1);
  digitalWrite(15, HIGH);
  delay(1000);
  
  digitalWrite(13, LOW);
  getValues(gyro2, 2);
  digitalWrite(13, HIGH);
  delay(1000);

  digitalWrite(12, LOW);
  getValues(gyro3, 3);
  digitalWrite(12, HIGH);
  delay(1000);
}

void getValues(Adafruit_MPU6050 &mpu, int n)
{
  Serial.print("Sensor number : ");
  Serial.println(n);
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  float ax = a.acceleration.x;
  float ay = a.acceleration.y;
  float az = a.acceleration.z;
  float rx = g.gyro.x;
  float ry = g.gyro.y;
  float rz = g.gyro.z;
  
  accToDisp(ax, ay, az, rx, ry, rz, n);
}

void accToDisp(float ax, float ay, float az , float rx, float ry, float rz, int n){
  accAngleX = (atan(ay / sqrt(pow(ax, 2) + pow(az, 2))) * 180 / PI); //- 0.58; // AccErrorX ~(0.58) See the calculate_IMU_error()custom function for more details
  accAngleY = (atan(-1 * ax / sqrt(pow(ay, 2) + pow(az, 2))) * 180 / PI); // + 1.58; // AccErrorY ~(-1.58)

  previousTime = currentTime;        // Previous time is stored before the actual time read
  currentTime = millis();            // Current time actual time read
  elapsedTime = (currentTime - previousTime) / 1000; 
  yaw =  rz * elapsedTime;
  elapsedTime -= 0.25;
  
  // rx = rx + 0.56; // GyroErrorX ~(-0.56)
  // ry = ry - 2; // GyroErrorY ~(2)
  // rz = rz + 0.79; // GyroErrorZ ~ (-0.8)
  
  // Currently the raw values are in degrees per seconds, deg/s, so we need to multiply by sendonds (s) to get the angle in degrees
  gyroAngleX = rx * elapsedTime;
  gyroAngleY = ry * elapsedTime;
  
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
}

void scanI2CDevices() {
  byte error, address;
  int nDevices = 0;
  
  Serial.println("Scanning I2C bus...");

  for (address = 1; address < 127; address++ ) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      if (address < 16) 
        Serial.print("0");
      Serial.print(address, HEX);
      Serial.println(" !");
      
      nDevices++;
    } else if (error == 4) {
      Serial.print("Unknown error at address 0x");
      if (address < 16) 
        Serial.print("0");
      Serial.println(address, HEX);
    }
  }

  if (nDevices == 0)
    Serial.println("No I2C devices found\n");
  else
    Serial.println("I2C scan complete\n");
}
