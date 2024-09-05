#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

Adafruit_MPU6050 gyro1; // First gyroscope
Adafruit_MPU6050 gyro2; // Second gyroscope
Adafruit_MPU6050 gyro3; // Third gyroscope

// Function to initialize a gyroscope with its address
bool initializeGyro(Adafruit_MPU6050 &gyro, const char *name) {
  Serial.print("Initializing ");
  Serial.println(name);

  if (!gyro.begin()) { // Initialize without specifying address
    Serial.print(name);
    Serial.println(" initialization failed!");
    return false;
  }

  Serial.print(name);
  Serial.println(" initialized successfully.");
  return true;
}

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  delay(1000); // Small delay to ensure everything starts up properly

  // Initialize I2C communication
  Wire.begin();
  Wire.setClock(100000); // Reduce I2C clock speed to 100kHz

  // Initialize Gyro1, Gyro2, and Gyro3
  if (!initializeGyro(gyro1, "Gyro1")) {
    while (1); // Stop if initialization fails
  }

  delay(500);  // Add delay for initialization

  // Gyro2 and Gyro3 are automatically initialized through the chain
  if (!initializeGyro(gyro2, "Gyro2")) {
    while (1); // Stop if initialization fails
  }

  delay(500);  // Add delay for initialization

  if (!initializeGyro(gyro3, "Gyro3")) {
    while (1); // Stop if initialization fails
  }
}

void loop() {
  sensors_event_t a1, g1, temp1;
  sensors_event_t a2, g2, temp2;
  sensors_event_t a3, g3, temp3;

  // Read and display data from Gyro1
  gyro1.getEvent(&a1, &g1, &temp1);
  Serial.println("Gyro1 data:");
  Serial.print("Accel X: "); Serial.println(a1.acceleration.x);
  Serial.print("Accel Y: "); Serial.println(a1.acceleration.y);
  Serial.print("Accel Z: "); Serial.println(a1.acceleration.z);
  Serial.print("Gyro X: "); Serial.println(g1.gyro.x);
  Serial.print("Gyro Y: "); Serial.println(g1.gyro.y);
  Serial.print("Gyro Z: "); Serial.println(g1.gyro.z);
  Serial.println();

  // Read and display data from Gyro2
  gyro2.getEvent(&a2, &g2, &temp2);
  Serial.println("Gyro2 data:");
  Serial.print("Accel X: "); Serial.println(a2.acceleration.x);
  Serial.print("Accel Y: "); Serial.println(a2.acceleration.y);
  Serial.print("Accel Z: "); Serial.println(a2.acceleration.z);
  Serial.print("Gyro X: "); Serial.println(g2.gyro.x);
  Serial.print("Gyro Y: "); Serial.println(g2.gyro.y);
  Serial.print("Gyro Z: "); Serial.println(g2.gyro.z);
  Serial.println();

  // Read and display data from Gyro3
  gyro3.getEvent(&a3, &g3, &temp3);
  Serial.println("Gyro3 data:");
  Serial.print("Accel X: "); Serial.println(a3.acceleration.x);
  Serial.print("Accel Y: "); Serial.println(a3.acceleration.y);
  Serial.print("Accel Z: "); Serial.println(a3.acceleration.z);
  Serial.print("Gyro X: "); Serial.println(g3.gyro.x);
  Serial.print("Gyro Y: "); Serial.println(g3.gyro.y);
  Serial.print("Gyro Z: "); Serial.println(g3.gyro.z);
  Serial.println();

  // Delay before reading again
  delay(3000);
}
