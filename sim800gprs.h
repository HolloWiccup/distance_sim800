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

enum StateSend {
  SENDING,
  WAIT_TO_SEND,
  SENDED,
  SEND,
};

class SIM800GPRS {
  public:
    SIM800GPRS(Stream* GPRS, Stream* SerialDebug, String &URL) {
      _GPRS = GPRS;
      _SerialDebug = SerialDebug;
      _serverURL = URL;
    }

    void start() {
      if (millis() - _timer > _delayCommand) {
        connectModem();
        _timer = millis();
      }
    }

    void disconnectModem() {
      sendCommand("AT+CIPSHUT");
      _currentState = SETUP_GPRS;
    }

    void sendRequest(String &message) {
      _message = message;
      _currentStateSend = SEND;
    }

    bool isReadyToSend() {
      return _currentState == HTTP_LENGTH &&
             _currentStateSend == WAIT_TO_SEND;
    }

    bool isSendedRequest() {
      return _currentStateSend == SENDED;
    }

    void nextSend() {
      _currentStateSend = WAIT_TO_SEND;
    }

  private:
    State _currentState = SETUP_GPRS;
    StateSend _currentStateSend = WAIT_TO_SEND;
    String &_message;
    String &_serverURL;
    bool _connect = false;
    int _delayCommand = 500;
    int _timer = 0;
    uint32_t _timeLastSend = 0;
    uint32_t _interval = 30000;

    Stream* _GPRS;
    Stream* _SerialDebug;

    String sendCommand(String command) {
      _GPRS->println(command);

      String response = "";
      while (_GPRS->available()) {
        response += _GPRS->readString();
      }
      _SerialDebug->println(response);

      return (response);
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
          _currentStateSend = WAIT_TO_SEND;
          break;
        case HTTP_LENGTH:
          if (_currentStateSend == SEND) {
            byte len = _message.length();
            sendCommand("AT+HTTPDATA=" + String(len) + ",10000");  //33 was "AT+HTTPDATA=15,10000"
            _currentState = HTTP_DATA;
            _currentStateSend = SENDING;
          }
          break;
        case HTTP_DATA:
          sendCommand(_message);
          _currentState = HTTP_ACTION;
          break;
        case HTTP_ACTION:
          String response = sendCommand(F("AT+HTTPACTION=1"));
          if (response.indexOf("OK") == -1) _currentState = RECONNECT;
          else {
            _currentState = HTTP_LENGTH;
            _currentStateSend = SENDED;
          }
          break;
        case RECONNECT:
          disconnectModem();
          break;
      }
    }
};

#endif
