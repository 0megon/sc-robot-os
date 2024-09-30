#include <fstream>
#include <wiringPi.h>
#include <iostream>
#include <chrono>
#include <thread>
#include <atomic>
#include <string>
#include <sstream>
#include <vector>
#include <cstdlib>
#include <math.h>

#define _USE_MATH_DEFINES

namespace rTypes 
{
  /* speed_t is a double from [0.0; 1.0]
  // interp. velocity of a robot in meters per second
  */
  typedef double speed_t;

  /* acceleration_t is a double
  // interp. velocity of a robot in meters per second
  */
  typedef double acceleration_t;


  /* degree_t is an integer
  // interp. -
  */
  typedef int degree_t;


  /* voltage is a float from [0.0; 5.0]
  // interp. voltage that is aplied to the BLDC's drivers in volts
  */
  typedef double voltage_t;

  /* time_t is an double
  // interp. -
  */
  typedef double time_t;
  
  /* meter_t is a double
  // interp. -
  */
  typedef double meter_t;

  /* code_t is a 1 byte integer
  // interp. a digit used for warnings and notifications about faults
  */
  typedef char code_t;
}

constexpr rTypes::meter_t b{1.0}; // distsnce between tracks, m
constexpr double chargeThreashold{50.0}; // when robot is charged, %
constexpr rTypes::acceleration_t driverAcceleration{0.25}; // the acceleration set on BLDC controllers
constexpr int leftSpeedPin{1};  
constexpr int rightSpeedPin{2};
constexpr int leftDirectoinPin{3};  
constexpr int rightDirectionPin{4};
constexpr int batteryInPin{5};
constexpr rTypes::voltage_t maxBatteryCharge{25.0};
constexpr rTypes::speed_t maxSpeed{0.5};
constexpr rTypes::speed_t noSpeed{0.0};
constexpr int sToVCoef{5}; // speed to voltage

rTypes::code_t g_errorStatus{0};
bool g_resetErrors{0};


class Timer 
{
public:
  Timer() : running(false), paused(false) {}

  void start() 
  {
    running = true;
    paused = false;
    start_time = std::chrono::steady_clock::now();
    timer_thread = std::thread(&Timer::run, this);
  }

  void pause() 
  {
    paused = true;
    pause_time = std::chrono::steady_clock::now();
  }

  void resume() 
  {
    paused = false;
    start_time += std::chrono::steady_clock::now() - pause_time;
  }

  void stop() 
  {
    running = false;
    if (timer_thread.joinable()) 
    {
      timer_thread.join();
    }
  }

  double getElapsedTime() 
  {
    if (paused) {
      return std::chrono::duration<double>(pause_time - start_time).count();
    } else {
      auto now = std::chrono::steady_clock::now();
      return std::chrono::duration<double>(now - start_time).count();
    }
  }

  ~Timer() {
    stop();
  }

private:
  void run()
  {
    while (running) 
    {
      if (!paused) 
      {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration<double>(now - start_time).count();
        std::cout << "Elapsed time: " << elapsed << " seconds\n";
        std::this_thread::sleep_for(std::chrono::seconds(1));
      }
    }
  }

  std::atomic<bool> running;
  std::atomic<bool> paused;
  std::chrono::steady_clock::time_point start_time;
  std::chrono::steady_clock::time_point pause_time;
  std::thread timer_thread;
};

rTypes::voltage_t speedToVoltage(rTypes::speed_t s) {
  return s * sToVCoef;
}

rTypes::time_t calcTurnTime(rTypes::degree_t d, rTypes::speed_t s) 
{
  return sqrt(2 * (fabs(d) * M_PI / 180) * (b / 2) / driverAcceleration);
}

// assumes the acceleration is set to the same values on both BLDC controllers
rTypes::time_t calcPathTime(rTypes::meter_t path, rTypes::speed_t s) 
{
  return path / fabs(s) + 2 * path / driverAcceleration;
}

void setDirectionForward()
{
  digitalWrite(leftDirectoinPin, LOW);
  digitalWrite(rightDirectionPin, LOW);
}

void setDirectionBackward()
{
  digitalWrite(leftDirectoinPin, HIGH);
  digitalWrite(rightDirectionPin, HIGH);
}

void setDriveSpeed(rTypes::speed_t s) {
  digitalWrite(leftSpeedPin, speedToVoltage(s));
  digitalWrite(rightSpeedPin, speedToVoltage(s));
}

void setDriveSpeed(rTypes::speed_t s1, rTypes::speed_t s2) 
{
  digitalWrite(leftSpeedPin, speedToVoltage(s1));
  digitalWrite(rightSpeedPin, speedToVoltage(s2));
}

bool detectedObstacle() 
{
  std::ifstream file("../database/sensor.txt");
  int code{0};
  file >> code;
  file.close();
  return static_cast<bool>(code);
}

bool shortCircuitDetected()
{
  std::ifstream file("../database/hall_sensor_output.txt");
  int code{0};
  file >> code;
  file.close();
  return static_cast<bool>(code);
}

// TODO
double getRobotChargePer()
{  
  return digitalRead(batteryInPin) / maxBatteryCharge * 100;
}

bool isRobotCharged() 
{
  if(getRobotChargePer() < chargeThreashold)
    return false;
  return true;
}

