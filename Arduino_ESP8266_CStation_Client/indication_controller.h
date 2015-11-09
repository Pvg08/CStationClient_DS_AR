#ifndef INDICATION_CONTROLLER_H
#define INDICATION_CONTROLLER_H

#define LED_BLUE_PIN 22
#define LED_YELLOW_PIN 24
#define LED_RED_PIN 26

class IndicationController 
{
  private:
    bool led_blue_state;
    bool led_yell_state;
    bool led_red_state;

  public:
    static IndicationController *_self_controller;

    static IndicationController* Instance() {
      if(!_self_controller)
      {
          _self_controller = new IndicationController();
      }
      return _self_controller;
    }
    static bool DeleteInstance() {
      if(_self_controller)
      {
          delete _self_controller;
          _self_controller = NULL;
          return true;
      }
      return false;
    }

    IndicationController() 
    {
      pinMode(LED_BLUE_PIN, OUTPUT);
      pinMode(LED_YELLOW_PIN, OUTPUT);
      pinMode(LED_RED_PIN, OUTPUT);
      setRed(false);
      setYellow(false);
      setBlue(false);
    }

    void setRed(bool state)
    {
      led_red_state = state;
      digitalWrite(LED_RED_PIN, state ? HIGH : LOW);
    }
    void setYellow(bool state)
    {
      led_yell_state = state;
      digitalWrite(LED_YELLOW_PIN, state ? HIGH : LOW);
    }
    void setBlue(bool state)
    {
      led_blue_state = state;
      digitalWrite(LED_BLUE_PIN, state ? HIGH : LOW);
    }

    bool getProgLedState() 
    {
      return led_blue_state;
    }

    void ConfigState(byte state) {
      setRed(state>0);
      setYellow(state>1);
      setBlue(state>1);
    }

    void ConnectState(byte state) {
      setRed(state>0);
      setYellow(state>1);
      setBlue(false);
    }

    void SensorsSendingState(byte state) {
      if (!state && led_red_state) {
        delay(200);
      }
      if(led_red_state || !led_yell_state) setRed(state>0);
      if (!state && led_yell_state) {
        setYellow(false);
      }
    }

    void SensorsSendingSignalState(byte state) {
      SensorsSendingState(state);
    }

    void PresenceState(byte state) {
      if (!led_red_state) setYellow(state>0);
    }

    void OuterState(byte state) {
      if (!led_red_state) setYellow(state>0);
    }

    void ToneState(byte state) {
      setBlue(state>0);
    }

    void SetProgLedState(byte state) {
      setBlue(state>0);
    }
};

IndicationController *IndicationController::_self_controller = NULL;

#endif
