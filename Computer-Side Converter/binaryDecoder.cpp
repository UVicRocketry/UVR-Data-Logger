/* Uses the LSMDS0 libary to retrive and convert binary files created by binary-data-logger.ino
  Due to the nature of binary files, you will have to match the structs here to the ones you use in the logger
  By using an offloaded converter, we can save space on the arduino and save time in retriving data.
*/
#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iterator>
#include <string>
#define BASE_NAME "data01";
//Values adapted from LSM9DS0 library to use in converting raw data to proper types
//----------------------------------------------------
#define SENSORS_GRAVITY_STANDARD       (9.80665F)
#define LSM9DS0_ACCEL_MG_LSB_2G        (0.061F)
#define LSM9DS0_ACCEL_MG_LSB_4G        (0.122F)
#define LSM9DS0_ACCEL_MG_LSB_6G        (0.183F)
#define LSM9DS0_ACCEL_MG_LSB_8G        (0.244F)
#define LSM9DS0_ACCEL_MG_LSB_16G       (0.732F)
#define LSM9DS0_GYRO_DPS_DIGIT_245DPS  (0.00875F)
#define LSM9DS0_GYRO_DPS_DIGIT_500DPS  (0.01750F)
#define LSM9DS0_GYRO_DPS_DIGIT_2000DPS (0.07000F)
#define RAW_TEMP_TO_C (0.0078125)
//----------------------------------------------------
//Set to the same value which you grab
const float accel_mg_lsb = LSM9DS0_ACCEL_MG_LSB_2G;
const float gyro_dps_digit = LSM9DS0_GYRO_DPS_DIGIT_245DPS;
//create a data holder object and store in memory as a continous item
#pragma pack(1)
struct data_t {
  std::uint32_t ms;
  std::int16_t accelX;
  std::int16_t accelY;
  std::int16_t accelZ;
  std::int16_t gyroX;
  std::int16_t gyroY;
  std::int16_t gyroZ;
  std::int16_t temp;
};

const int BLOCK_SIZE = 512;
const int DATA_DIM = (BLOCK_SIZE - 4) / sizeof(data_t);
const int FILL_DIM = (BLOCK_SIZE - 4) - sizeof(data_t) * DATA_DIM;
//create a block object and store in memory as a continous item
#pragma pack(1)
struct block_t {
  std::uint16_t count;
  std::uint16_t overrun;
  data_t data[DATA_DIM];
  std::uint8_t fill[FILL_DIM];
};

//for retriving info from block
std::istream& operator>>(std::istream& stream, const data_t& data) {
  stream.read((char*)&data, sizeof(data_t));
  return stream;
}

//for retiving block data from the binary file
std::istream& operator>>(std::istream& stream, const block_t& block) {
  stream.read((char*)&block, sizeof(block_t));
  return stream;
}

//for outputting data holder types, we want to have a standard format when we pass it, as well as convert the values in the data struct
std::ostream& operator<<(std::ostream& stream, const data_t& data) {
  stream << data.ms << ",";
  stream << data.accelX * accel_mg_lsb * SENSORS_GRAVITY_STANDARD / 1000 << ",";
  stream << data.accelY * accel_mg_lsb * SENSORS_GRAVITY_STANDARD / 1000 << ",";
  stream << data.accelZ * accel_mg_lsb * SENSORS_GRAVITY_STANDARD / 1000 << ",";
  stream << data.gyroX * gyro_dps_digit << ",";
  stream << data.gyroY * gyro_dps_digit << ",";
  stream << data.gyroZ * gyro_dps_digit;
  stream << data.temp * RAW_TEMP_TO_C<< ",";
  return stream;
}

//for printing block types we wish to only pass the data struct
std::ostream& operator<<(std::ostream& stream, const block_t& block) {
  for (const data_t* data = block.data; data < block.data + block.count; data++)
    stream << *data << "\n";
  return stream;
}

int main() {
  //prep files for write/read
  std::ifstream bin("data01.bin", std::ifstream::binary);
  std::ofstream csv("data.csv");
  //preform the transfer using the above
  std::copy(
    std::istream_iterator<block_t>(bin),
    std::istream_iterator<block_t>(),
    std::ostream_iterator<block_t>(csv));
}