void moveRobotForward(rTypes::meter_t path, rTypes::speed_t s = maxSpeed, bool obstructionDetection = true) {
  if(s < 0)
    setDirectionForward();
  else 
    setDirectionBackward();
  rTypes::time_t pathTime{calcPathTime(path, s)};
  std::cout << "Required time: " << pathTime << " seconds to traverse " <<  path << " meters.";
  Timer timer;
  setDriveSpeed(s);
  timer.start();
  char errorStatus{'0'};
  while(timer.getElapsedTime() < pathTime)
  {
    if(detectedObstacle() && obstructionDetection && errorStatus == '0') 
    {
      setDriveSpeed(noSpeed);
      timer.pause();
      std::ofstream errFile("cs/err.txt");
      if (errFile.is_open()) 
      {
        errFile << '1';
        errFile.close();
        errorStatus = '1';
      } else 
      {
        std::cerr << "Не удалось открыть файл для записи" << std::endl;
      }
    }

    if(errorStatus == '1') 
    {
      std::fstream rstFile("cs/rst.txt", std::ios::in | std::ios::out);
      std::ofstream errFile("cs/err.txt");
      if(rstFile.is_open() && errFile.is_open())
      {
        std::string line;
        std::getline(rstFile, line);
        char rst = line[0];
        if(rst == '1')
        {
          rstFile << '0';
          errFile << '0';
          rstFile.close();
          errFile.close();

          errorStatus = 0;
          setDriveSpeed(maxSpeed);
          timer.resume();

        } else 
        {
          std::cerr << "Не удалось открыть файл для записи" << std::endl;
        }
      }
    } 
  }
  setDriveSpeed(noSpeed);
  setDirectionForward(); // for lowering current consumption
  // no need to stop the timer manually
}

void moveRobotBackward(rTypes::meter_t path, rTypes::speed_t s = maxSpeed, bool obstructionDetection = true)
{
  moveRobotForward(path, -s, obstructionDetection);
}

void turnRobotInPlace(rTypes::degree_t d, rTypes::speed_t s = maxSpeed) 
{
  rTypes::time_t timeOfTurn = calcTurnTime(d, s);
  Timer timer;
  std::cout << "Required time: " << timeOfTurn << " to rotate " <<  d << " degrees.";
  timer.start();
  if(d > 0)
    setDriveSpeed(noSpeed, s);
  else if(d < 0)
    setDriveSpeed(s, noSpeed);
  else
    timeOfTurn = 0.0;
  while(timer.getElapsedTime() < timeOfTurn) 
    ;
  setDriveSpeed(noSpeed);
  // no need to stop the timer manually
}



void processLine(const std::string& line, std::string& command, double& value) 
{
  std::istringstream iss(line);
  std::getline(iss, command, '=');
  iss >> value;
}

void removeFirstLine(const std::string& filename) 
{
  std::ifstream file(filename);
  std::vector<std::string> lines;
  std::string line;

  while (std::getline(file, line)) 
    lines.push_back(line);
  
  file.close();

  if (!lines.empty()) 
  {
    lines.erase(lines.begin());
  }

  std::ofstream outFile(filename);
  for (const auto& l : lines) 
  {
    outFile << l << std::endl;
  }

  outFile.close();
}

int main(int argc, char* argv[])
{ 
  pinMode(leftSpeedPin, OUTPUT); 
  pinMode(rightSpeedPin, OUTPUT); 
  pinMode(leftDirectoinPin, OUTPUT); 
  pinMode(rightDirectionPin, OUTPUT); 

  if (argc > 2)
  {
    std::cout << "Error: invalid number of arguments!\n";
    return 1;
  }

// With predefined algorithm:
   if(argc == 2 && argv[1] == "-p") 
  {
    while(!isRobotCharged())
      std::cout << "Robot does not have a sufficient charge level:" << getRobotChargePer() << "%\n";

    /*Set a custom user algorithm here. 
    Options:  moveRobotForward(m);
              moveRobotBackward(m);
              turnRobotInPlace(d);
    Where:  m - a number of meters (integer),
            d - a number of degreees (integer).
    */

    return 0;
  }
  
// From the web interface:
  if(argc == 1 || (argc == 2 && argv[1] == "-m"))
  {
    while (1) 
    {
      while(!isRobotCharged())
      	std::cout << "Robot does not have a sufficient charge level:" << getRobotChargePer() << "%\n";

      std::string filename = "program-calls.txt";
      std::string command;
      double value;

      std::ifstream file(filename);
      if (!file.is_open()) 
      {
        std::cerr << "Unable to open file" << std::endl;
        return 1;
      }

      std::string line;
      if (std::getline(file, line)) 
      {
        processLine(line, command, value);
        std::cout << "Command: " << command << ", Value: " << value << std::endl;
        removeFirstLine(filename);
      } 
      file.close();
      
      if(command == "fwd")
      	moveRobotForward(value);
      else if (command == "bwd")
	moveRobotBackward(value);
      else if (command == "rot")
        turnRobotInPlace(static_cast<rTypes::degree_t>(value));
      
    }
  } else 
  {
    std::cout << "Error: invalid arguments";
    return 1;
  }

  return 0;
}
