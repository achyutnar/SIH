#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
//#include <SoftwareWire.h>
#include <Wire.h>

Adafruit_MPU6050 mpuMain;
Adafruit_MPU6050 mpuThumb;
Adafruit_MPU6050 mpuPoint;
Adafruit_MPU6050 mpuMiddle;

TwoWire Wire1 = TwoWire();
TwoWire Wire2 = TwoWire();

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
  if (!mpuThumb.begin(0x69, &Wire)) {
    Serial.println("Failed to find MPU6050 thumb Chip");
    while (1) {
      delay(10);
    }
  }
  Serial.println("Thumb chip initialized");

  // Initialize point chip with address 0x69
  if (!mpuThumb.begin(0x68, &Wire)) {
    Serial.println("Failed to find MPU6050 point Chip");
    while (1) {
      delay(10);
    }
  }
  Serial.println("Point chip initialized");
  
  // Initialize middle chip with address 0x69
  if (!mpuThumb.begin(0x69, &Wire)) {
    Serial.println("Failed to find MPU6050 middle Chip");
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

  /* Print out the values */
  Serial.print("Acceleration X: ");
  Serial.print(a.acceleration.x);
  Serial.print(", Y: ");
  Serial.print(a.acceleration.y);
  Serial.print(", Z: ");
  Serial.print(a.acceleration.z);
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
}

void setup() 
{
  Serial.begin(9600);
  Wire.begin();
  //Wire1.begin(4, 3);
  //Wire2.begin(5, 6);
  Serial.println("Trying to initialize sensors");
  mpu6050Init(); //initialize both MPU 6050 chips
  //setup MPU 6050
  mpu6050Setup(mpuMain); 
  mpu6050Setup(mpuThumb); 
  mpu6050Setup(mpuPoint);
  mpu6050Setup(mpuMiddle);
}

void loop() 
{
  getValues(mpuMain, 1);
  delay(1000); // Shorter delay
  getValues(mpuThumb, 2);
  delay(1000); // Shorter delay
  getValues(mpuPoint, 3);
  delay(1000); // Shorter delay
  getValues(mpuMiddle, 4);
  delay(1000); // Shorter delay
}
