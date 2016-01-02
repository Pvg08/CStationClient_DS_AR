#ifndef TONE_CONTROLLER_H
#define TONE_CONTROLLER_H

#define TONE_PIN 7
#define MAX_TIMER_PERIOD 1000

unsigned int oct_freq[9][7]= {
  {16,  18,  21,  22,  25,  28,  31  },
  {33,  37,  41,  44,  49,  55,  62  },
  {65,  73,  82,  87,  98,  110, 123 },
  {131, 147, 165, 175, 196, 220, 247 },
  {262, 294, 330, 349, 392, 440, 494 },
  {523, 587, 659, 698, 784, 880, 988 },
  {1046,1175,1319,1568,1760,1976,1967},
  {2093,2349,2637,2794,3136,3520,3951},
  {4186,4699,5274,5588,6272,7040,7902}
};

unsigned int melody_tempo = 600;

class ToneController 
{
  private:
    volatile unsigned tone_frequency;
    volatile unsigned long tone_period;

    volatile bool tone_is_active; //Signal or periodic signal is running
    volatile bool tone_state; // Current tone state
    volatile bool tone_periodic; // Periodic tone is running
    volatile unsigned tone_periodic_repeats; // Cound of periodic repititions
    volatile bool prog_led_tone_control; // Led control is running
    volatile bool prog_led_state; // Current Led state
    volatile bool fast_signal_active; // Fast signal flag
    volatile bool tone_muted;
    volatile bool tone_is_melody;
	char *melody;
    volatile unsigned int melody_pos;

    volatile unsigned long timer_counter;
    volatile unsigned long timer_counter_max;

    void StartTonePeriodTimer(unsigned long period_ms) 
    {
      if (!tone_periodic) {

        if (period_ms>MAX_TIMER_PERIOD) {
          unsigned long correct_period;
          if (period_ms>=2*MAX_TIMER_PERIOD) {
            correct_period = MAX_TIMER_PERIOD;
          } else {
            correct_period = period_ms / 2;
          }
          for(;period_ms % correct_period != 0; correct_period--);
          timer_counter_max = period_ms/correct_period;
          period_ms = correct_period;
        } else {
          timer_counter_max = 1;
        }
        timer_counter = timer_counter_max;

        DEBUG_WRITE("Timer5 period = "); DEBUG_WRITELN(period_ms*1000);
        DEBUG_WRITE("Timer5 max counter = "); DEBUG_WRITELN(timer_counter_max);
        startTimer5(period_ms*1000);
        resetTimer5();
        tone_periodic = true;
        tone_period = period_ms;
      }
    }

    void StopTonePeriodTimer() 
    {
      if (tone_periodic) {
        pauseTimer5();
        DEBUG_WRITELN("Timer5 paused");
        tone_periodic = false;
        tone_period = 0;
        timer_counter = 0;
        timer_counter_max = 0;
      }
    }

    void StartSimpleTone(unsigned frequency)
    {
      DEBUG_WRITE("Starting tone. F="); DEBUG_WRITELN(tone_frequency);
      StopTonePeriodTimer();
      tone_frequency = frequency;
      tone_period = 0;
      tone_periodic_repeats = 0;
      tone_is_active = true;
      tone(TONE_PIN, tone_frequency);
      if (prog_led_tone_control) {
        ind_controller->ToneState(1);
        prog_led_state = true;
      }
      tone_state = true;
    }

    void StartPeriodicTone(unsigned frequency, unsigned long period, unsigned long repeats_count, bool start_state)
    {
      DEBUG_WRITE("Starting tone. F="); DEBUG_WRITE(frequency); DEBUG_WRITE(" P="); DEBUG_WRITE(period); DEBUG_WRITE(" R="); DEBUG_WRITELN(repeats_count);
      tone_frequency = frequency;
      tone_periodic_repeats = repeats_count;
      tone_state = !start_state;
      tone_is_active = true;
      if (prog_led_tone_control) {
        ind_controller->ToneState(0);
        prog_led_state = tone_state;
      }
      TonePeriodAction();
      StartTonePeriodTimer(period);
    }

