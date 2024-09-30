#include <ESP32QRCodeReader.h>
#include <SPI.h>

// See available models on README.md or ESP32CameraPins.h
ESP32QRCodeReader reader(CAMERA_MODEL_AI_THINKER);

void onQrCodeTask(void *pvParameters)
{
  struct QRCodeData qrCodeData;

  while (true)
  {
    if (reader.receiveQrCode(&qrCodeData, 100))
    {
      // QR код обнаружен, отправляем 1
      digitalWrite(SS, LOW);
      SPI.transfer(1);
      digitalWrite(SS, HIGH);
    }
    else
    {
      // QR код не обнаружен, отправляем 0
      digitalWrite(SS, LOW);
      SPI.transfer(0);
      digitalWrite(SS, HIGH);
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

void setup()
{
  Serial.begin(115200);
  Serial.println();
  
  SPI.begin();
  pinMode(SS, OUTPUT);

  reader.setup();
  reader.beginOnCore(1);
  xTaskCreate(onQrCodeTask, "onQrCode", 4 * 1024, NULL, 4, NULL);
}

void loop() {

}