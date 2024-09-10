#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

const int MPU_ADDR = 0x68;
const int NUM_OF_SENSORS = 3;

// MPU Data Variables
Adafruit_MPU6050 MPU[NUM_OF_SENSORS];                              /// Array of MPU6050 Objects
VectorInt16 accel[NUM_OF_SENSORS], gyro[NUM_OF_SENSORS];  /// Accelerometer & Gyroscope Data
Quaternion quat[NUM_OF_SENSORS];                          /// Quaternion
VectorFloat gravity[NUM_OF_SENSORS];                      /// Gravity
float rpy[NUM_OF_SENSORS][3];                             /// Roll-Pitch-Yaw Data
unsigned long time_stamp[NUM_OF_SENSORS];                 /// Time Stamp (ms)

/// DMP Variables
uint16_t packet_size[NUM_OF_SENSORS];           /// DMP Packet Size
uint16_t fifo_count[NUM_OF_SENSORS];            /// FIFO Count
uint8_t fifo_buffer[NUM_OF_SENSORS][64];        /// FIFO Buffer

/// Interrupt Variables
bool mpu_interrupt[NUM_OF_SENSORS];         /// MPU Interrupt
uint16_t fifo_alive[NUM_OF_SENSORS];        /// FIFO Alive

void MPU6050init()
{
  for ( int imu = 0; 
        imu < NUM_OF_SENSORS; 
        imu++ ) 
  {
    Serial.print("[INFO] {IMU #");  Serial.print(imu);  Serial.print("} Initializing MPU...");
    ToggleMPU(imu);                     /// Toggle MPU

    MPU[imu].initialize();                  /// Initialize MPU6050
    MPU[imu].setFullScaleGyroRange(MPU6050_GYRO_FS_1000); /// Set Gyro Range to 1000dps
    MPU[imu].setFullScaleAccelRange(MPU6050_ACCEL_FS_4);  /// Set Accel Range to 4g
    MPU[imu].CalibrateGyro(3);                /// Calibrate Gyro
    MPU[imu].CalibrateAccel(3);               /// Calibrate Accel
    MPU[imu].dmpInitialize();               /// Initialize DMP
    MPU[imu].setDMPEnabled(true);             /// Enable DMP
    packet_size[imu] = MPU[imu].dmpGetFIFOPacketSize();   /// Get DMP Packet Size
    fifo_count[imu] = MPU[imu].getFIFOCount();        /// Get FIFO Count
    Serial.println(" Done! (");

    UnToggleMPU(imu);                   /// Untoggle MPU
  }
}

int PinSelector(int IMU_NUM)
{
  switch(IMU_NUM) {
    case 0:   return 2;
    case 1:   return 3;
    case 2:   return 4;
    case 3:   return 5;
    case 4:   return 6;
    case 5:   return 7;
    case 6:   return 8;
    case 7:   return 9;
    case 8:   return 10;
    case 9:   return 11;
  }
}

void UnToggleMPU(int IMU_NUM)
{
  int pin = PinSelector(IMU_NUM);
  digitalWrite(pin, HIGH);
}

void ToggleMPU(int IMU_NUM)
{
  int pin = PinSelector(IMU_NUM);
  digitalWrite(pin, LOW);
}

void SetDMPInterrupt(int IMU_NUM)
{
  static unsigned long _ETimer[num_sens];

  if (millis() - _ETimer[IMU_NUM] >= (10)) 
  {  // After 10ms, enable MPU Interrupt
    _ETimer[IMU_NUM] += (10);       // Increment Timer by 10ms
    mpu_interrupt[IMU_NUM] = true;  // Set MPU Interrupt to true
  }
  if (mpu_interrupt[IMU_NUM]) 
  {  // If MPU Interrupt is true
    GetDMP(IMU_NUM);        // Get DMP Data
  }
}

