#include <SoftwareSerial.h>
#include "filterDistance.h"
#include "sim800gprs.h"

#define SIM_RX_PIN 11  // Подключен к TXD SIM800C
#define SIM_TX_PIN 10  // Подключен к RXD SIM800C

#define TRIG_PIN 2
#define ECHO_PIN 3

#define SIZE_READ 64 // количество усреднений для средних арифм. фильтров
#define DELAY_MICROS 10 // для hc sr04 10мкс, для jsn sr04 20мкс
#define MAX_DISTANCE 530 // максимальная дистанция которую может считать датчик

#define PORT 5082
#define SERVER_IP "91.122.217.111"

SoftwareSerial GPRS(SIM_RX_PIN, SIM_TX_PIN);  //Digital Pin 7 = TX, Digital Pin 8 = RX
FilterDistance<SIZE_READ> sensor(TRIG_PIN, ECHO_PIN, MAX_DISTANCE, DELAY_MICROS);

SIM800GPRS modem(&GPRS, &Serial);

uint32_t ms, timer;
uint32_t intervalSend = 30000;
int intervalSensor = 200;

int distanceValue = 0;

char data[50];

void createMessage() {
  sprintf(data, "%s%d%s%d\"}",
          "{\"Api\":\"lvl\",\"Value\":\"",
          distanceValue < 100, "\",\"Dist\":\"",
          (int)distanceValue);
}

void setup() {
  delay(2000);

  modem.setInterval(intervalSend);
  modem.setURL(SERVER_IP, PORT);

  GPRS.begin(19200);
  Serial.begin(9600);
  Serial.println("Serial ready");

  delay(1000);
}

void loop() {
  ms = millis();
  //  if (modem.connectAvailable()) {
  //    createMessage();
  //    modem.sendRequest(data);
  //  }

  if (ms - timer > intervalSensor) {
    distanceValue = sensor.getDistance();
    timer = ms;
    Serial.println(distanceValue);
  }
}
