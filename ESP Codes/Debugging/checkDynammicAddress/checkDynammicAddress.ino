#include <Wire.h>
#include <Adafruit_MPU6050.h>

const int MPU6050_ADDR_1 = 0x68;  // Default I2C address of MPU6050
Adafruit_MPU6050 gyro1;
Adafruit_MPU6050 gyro2;
Adafruit_MPU6050 gyro3;

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

  /* Print out the values */
  Serial.print("Acceleration X: ");
  Serial.print(ax);
  Serial.print(", Y: ");
  Serial.print(ay);
  Serial.print(", Z: ");
  Serial.print(az);
  Serial.println(" m/s^2");

  Serial.print("Rotation X: ");
  Serial.print(g.gyro.x);
  Serial.print(", Y: ");
  Serial.print(g.gyro.y);
  Serial.print(", Z: ");
  Serial.print(g.gyro.z);
  Serial.println(" rad/s");

  Serial.print("Temperature: ");
  Serial.print(temp.temperature);
  Serial.println(" degC");
  Serial.println("");
  //accToDisp(ax, ay, az, rx, ry, rz, n);
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