void GetDMP(int IMU_NUM)
{

  /// Local Variables -- FIFO Count, Packet Size, Last Good Packet Time
  static unsigned long LastGoodPacketTime[num_sens];
  mpu_interrupt[IMU_NUM] = false;
  fifo_alive[IMU_NUM] = 1;
  fifo_count[IMU_NUM] = MPU[IMU_NUM].getFIFOCount();

  /// If FIFO Count is 0 or not a multiple of packet size
  if ((!fifo_count[IMU_NUM]) || (fifo_count[IMU_NUM] % packet_size[IMU_NUM])) {
    MPU[IMU_NUM].resetFIFO();                           /// Failed to fetch DMP, reset FIFO

  } else {
    while (fifo_count[IMU_NUM] >= packet_size[IMU_NUM]) {           /// While FIFO Count is greater than packet size
      MPU[IMU_NUM].getFIFOBytes(fifo_buffer[IMU_NUM], packet_size[IMU_NUM]);  /// Get latest packet
      fifo_count[IMU_NUM] -= packet_size[IMU_NUM];              /// Update FIFO Count
    }
    LastGoodPacketTime[IMU_NUM] = millis();
    ComputeData(IMU_NUM);     /// On Success, Calculate MPU Math
  }
}

void ComputeData(int IMU_NUM)
{
  /// Quaternion, Gravity, Yaw-Pitch-Roll
  float ypr[3];
  MPU[IMU_NUM].dmpGetQuaternion(&quat[IMU_NUM], fifo_buffer[IMU_NUM]);
  MPU[IMU_NUM].dmpGetGravity(&gravity[IMU_NUM], &quat[IMU_NUM]);
  MPU[IMU_NUM].dmpGetYawPitchRoll(ypr, &quat[IMU_NUM], &gravity[IMU_NUM]);

  /// Acceleration, Gyro
  MPU[IMU_NUM].dmpGetAccel(&accel[IMU_NUM], fifo_buffer[IMU_NUM]);  /// Get Acceleration: x, y, z (gForces: m/s^2)
  MPU[IMU_NUM].dmpGetGyro(&gyro[IMU_NUM], fifo_buffer[IMU_NUM]);    /// Get Gyro: x, y, z (rad/s)

  /// Update Yaw-Pitch-Roll (radians)
  rpy[IMU_NUM][0] = ypr[2];   /// Roll
  rpy[IMU_NUM][1] = ypr[1];   /// Pitch
  rpy[IMU_NUM][2] = ypr[0];   /// Yaw

  time_stamp[IMU_NUM] = millis();

  PrintData(IMU_NUM);
}

void PrintData(int IMU_NUM)
{
  Serial.print("$ ID: "); Serial.print(IMU_NUM);        Serial.print(" | ");
  Serial.print("Tms:\t"); Serial.print(time_stamp[IMU_NUM]);  Serial.print(" | ");
  Serial.print("acc:\t"); Serial.print(accel[IMU_NUM].x);
  Serial.print(",\t");  Serial.print(accel[IMU_NUM].y);
  Serial.print(",\t");  Serial.print(accel[IMU_NUM].z);   Serial.print("\t| ");

  Serial.print("rot:\t"); Serial.print(gyro[IMU_NUM].x);
  Serial.print(",\t");  Serial.print(gyro[IMU_NUM].y);
  Serial.print(",\t");  Serial.print(gyro[IMU_NUM].z);    Serial.print("\t| ");

  Serial.print("rpy:\t"); Serial.print(rpy[IMU_NUM][0]);
  Serial.print(",\t");  Serial.print(rpy[IMU_NUM][1]);
  Serial.print(",\t");  Serial.println(rpy[IMU_NUM][2]);
}

void setup() 
{
  Serial.begin(115200);
  while (!Serial)
  {
  
  }

  //I2C init
  Wire.begin();
  Wire.setClock(400000);

  //Configure AD0 for all sensors
  for ( int pin = PinSelector(0); 
        pin <= PinSelector(NUM_OF_SENSORS - 1); 
        pin++ ) 
  {
    pinMode(pin, OUTPUT);       /// Configure AD0 Pins as Output
    digitalWrite(pin, HIGH);    /// Set AD0 Pins to HIGH
  }

  //MPU6050 init
  MPU6050init();
}

void loop()
{
  for ( int imu = 0; 
        imu < NUM_OF_SENSORS; 
        imu++ )
  {
    ToggleMPU(imu);         // Toggle MPU
    SetDMPInterrupt(imu);   // Set DMP Interrupt for the IMU
    UnToggleMPU(imu);       // Untoggle MPU
  }
  Serial.println();
}
