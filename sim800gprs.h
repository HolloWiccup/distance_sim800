#ifndef _sim800gprs_h
#define _sim800gprs_h

enum State {
  SETUP_GPRS,
  SETUP_APN,
  SETUP_USER,
  SETUP_PWD,
  START_GPRS,
  STOP_GPRS,
  HTTP_INIT,
  HTTP_PROFILE,
  HTTP_URL,
  HTTP_CONTENT,
  HTTP_LENGTH,
  HTTP_DATA,
  HTTP_ACTION,
  HTTP_PERM,
  RECONNECT,
  DISCONNECT,
  WAITING
};

class SIM800GPRS {
  public:
    SIM800GPRS(Stream* GPRS, Stream* SerialDebug) {
      _GPRS = GPRS;
      _SerialDebug = SerialDebug;
    }
    void setURL(String url, int port) {
      _serverURL = url + ":" + String(port);
    }

    void setInterval(uint32_t interval) {
      _interval = interval;
    }

    void sendRequest(char* message) {
      if (checkReadyToSend() && millis() -  _timeLastSend > _interval) {
        httpRequst(message);
      }
    }

    bool connectAvailable() {
      if (millis() - _timer > _delayCommand && !_connect) {
        connectModem();
      }
      return _connect;
    }

  private:
    State _currentState = SETUP_GPRS;
    String _serverURL = "91.122.217.111:5082";
    bool _connect = false;
    int _delayCommand = 500;
    int _timer = 0;
    uint32_t _timeLastSend = 0;
    uint32_t _interval = 30000;

    Stream* _GPRS;
    Stream* _SerialDebug;

    bool checkReadyToSend() {
      bool isReady = false;
      if (millis() - _timer > _delayCommand && _connect) {
        isReady = true;
        _timer = millis();
      }
      return isReady;
    }

    String sendCommand(String command) {
      _GPRS->println(command);

      String response = "";
      while (_GPRS->available()) {
        response += _GPRS->readString();
      }
      _SerialDebug->println(response);

      return (response);
    }

    void httpRequst(char* message) {
      byte len = strlen(message);

      switch (_currentState) {
        case HTTP_LENGTH:
          sendCommand("AT+HTTPDATA=" + String(len) + ",10000");  //33 was "AT+HTTPDATA=15,10000"
          _currentState = HTTP_DATA;
          break;
        case HTTP_DATA:
          sendCommand(message);
          _currentState = HTTP_ACTION;
          break;
        case HTTP_ACTION:
          String response = sendCommand("AT+HTTPACTION=1");
          if (response.indexOf("OK") == -1) _currentState = RECONNECT;
          else {
            _currentState = HTTP_LENGTH;
            _timeLastSend = millis();
          }
          break;
      }
    }

    void disconnectModem() {
      _connect = false;
      sendCommand("AT+CIPSHUT");
    }

    void connectModem() {
      switch (_currentState) {
        case SETUP_GPRS:
          sendCommand("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"");
          _currentState = SETUP_APN;
          break;
        case SETUP_APN:
          sendCommand("AT+SAPBR=3,1,\"APN\",\"internet.beeline.ru\"");
          _currentState = SETUP_USER;
          break;
        case SETUP_USER:
          sendCommand("AT+SAPBR=3,1,\"USER\",\"beeline\"");
          _currentState = SETUP_PWD;
          break;
        case SETUP_PWD:
          sendCommand("AT+SAPBR=3,1,\"PWD\",\"beeline\"");
          _currentState = SETUP_PWD;
          break;
        case START_GPRS:
          sendCommand("AT+SAPBR=1,1");
          _currentState = HTTP_INIT;
          break;
        case HTTP_INIT:
          sendCommand("AT+HTTPINIT");
          _currentState = HTTP_PROFILE;
          break;
        case HTTP_PROFILE:
          sendCommand("AT+HTTPPARA=\"CID\",1");
          _currentState = HTTP_URL;
          break;
        case HTTP_URL:
          sendCommand("AT+HTTPPARA=\"URL\",\"" + _serverURL + "\"");
          _currentState = HTTP_LENGTH;
          _connect = true;
          break;
        case RECONNECT:
          disconnectModem();
          _currentState = SETUP_APN;
          break;
        case DISCONNECT:
          disconnectModem();
          _currentState = SETUP_APN;
          break;
      }
    }
};

#endif
