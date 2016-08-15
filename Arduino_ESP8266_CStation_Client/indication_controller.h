#ifndef INDICATION_CONTROLLER_H
#define INDICATION_CONTROLLER_H

#define LED_BLUE_PIN 22
#define LED_YELLOW_PIN 24
#define LED_RED_PIN 26

#define FAN_PIN 38
#define LIGHT_PIN 39

#define INDICATION_FAN_STATE_ADDR 21
#define INDICATION_LIGHT_STATE_ADDR 22

#define LIGHT_AUTO_SKIP_START_HOUR 7
#define LIGHT_AUTO_SKIP_STOP_HOUR 22
#define LIGHT_AUTO_MAX_LEVEL 3
#define LIGHT_AUTO_TIMEOUT_LENGTH 60000
#define LIGHT_AUTO_ON_ADD 67

#define FAN_FIRST_RUN 80
#define FAN_MIN_PERIOD_OFF_LENGTH 40000
#define FAN_MAX_PERIOD_OFF_LENGTH 180000
#define FAN_MIN_PERIOD_ON_LENGTH 90000
#define FAN_MAX_PERIOD_ON_LENGTH 25000
#define FAN_COUNTER_PER_MINUTE_FOR_PERM_ON 10.0
#define FAN_COUNTER_PER_MINUTE_FOR_MAX 8.0
#define FAN_COUNTER_PER_MINUTE_FOR_MIN 2.0
#define FAN_COUNTER_PER_MINUTE_FOR_PERM_OFF 0.9
#define FAN_MIN_TIMEOUT_CUT 0.2
#define FAN_MAX_TIMEOUT_CUT 1.0

class IndicationController 
{
  private:
    volatile bool led_blue_state;
    volatile bool led_yell_state;
    volatile bool led_red_state;

    bool fan_state;
    bool light_g4_state;
    bool fan_auto_state;
    bool light_g4_auto_state;

    volatile unsigned long int light_last_time_state;

    unsigned int fan_curr_timeout;
    unsigned long int fan_last_time_state;
    volatile unsigned long fan_increment;

    uint16_t last_light_level;
    bool light_level_initialized;
    volatile bool light_need_update;

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
      pinMode(FAN_PIN, OUTPUT);
      pinMode(LIGHT_PIN, OUTPUT);
      setRed(false);
      setYellow(false);
      setBlue(false);
      setLight(false);
      fan_state = false;
      fan_auto_state = true;
      light_g4_auto_state = true;
      fan_curr_timeout = FAN_FIRST_RUN;
      fan_increment = 0;
      fan_last_time_state = 0;
      last_light_level = 0;
      light_last_time_state = 0;
      light_need_update = false;
      light_level_initialized = false;

      EEPROM_Helper::readAutoState(INDICATION_FAN_STATE_ADDR, &fan_auto_state, &fan_state);
      if (!fan_auto_state) {
        setFan(fan_state);
      }
      EEPROM_Helper::readAutoState(INDICATION_LIGHT_STATE_ADDR, &light_g4_auto_state, &light_g4_state);
      if (!light_g4_auto_state) {
        setLight(light_g4_state);
      }
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
    void setLight(bool state)
    {
      light_g4_state = state;
      digitalWrite(LIGHT_PIN, state ? HIGH : LOW);
    }
    void setFan(bool state)
    {
      fan_state = state;
      digitalWrite(FAN_PIN, state ? HIGH : LOW);
    }

    bool getProgLedState() 
    {
      return led_blue_state;
    }

    bool getFanState() {
      return fan_state;
    }

    bool getLightG4State() {
      return light_g4_state;
    }

    void setFanState(bool ison)
    {
      setFan(ison);
      fan_auto_state = false;
      EEPROM_Helper::writeAutoState(INDICATION_FAN_STATE_ADDR, fan_auto_state, fan_state);
    }
    
    void setFanAutoState()
    {
      fan_auto_state = true;
      EEPROM_Helper::writeAutoState(INDICATION_FAN_STATE_ADDR, fan_auto_state, fan_state);
    }

    void setLightG4State(bool ison)
    {
      setLight(ison);
      light_g4_auto_state = false;
      EEPROM_Helper::writeAutoState(INDICATION_LIGHT_STATE_ADDR, light_g4_auto_state, light_g4_state);
    }
    
