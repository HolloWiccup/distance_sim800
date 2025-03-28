#ifndef _filterDistance_h
#define _filterDistance_h
#include <Arduino.h>


template <int _SIZE>
class FilterDistance {
  public:
    FilterDistance(
      int TRIG_PIN,
      int ECHO_PIN,
      int MAX_DISTANCE = 400,
      int DELAY_MICROS = 20
    )
    {
      _TRIG_PIN = TRIG_PIN;
      _ECHO_PIN = ECHO_PIN;
      _MAX_DISTANCE = MAX_DISTANCE;
      _DELAY_MICROS = DELAY_MICROS;
      pinMode(_TRIG_PIN, OUTPUT);
      pinMode(_ECHO_PIN, INPUT);
      _ms = millis();
    }

    int getDistance(float temp = 60.5) {
      if (millis() - _ms < _interval) return buffer[_SIZE / 2];

      float k;

      digitalWrite(_TRIG_PIN, HIGH);
      delayMicroseconds(_DELAY_MICROS);
      digitalWrite(_TRIG_PIN, LOW);

      float dist = pulseIn(_ECHO_PIN, HIGH) / temp;
      if (dist > _MAX_DISTANCE) dist = _MAX_DISTANCE;

      if (abs(dist - _filVal) > 3) k = 0.9;
      else k = 0.03;

      _filVal += (dist - _filVal) * k;

      buffer[_count] = _filVal;
      if ((_count < _SIZE - 1) && (buffer[_count] > buffer[_count + 1])) {
        for (int i = _count; i < _SIZE - 1; i++) {
          if (buffer[i] > buffer[i + 1]) {
            float buff = buffer[i];
            buffer[i] = buffer[i + 1];
            buffer[i + 1] = buff;
          }
        }
      } else {
        if ((_count > 0) && (buffer[_count - 1] > buffer[_count])) {
          for (int i = _count; i > 0; i--) {
            if (buffer[i] < buffer[i - 1]) {
              float buff = buffer[i];
              buffer[i] = buffer[i - 1];
              buffer[i - 1] = buff;
            }
          }
        }
      }
      if (++_count >= _SIZE) _count = 0;

      _ms = millis();
      int val = buffer[_SIZE / 2] < _filVal / 2 ? _filVal : buffer[_SIZE / 2];
      return val;
    }

  private:
    int _TRIG_PIN;
    int _ECHO_PIN;
    int _MAX_DISTANCE;
    int _DELAY_MICROS;
    int _count = 0;
    int _filVal = 0;
    int _ms = 0;
    int _interval = 50;
    float buffer[_SIZE];
};

#endif
