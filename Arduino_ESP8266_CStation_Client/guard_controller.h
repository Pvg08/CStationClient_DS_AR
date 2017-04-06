#ifndef GUARD_CONTROLLER_H
#define GUARD_CONTROLLER_H

#define WATCH_MODE_DELAY_TIME 30000
#define WATCH_MODE_DELAY_TIME_ALERT 90000
#define WATCH_MODE_BLINK_INTERVAL1 500000L
#define WATCH_MODE_BLINK_INTERVAL2 1000000L

class GuardController 
{
  private:
  	/* watch mode */
  	bool watch_mode;
  	bool is_watch_delay, is_alert_delay;
  	volatile bool has_presence;
  	unsigned long int watch_delay_millis;

    GuardController() 
    {
      watch_mode = false;
      is_watch_delay = false;
      is_alert_delay = false;
      has_presence = false;
      watch_delay_millis = 0;
    }
  
  public:
    static GuardController *_self_controller;

    static GuardController* Instance() {
      if(!_self_controller)
      {
          _self_controller = new GuardController();
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

  	void periodTimerSignal()
  	{
  		ind_controller->toggleBlue();
		if (is_alert_delay) {
			ind_controller->toggleLight();
		}
  	}
  	
  	void toggleWatchMode()
  	{
  		if (!watch_mode) {
  			watch_mode = true;
  			is_watch_delay = true;
  			is_alert_delay = false;
  			has_presence = false;
  			watch_delay_millis = millis();
  			ind_controller->setBlue(false);
  			startTimer1(WATCH_MODE_BLINK_INTERVAL1);
  			resetTimer1();
  			tone_controller->FastToneSignal(1000, 200);
  		} else {
  			watch_mode = false;
  			is_watch_delay = false;
  			is_alert_delay = false;
  			has_presence = false;
  			watch_delay_millis = 0;
  			pauseTimer1();
  			ind_controller->setBlue(false);
  			ind_controller->setLight(false);
  			tone_controller->StopTone();
  		}
		delay(500);
  	}
  	
  	void fixPresence()
  	{
  		has_presence = true;
  	}
	
    void timerProcess(bool reset_btn_pressed)
    {
		if (watch_mode) {
			if (is_watch_delay) {
				if ((is_alert_delay && ((millis() - watch_delay_millis) > WATCH_MODE_DELAY_TIME_ALERT)) || (!is_alert_delay && ((millis() - watch_delay_millis) > WATCH_MODE_DELAY_TIME))) {
					is_watch_delay = false;
					is_alert_delay = false;
					has_presence = false;
					pauseTimer1();
					startTimer1(WATCH_MODE_BLINK_INTERVAL2);
					resetTimer1();
					tone_controller->StopTone();
					tone_controller->FastToneSignal(500, 1200);
					ind_controller->setLight(false);
				}
			} else {
				if (has_presence) {
					is_watch_delay = true;
					is_alert_delay = true;
					has_presence = false;
					watch_delay_millis = millis();
					ind_controller->setLight(true);
					pauseTimer1();
					startTimer1(WATCH_MODE_BLINK_INTERVAL1);
					resetTimer1();
					tone_controller->StopTone();
					tone_controller->StartTone(800, 500);
				}
			}
		}
	    if (reset_btn_pressed) toggleWatchMode();
    }
};

ISR(timer1Event)
{
  resetTimer1();
  GuardController::Instance()->periodTimerSignal();
}

GuardController *GuardController::_self_controller = NULL;

#endif
