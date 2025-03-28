#include <SoftwareSerial.h>
#include "filterDistance.h"
#include "sim800gprs.h"

#define SIM_RX_PIN 11  // Подключен к TXD SIM800C
#define SIM_TX_PIN 10  // Подключен к RXD SIM800C

#define TRIG_PIN 2
#define ECHO_PIN 3

#define SWITCH_PIN 6

#define SIZE_READ 64 // количество усреднений для средних арифм. фильтров
#define DELAY_MICROS 10 // для hc sr04 10мкс, для jsn sr04 20мкс
#define MAX_DISTANCE 530 // максимальная дистанция которую может считать датчик

#define INTERVAL_SENSOR 200
#define INTERVAL_SEND 30000
#define INTERVAL_SLEEP 300000

String SERVER_URL = "91.122.217.111:5085";

SoftwareSerial GPRS(SIM_RX_PIN, SIM_TX_PIN);  //Digital Pin 7 = TX, Digital Pin 8 = RX

FilterDistance<SIZE_READ> sensor(TRIG_PIN, ECHO_PIN, MAX_DISTANCE, DELAY_MICROS);
SIM800GPRS modem(&GPRS, &Serial, SERVER_URL);

uint32_t ms, timer1, timer2, timerSleep;

int distanceValue = 0;

bool ecoMode = false;
bool flagSleep = false;

void createMessage(String &s) {
  s += F("{\"Api\":\"lvl\",\"Value\":\"");
  s += distanceValue < 100;
  s += F("\",\"Dist\":\"");
  s += distanceValue;
  s += F("\"}");
}

void setup() {
  pinMode(SWITCH_PIN, INPUT);
  if (digitalRead(SWITCH_PIN)) {
    //    ecoMode = true;
  }

  delay(2000);

  GPRS.begin(19200);
  Serial.begin(9600);
  Serial.println("Serial ready");

  delay(1000);
}

void loop() {
  ms = millis();

  if (!flagSleep) {
    modem.start();
    
    timerSleep = ms;
    
    if (ms - timer2 > INTERVAL_SENSOR) {
      distanceValue = sensor.getDistance();
      timer2 = ms;
      Serial.println(distanceValue);
    }
  }


  if (modem.isReadyToSend() && ms - timer1 > INTERVAL_SEND) {
    String s;
    createMessage(s);
    modem.sendRequest(s);
    timer1 = ms;
  }


  if (modem.isSendedRequest()) {
    if (ecoMode) {
      //sleep
      modem.disconnectModem();
      flagSleep = true;
    } else {
      modem.nextSend();
    }
  }

  if(ms - timerSleep > INTERVAL_SLEEP){
    flagSleep = false;
  }
}