    void ToneMelodyAction()
    {
      if (melody[melody_pos]==0) {
        noTone(TONE_PIN);
        digitalWrite(TONE_PIN, HIGH);
        tone_muted = true;
        return;
      }
      if (melody[melody_pos]==',') melody_pos++;
      if (melody[melody_pos]==',' || melody[melody_pos]==0) {
        noTone(TONE_PIN);
        digitalWrite(TONE_PIN, HIGH);
        if (melody[melody_pos]==0) {
          tone_muted = true;
          return;
        }
      } else {
        char bukv = melody[melody_pos];
        byte b_pos = 0;
        switch(bukv) {
          case 'C': b_pos = 0; break;
          case 'D': b_pos = 1; break;
          case 'E': b_pos = 2; break;
          case 'F': b_pos = 3; break;
          case 'G': b_pos = 4; break;
          case 'A': b_pos = 5; break;
          case 'B': b_pos = 6; break;
        }
        melody_pos++;
        char cifr = melody[melody_pos];
        byte c_pos = cifr - '0';
        tone(TONE_PIN, oct_freq[c_pos][b_pos]);
      }
      melody_pos++;
    }

    void TonePeriodAction()
    {
      tone_state = !tone_state;
      if (tone_state) {
        tone(TONE_PIN, tone_frequency);
      } else {
        noTone(TONE_PIN);
        digitalWrite(TONE_PIN, HIGH);
      }
      if (prog_led_tone_control) {
        prog_led_state = !prog_led_state;
        ind_controller->ToneState(prog_led_state?1:0);
      }
      if (!tone_state && tone_periodic_repeats) {
        tone_periodic_repeats--;
        if (!tone_periodic_repeats) {
          tone_muted = true;
        }
      }
    }

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
      pinMode(TONE_PIN, OUTPUT);
      digitalWrite(TONE_PIN, HIGH);
      melody_pos = 0;
      tone_frequency = 0;
      tone_period = 0;
      tone_is_active = false;
      tone_state = false;
      tone_periodic = false;
      tone_periodic_repeats = 0;
      prog_led_tone_control = false;
      prog_led_state = false;
      fast_signal_active = false;
      tone_muted = false;
      tone_is_melody = false;
    }

    void timerProcess()
    {
      if (tone_muted) {
        StopTone();
      }
    }

    bool isToneRunning()
    {
      return tone_is_active && !tone_muted;
    }

    void setLedControl(bool state)
    {
      prog_led_tone_control = state;
    }

    void StartMelodyTone(char *cmelody)
    {
	  melody = cmelody;
      DEBUG_WRITELN("Starting melody");
      tone_frequency = 1;
      melody_pos = 0;
      tone_periodic_repeats = 1;
      tone_state = true;
      tone_is_active = true;
      tone_is_melody = true;
      ToneMelodyAction();
      StartTonePeriodTimer(melody_tempo);
    }

    void StartTone(unsigned frequency, unsigned long period)
    {
      if (frequency) {
        if (period) {
          StartPeriodicTone(frequency, period, 0, true);
        } else {
          StartSimpleTone(tone_frequency);
        }
      } else {
        StopTone();
      }
    }

    void StopTone() 
    {
      DEBUG_WRITELN("Stopping tone");
      StopTonePeriodTimer();
      noTone(TONE_PIN);
      digitalWrite(TONE_PIN, HIGH);
      tone_is_melody = false;
      tone_is_active = false;
      fast_signal_active = false;
      tone_state = false;
      if (prog_led_tone_control) {
        ind_controller->ToneState(0);
        prog_led_tone_control = false;
        prog_led_state = false;
      }
      tone_muted = false;
    }

    void TonePeriodTimerSignal()
    {
      if (timer_counter && tone_periodic && !tone_muted && tone_is_active) {
        if (tone_is_melody) {
          ToneMelodyAction();
        } else {
          timer_counter--;
          if (!timer_counter) {
            timer_counter = timer_counter_max;
            TonePeriodAction();
          }
        }
      }
    }

    void FastToneSignal(unsigned frequency, unsigned long tone_length)
    {
      if (!tone_is_active || tone_muted || fast_signal_active) {
        if (tone_is_active) {
          StopTone();
          delay(10);
        }
        StartPeriodicTone(frequency, tone_length, 1, true);
        fast_signal_active = true;
      }
    }

    void RunCommand(char* param)
    {
      fast_signal_active = false;
      if (param[0] == 'L') {
        prog_led_tone_control = true;
        param++;
        if (param[0] == ',') param++;
      }
      StopTonePeriodTimer();
      unsigned frequency = StringHelper::readIntFromString(param, 0);
      unsigned long period = 0;
      if (frequency) {
        period = StringHelper::readIntFromString(param, String(frequency).length()+1);
      }
      StartTone(frequency, period);
    }
};

ISR(timer5Event)
{
  resetTimer5();
  ToneController::Instance()->TonePeriodTimerSignal();
}

ToneController *ToneController::_self_controller = NULL;

#endif
