
#define TONE_PIN 7

void tone_period_function();

class ToneController 
{
  private:
    unsigned tone_frequency;
    unsigned tone_period;
    bool tone_state;
    bool tone_periodic;
    bool prog_led_tone_control;
    bool prog_led_state;
  
  public:
    static ToneController *_self_controller;

    static ToneController* Instance() {
      if(!_self_controller)
      {
          _self_controller = new ToneController();
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

    ToneController()
    {
      tone_frequency = 0;
      tone_period = 0;
      tone_state = false;
      tone_periodic = false;
      prog_led_tone_control = false;
      prog_led_state = false;
    }

    bool isToneOff()
    {
      return !tone_frequency || (tone_periodic && tone_state);
    }

    bool getToneState()
    {
      return tone_state;
    }

    bool getLedState()
    {
      return prog_led_state;
    }

    void setLedControl(bool state)
    {
      prog_led_tone_control = state;
    }

    void StartTone(unsigned frequency, unsigned period)
    {
      tone_frequency = frequency;
      tone_period = period;
      if (period) {
        DEBUG_WRITE("Starting tone. F="); DEBUG_WRITE(tone_frequency); DEBUG_WRITE(" P="); DEBUG_WRITELN(tone_period);
        tone_state = false;
        tone_periodic = true;
        if (prog_led_tone_control) {
          ind_controller->ToneState(0);
          prog_led_state = false;
        }
        Timer1.initialize(period*1000);
        Timer1.attachInterrupt(tone_period_function);
      } else {
        StartTone(tone_frequency);
      }
    }

    void StartTone(unsigned frequency)
    {
      tone_frequency = frequency;
      DEBUG_WRITE("Starting tone. F="); DEBUG_WRITELN(tone_frequency);
      tone(TONE_PIN, tone_frequency);
      if (prog_led_tone_control) {
        ind_controller->ToneState(1);
        prog_led_state = true;
      }
      tone_state = true;
      tone_periodic = false;
    }

    void StopTone() 
    {
      DEBUG_WRITELN("Stopping tone");
      noTone(TONE_PIN);
      digitalWrite(TONE_PIN, HIGH);
      tone_state = false;
      tone_periodic = false;
      if (prog_led_tone_control) {
        ind_controller->ToneState(0);
        prog_led_tone_control = false;
        prog_led_state = false;
      }
    }

    void StopTonePeriod() 
    {
      Timer1.detachInterrupt();
      Timer1.stop();
    }

    void TonePeriod()
    {
      if (tone_state) {
        tone(TONE_PIN, tone_frequency);
      } else {
        noTone(TONE_PIN);
        digitalWrite(TONE_PIN, HIGH);
      }
      tone_state = !tone_state;
      if (prog_led_tone_control) {
        if (prog_led_state) {
          ind_controller->ToneState(1);
        } else {
          ind_controller->ToneState(0);
        }
        prog_led_state = !prog_led_state;
      }
    }

    void RunProcess(char* param)
    {
      if (param[0] == 'L') {
        prog_led_tone_control = true;
        param++;
        if (param[0] == ',') param++;
      }
      StopTonePeriod();
      unsigned frequency = StringHelper::readIntFromString(param, 0);
      if (frequency) {
        unsigned long int period = StringHelper::readIntFromString(param, String(frequency).length()+1);
        StartTone(frequency, period);
      } else {
        StopTone();
      }
    }
};

void tone_period_function()
{
  ToneController::Instance()->TonePeriod();
};

ToneController *ToneController::_self_controller = NULL;
