#include <iostream>
#include <fstream>
#include <wiringPi.h>

#define HALL_SENSOR_PIN 0 // Пин, к которому подключен датчик Холла

int main() {
    // Инициализация библиотеки WiringPi
    if (wiringPiSetup() == -1) {
        std::cerr << "Ошибка инициализации WiringPi" << std::endl;
        return 1;
    }

    // Установка пина датчика Холла как входного
    pinMode(HALL_SENSOR_PIN, INPUT);

    // Открытие файла для записи
    std::ofstream outputFile("../database/hall_sensor_output.txt");
    if (!outputFile.is_open()) {
        std::cerr << "Ошибка открытия файла" << std::endl;
        return 1;
    }

    while (true) {
        // Чтение значения с датчика Холла
        int sensorValue = digitalRead(HALL_SENSOR_PIN);

        // Запись значения в файл
        if (sensorValue == HIGH) {
            outputFile << "1";
            std::cout << "1\n";
        } else {
            outputFile << "0";
            std::cout << "0\n";
        }

        // Задержка перед следующим чтением
        delay(1000); // 1 секунда
    }

    // Закрытие файла
    outputFile.close();

    return 0;
}