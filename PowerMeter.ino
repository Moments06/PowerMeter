#include <Arduino.h>
#include <Wire.h>
#include "INA226.h"
#include "Ticker.h"
#include "Adafruit_ssd1306syp.h"

#define MaxCurrent 8          // 最大电流
#define SamplingResistor 0.05 // 采样电阻的值
#define OLED_SDA_PIN PA_1
#define OLED_SCL_PIN PA_0
#define LED_PIN PB_3
#define BUTTON_PIN PB_6

void getINA226Data();
void updateOled();
void ChargeIntegration();
void led();

float INA_BusVoltage;
float INA_Current_mA;
float INA_Current_A;
float INA_Power;
float INA_Power_mW;
float electricChargeSum = 0;
float lastElectricChargeSum = 0;
int last_t = millis();

Adafruit_ssd1306syp display(OLED_SDA_PIN, OLED_SCL_PIN);
Ticker getINA226DataTicker(getINA226Data, 250, 0);
Ticker updateOledTicker(updateOled, 60, 0);
Ticker integrationTicker(ChargeIntegration, 3, 0);
Ticker ledTicker(led, 150, 0);
INA226 INA(0x40);

void setup()
{
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLDOWN);
  digitalWrite(LED_PIN, HIGH);
  display.initialize();
  display.clear();
  display.update();
  Wire.begin();
  if (INA.begin())
    Serial.println("INA226 is ready!");
  else
    Serial.println("INA226 begin fail");
  INA.setMaxCurrentShunt(MaxCurrent, SamplingResistor);
  getINA226DataTicker.start();
  updateOledTicker.start();
  integrationTicker.start();
  ledTicker.start();
  display.setTextSize(2);
  display.setCursor(24, 18);
  display.print("Moments.");
  display.update();
  delay(2000);
  display.clear();
  display.update();
}

void getINA226Data()
{
  if (INA.getShuntVoltage_mV() > 0 && INA.getBusVoltage() > 0)
  {
    INA_BusVoltage = INA.getBusVoltage();
    INA_Current_mA = INA.getShuntVoltage_mV() / SamplingResistor;
    INA_Current_A = INA_Current_mA / 1000;
    INA_Power = INA_Current_A * INA_BusVoltage;
    INA_Power_mW = INA_Current_mA * INA_BusVoltage;
  }
}

void updateOled()
{
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.drawRect(0, 0, 128, 64, WHITE);
  display.drawRoundRect(8, 5, 112, 10, 3, WHITE);
  display.setCursor(23, 6);
  display.println("VoltageAmmeter");
  display.setCursor(8, 17);
  display.print("Voltage:");
  display.print(INA_BusVoltage);
  display.print(" V");
  display.setCursor(8, 26);
  display.print("Current:");
  if (INA_Current_mA < 1000)
  {
    display.print(INA_Current_mA);
    display.print(" mA");
  }
  else
  {
    display.print(INA_Current_A);
    display.print(" A");
  }
  display.setCursor(8, 34);
  display.print("Power:  ");
  if (INA_Power_mW < 1000)
  {
    display.print(INA_Power_mW);
    display.print(" mW");
  }
  else
  {
    display.print(INA_Power);
    display.print(" W");
  }
  display.setCursor(8, 42);
  display.print("Capacity: ");
  if (electricChargeSum < 10000)
  {
    display.print(electricChargeSum * 1000 / 3600);
  }
  else
  {
    display.print(electricChargeSum * 1000 / (10000 * 3600));
    display.print("E4");
  }
  display.print("mAh");
  display.update();
  display.clear();
}

void ChargeIntegration()
{
  electricChargeSum = INA_Current_A * (millis() - last_t) / 1000;
  if (digitalRead(BUTTON_PIN))
  {
    electricChargeSum = 0;
  }
}

void led()
{
  if (electricChargeSum - lastElectricChargeSum > 25)
  {
    digitalWrite(LED_PIN, LOW);
    lastElectricChargeSum = electricChargeSum;
  }
  else
  {
    digitalWrite(LED_PIN, HIGH);
  }
}

void loop()
{
  getINA226DataTicker.update();
  updateOledTicker.update();
  integrationTicker.update();
  ledTicker.update();
}