#include <wiringPi.h>
#include <fstream>
#include <cstdlib>

#define TRIG 0 // GPIO 17
#define ECHO 1 // GPIO 18

long getDistance() 
{
    delay(100);
    long duration, distance;
    digitalWrite(TRIG, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG, LOW);
    
    while (digitalRead(ECHO) == LOW);
    long startTime = micros();
    
    while (digitalRead(ECHO) == HIGH);
    long travelTime = micros() - startTime;
    
    distance = (travelTime / 2) / 0.291; // Расчет расстояния в миллиметрах
    return distance;
}

void writeInFile(int t)
{
    std::ofstream file;
    file.open("../database/sensor.txt", std::ios::out);
    if (!file) {
        std::cerr << "Error opening file!\n";
        std::exit(1);
    }
    file << t;
    file.close();
}

int main(void) 
{
    constexpr int detectionDistance{500}; // 500 mm

    wiringPiSetup();
    pinMode(TRIG, OUTPUT);
    pinMode(ECHO, INPUT);
    digitalWrite(TRIG, LOW);
    delay(30);

    while (1)
        if(getDistance() <= detectionDistance)
        {
            if(getDistance() <= detectionDistance)
                if(getDistance() <= detectionDistance)
                    writeInFile(1);   
        } else
            writeInFile(0);
    return 0;
}
