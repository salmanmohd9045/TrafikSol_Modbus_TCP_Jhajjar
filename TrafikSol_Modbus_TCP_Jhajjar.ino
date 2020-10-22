#define SOFT_VERSION "WMS_MOD-01"

#include <SPI.h>
#include <LiquidCrystal.h>
#include <Ethernet.h>
#include "./src/Mudbus.h"
#include <Arduino.h>
#include "TimerOne.h"
#include "./src/DHT.h"
#include <EEPROM.h>
#include <Wire.h>

Mudbus Mb;
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

EthernetClient client;

//Define Pin Mappings
#define MENU 36
#define NEXT 37
#define ENTER 40
#define EXIT 41
#define brightnessPin 28
#define SerialVis Serial3
#define surfTempPin A0
#define WindVanePin A1
#define WindSensorPin 3
#define DHTPIN 5                //Change this pin
#define DHTTYPE DHT11

#define IP_ADDR 0             //Next four places occupied till address 3

template< typename T, size_t N > size_t ArraySize (T (&) [N]) {
  return N;
}

//Object Declarations
DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal lcd(29, 39, 24, 25, 26, 27);

//Data Buffer Sequence: Temperature, Humidity, Visibility, Surface Temperature, Wind Speed, Wind Direction
uint16_t modbus_data_buffer[6] = {3, 3, 3, 3, 3, 3};
uint32_t lastTime;

//Buffer Input for Visibility
byte ask[] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x01, 0x84, 0x0A};

//Variables Declarations
int lastDirectionValue;                                          //Wind Speed
volatile float windSpeed;
long Rotations;
volatile unsigned long ContactBounceTime;
int visibility, windDirection;                                                  //Visibility
float temperature, humidity, surfaceTemperature;  //Temperature, Humidity, Wind Direction, Surface Temperature
String arr_str_name[6] = {"Temp", "Hum ", " WD ", " WS ", " ST ", " Vis"};
String arr_str_unit[6] = {" *C", " %", " deg", " mph", " *C", " m"};
String arr_str[6];
int counter = 0;      //Print value counter for lcd
String requestString = "";
int ip_array[4] = {169, 254, 0, 5};
int dns_array[4] = {169, 254, 0, 1};

//---------Thermistor Varibales---------------------//

int Vo;
float R1 = 10000;
float logR2, R2, T, Tc, Tf;
float c1 = -0.00297, c2 = 0.000899, c3 = -2.5e-06;

//--------------------------------------------------//


void setup() {
  Ethernet.init(49);
  Serial.begin(9600);
  pinMode(brightnessPin, OUTPUT);
  digitalWrite(brightnessPin, HIGH);

  lcd.begin(16, 2);
  lcd.print(" E&E Solutions");
  lcd.setCursor(2, 1);
  lcd.print("WMS - Modbus");
  delay(3000);

  if (EEPROM.read(IP_ADDR) == 255) {
    EEPROM.write(IP_ADDR, 169);
    EEPROM.write(IP_ADDR + 1, 254);
    EEPROM.write(IP_ADDR + 2, 39);
    EEPROM.write(IP_ADDR + 3, 5);
  }

  //Retrieve configured IP Address from EEPROM
  for (int i = 0; i < 4; i++) {
    ip_array[i] = EEPROM.read(IP_ADDR + i);
    Serial.print(ip_array[i]);
    Serial.print(",");
  }
  Serial.println();

  lcd.clear();
  lcd.print(" Press MENU to");
  lcd.setCursor(0, 1);
  lcd.print("config in");
  long int lastTime = millis();
  while (millis() - lastTime < 10000) {
    if (!digitalRead(MENU))
      deviceSettings();

    if (!digitalRead(EXIT))
      break;

    lcd.setCursor(10, 1);
    lcd.print(9 - (millis() - lastTime) / 1000);
    lcd.print(" sec.");
  }
  lcd.clear();
  lcd.print("  Starting..");

  IPAddress ip(ip_array[0], ip_array[1], ip_array[2], ip_array[3]);
  IPAddress myDns(ip_array[0], ip_array[1], 0, 1);
  Ethernet.begin(mac, ip, myDns);

  SerialVis.begin(9600, SERIAL_8E1);
  dht.begin();

  pinMode(WindSensorPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(WindSensorPin), isr_rotation, FALLING);

  Timer1.initialize(2500000);  // Timer: 2.5 sec
  Timer1.attachInterrupt(isr_timer);

  lastTime = -5010L;
}

void loop() {
  Mb.Run();

  if (millis() - lastTime > 2000) {
    //Refresh the data variables
    getParameterValues();

    requestString = String(round(temperature * 10)) + "$" +
                    String(round(humidity * 10)) + "$" +
                    String(abs(visibility)) + "$" +
                    String(round(surfaceTemperature * 10)) + "$" +
                    String(round(windSpeed * 10)) + "$" +
                    String(windDirection) + "$";
    Serial.println(requestString);

    lcd.clear();
    lcd.print(arr_str_name[counter]);
    lcd.print(":");
    lcd.print(arr_str[counter]);
    lcd.print(arr_str_unit[counter++]);
    lcd.setCursor(0, 1);
    lcd.print(arr_str_name[counter]);
    lcd.print(":");
    lcd.print(arr_str[counter]);
    lcd.print(arr_str_unit[counter++]);
    if (counter > 5) counter = 0;

    Mb.R[0] = abs(temperature * 10);
    Mb.R[1] = abs(humidity * 10);
    Mb.R[2] = visibility;
    Mb.R[3] = abs(surfaceTemperature * 10);
    Mb.R[4] = abs(windSpeed * 10);
    Mb.R[5] = windDirection;

    //update the clock variable
    lastTime = millis();
  }
}
