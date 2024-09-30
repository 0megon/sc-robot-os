#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <iostream>
#include <fstream>

#define CHANNEL 0
#define SPEED 50000

int main() {
  if (wiringPiSetup() == -1) {
    std::cerr << "Failed to setup wiringPi" << std::endl;
    return 1;
  }

  if (wiringPiSPISetup(CHANNEL, SPEED) == -1) {
    std::cerr << "Failed to setup SPI" << std::endl;
    return 1;
  }

  unsigned char buffer[1];
  std::ofstream file("..\\database\\qr_code_status.txt");

  if (!file.is_open()) {
    std::cerr << "Failed to open file" << std::endl;
    return 1;
  }

  while (true) {
    buffer[0] = 0x00;  // Отправка пустого байта для получения данных
    wiringPiSPIDataRW(CHANNEL, buffer, 1);
    std::cout << "Received: " << static_cast<int>(buffer[0]) << std::endl;
    file << static_cast<int>(buffer[0]) << std::endl;
    delay(1000);  // Задержка в 1 секунду
  }

  file.close();
  return 0;
}