#include "I2Cdev.h"              /// I2C Library
#include "MPU6050_6Axis_MotionApps612.h"  /// 6 Axis Motion Apps Library


/** ==========[ VARIABLES ]========== **/
// Sensor Details
const int MPU_ADDR = 0x68;            /// I2C address of the MPU-6050; AD0(HIGH) -> 0x69 | AD0(LOW) -> 0x68
const int num_sens = 3;             /// Number of sensors used

// MPU Data Variables
MPU6050 MPU[num_sens];              /// Array of MPU6050 Objects
VectorInt16 accel[num_sens], gyro[num_sens];  /// Accelerometer & Gyroscope Data
Quaternion quat[num_sens];            /// Quaternion
VectorFloat gravity[num_sens];          /// Gravity
float rpy[num_sens][3];             /// Roll-Pitch-Yaw Data
unsigned long time_stamp[num_sens];       /// Time Stamp (ms)

/// DMP Variables
uint16_t packet_size[num_sens];         /// DMP Packet Size
uint16_t fifo_count[num_sens];          /// FIFO Count
uint8_t fifo_buffer[num_sens][64];        /// FIFO Buffer

/// Interrupt Variables
bool mpu_interrupt[num_sens];         /// MPU Interrupt
uint16_t fifo_alive[num_sens];          /// FIFO Alive


/** ==========[ SETUP ]========== **
 *  Initial Setup for Arduino & MPU6050
 */
void setup()
{
  /// Initialize Serial
  Serial.begin(115200);
  while (!Serial)
    ; /// Wait for Serial to be ready

  /// Initialize I2C
  Wire.begin();
  Wire.setClock(400000); // 400kHz I2C clock.

  /// Configure AD0 Pins for All MPUs
  for (int pin = PinSelector(0); pin <= PinSelector(num_sens - 1); pin++) {
    pinMode(pin, OUTPUT);     /// Configure AD0 Pins as Output
    digitalWrite(pin, HIGH);    /// Set AD0 Pins to HIGH
  }

  /// Initialize All MPU6050 and Enable DMP
  for (int imu = 0; imu < num_sens; imu++) {
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


/** ==========[ LOOP ]========== **
 *  Main Loop for Arduino
 */
void loop()
{
  /// Cycle through all MPUs and Set DMP Interrupts
  for (int imu = 0; imu < num_sens; imu++)
  {
    ToggleMPU(imu);     /// Toggle MPU
    SetDMPInterrupt(imu);   /// Set DMP Interrupt for the IMU
    UnToggleMPU(imu);     /// Untoggle MPU
  }
   Serial.println();


}


/** ==========[ SET DMP INTERRUPT ]========== **
 *  Set DMP Interrupt for the IMU
 *  @param IMU_NUM IMU Number
 */
void SetDMPInterrupt(int IMU_NUM)
{
  static unsigned long _ETimer[num_sens];

  if (millis() - _ETimer[IMU_NUM] >= (10)) {  /// After 10ms, enable MPU Interrupt
    _ETimer[IMU_NUM] += (10);       /// Increment Timer by 10ms
    mpu_interrupt[IMU_NUM] = true;      /// Set MPU Interrupt to true
  }
  if (mpu_interrupt[IMU_NUM]) {       /// If MPU Interrupt is true
    GetDMP(IMU_NUM);            /// Get DMP Data
  }
}


/** ==========[ GET DMP ]========== **
 *  Get DMP Data from MPU6050
 *  If FIFO Count is 0 or not a multiple of packet size, reset FIFO
 *  Else, Proceed with Calculations
 *  @param IMU_NUM IMU Number
 */
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


/** ========== [ MPU MATH ] ========== **
 *  Using DMP, Obtain Accel/Gyro Data
 *  Compute Yaw-Pitch-Roll
 *  @param IMU_NUM IMU Number
*/
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


/** ==========[ PRINT DATA ]========== **
 * Prints the Data for the corresponding IMU
 * format: "> ID: 0 | Tms: 0 | acc: 0.00, 0.00, 0.00 | gyro: 0.00, 0.00, 0.00 | rpy: 0.00, 0.00, 0.00"
 *  @param IMU_NUM IMU Number
 */
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


/** ==========[ TOGGLE MPU ]========== **
 *  Sets corresponding AD0 --> LOW
 *  Toggling the MPU | I2C Address --> 0x68
 *  @param IMU_NUM IMU Number
 */
void ToggleMPU(int IMU_NUM)
{
  int pin = PinSelector(IMU_NUM);
  digitalWrite(pin, LOW);
  Serial.println("Check");
}


/** ==========[ UNTOGGLE MPU ]========== **
 *  Sets corresponding AD0 --> HIGH
 *  Toggling the MPU | I2C Address --> 0x69
 *  @param IMU_NUM IMU Number
 */
void UnToggleMPU(int IMU_NUM)
{
  int pin = PinSelector(IMU_NUM);
  digitalWrite(pin, HIGH);
}


/** ==========[ PIN SELECTOR ]========== **
 *  Returns the Pin Number corresponding to the IMU
 *  @param IMU_NUM IMU Number
 *  @return GPIO Pin Number (AD0)
 */
int PinSelector(int IMU_NUM)
{
  switch(IMU_NUM) {
    case 0:   return 1;
    case 1:   return 3;
    case 2:   return 15;
    case 3:   return 5;
    case 4:   return 6;
    case 5:   return 7;
    case 6:   return 8;
    case 7:   return 9;
    case 8:   return 10;
    case 9:   return 11;
  }
}