    void setLightG4AutoState()
    {
      light_g4_auto_state = true;
      EEPROM_Helper::writeAutoState(INDICATION_LIGHT_STATE_ADDR, light_g4_auto_state, light_g4_state);
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
      if (fan_auto_state) fan_increment++;
      if (light_g4_auto_state && timeStatus()!=timeNotSet && light_level_initialized) {
        light_need_update = true;
      }
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

    void timerProcess()
    {
      unsigned long int cmilli = millis();
      if (fan_auto_state && cmilli>fan_last_time_state && cmilli-fan_last_time_state > fan_curr_timeout) {
        nextFanState(!fan_state, 0);
      }
      if (light_g4_auto_state) {
        if (light_need_update) {
          if (timeStatus()!=timeNotSet) {
            byte chour = hour();
            if ((chour<LIGHT_AUTO_SKIP_START_HOUR || chour>=LIGHT_AUTO_SKIP_STOP_HOUR) && ((last_light_level<=LIGHT_AUTO_MAX_LEVEL && !light_g4_state) || (last_light_level<=LIGHT_AUTO_MAX_LEVEL+LIGHT_AUTO_ON_ADD && light_g4_state))) {
              setLight(true);
              light_last_time_state = millis();
            }
          }
          light_need_update = false;
        } else if (light_g4_state && cmilli>light_last_time_state && cmilli-light_last_time_state > LIGHT_AUTO_TIMEOUT_LENGTH) {
          setLight(false);
        }
      }
    }

    void nextFanState(bool nextstate, unsigned long old_timeout_inc) 
    {
      DEBUG_WRITE("nextFanState:");DEBUG_WRITELN(fan_increment);
      
      unsigned long last_fan_curr_timeout = fan_curr_timeout;
      long custom_increment = fan_increment;
      if (light_g4_state) custom_increment-=3;
      if (getState(STATE_TONE)) custom_increment-=6;
      if (custom_increment<0) custom_increment = 0;
      float nfreq = fan_curr_timeout/60000.0;
      if (nfreq<=0) nfreq = 1;
      nfreq = custom_increment / nfreq;
      if (!nextstate && nfreq>=FAN_COUNTER_PER_MINUTE_FOR_PERM_ON) {
        nextstate = true;
      } else if (nextstate && nfreq<=FAN_COUNTER_PER_MINUTE_FOR_PERM_OFF) {
        nextstate = false;
      }
      if (nfreq<FAN_COUNTER_PER_MINUTE_FOR_MIN) nfreq = FAN_COUNTER_PER_MINUTE_FOR_MIN;
      if (nfreq>FAN_COUNTER_PER_MINUTE_FOR_MAX) nfreq = FAN_COUNTER_PER_MINUTE_FOR_MAX;
      nfreq = (nfreq-FAN_COUNTER_PER_MINUTE_FOR_MIN)/(FAN_COUNTER_PER_MINUTE_FOR_MAX - FAN_COUNTER_PER_MINUTE_FOR_MIN);
      if (nextstate) {
        fan_curr_timeout = round(nfreq * (FAN_MIN_PERIOD_ON_LENGTH - FAN_MAX_PERIOD_ON_LENGTH)) + FAN_MAX_PERIOD_ON_LENGTH;
      } else {
        fan_curr_timeout = round(nfreq * (FAN_MAX_PERIOD_OFF_LENGTH - FAN_MIN_PERIOD_OFF_LENGTH)) + FAN_MIN_PERIOD_OFF_LENGTH;
      }
      if (old_timeout_inc) {
        float old_state_k = 1 - old_timeout_inc / last_fan_curr_timeout;
        if (old_state_k<FAN_MIN_TIMEOUT_CUT) old_state_k = FAN_MIN_TIMEOUT_CUT;
        if (old_state_k>FAN_MAX_TIMEOUT_CUT) old_state_k = FAN_MAX_TIMEOUT_CUT;
        fan_curr_timeout = round(fan_curr_timeout * old_state_k);
      }
      fan_last_time_state = millis();
      fan_increment = floor(fan_increment*0.75);
      setFan(nextstate);

      DEBUG_WRITELN("FAN state changed"); 
      DEBUG_WRITE("NFREQ:");DEBUG_WRITELN(nfreq);
      DEBUG_WRITE("Timeout:");DEBUG_WRITELN(fan_curr_timeout);
    }

    void updateLightLevel(uint16_t lux)
    {
      last_light_level = lux;
      light_level_initialized = true;
    }
};

IndicationController *IndicationController::_self_controller = NULL;

#endif
