
#define LED_BLUE_PIN 22
#define LED_YELLOW_PIN 24
#define LED_RED_PIN 26

class IndicationController 
{
  public:

    static bool led_blue_state;
    static bool led_yell_state;
    static bool led_red_state;

    static void setRed(bool state)
    {
      IndicationController::led_red_state = state;
      digitalWrite(LED_RED_PIN, state ? HIGH : LOW);
    }
    static void setYellow(bool state)
    {
      IndicationController::led_yell_state = state;
      digitalWrite(LED_YELLOW_PIN, state ? HIGH : LOW);
    }
    static void setBlue(bool state)
    {
      IndicationController::led_blue_state = state;
      digitalWrite(LED_BLUE_PIN, state ? HIGH : LOW);
    }

    static void Init()
    {
      pinMode(LED_BLUE_PIN, OUTPUT);
      pinMode(LED_YELLOW_PIN, OUTPUT);
      pinMode(LED_RED_PIN, OUTPUT);
      IndicationController::setRed(false);
      IndicationController::setYellow(false);
      IndicationController::setBlue(false);
    }

    static void ConfigState(byte state) {
      IndicationController::setRed(state>0);
      IndicationController::setYellow(state>1);
      IndicationController::setBlue(state>1);
    }

    static void ConnectState(byte state) {
      IndicationController::setRed(state>0);
      IndicationController::setYellow(state>1);
      IndicationController::setBlue(false);
    }

    static void SensorsSendingState(byte state) {
      if (!state && IndicationController::led_red_state) {
        delay(200);
      }
      IndicationController::setRed(state>0);
      if (!state && IndicationController::led_yell_state) {
        IndicationController::setYellow(false);
      }
    }

    static void SensorsSendingSignalState(byte state) {
      IndicationController::SensorsSendingState(state);
    }

    static void PresenceState(byte state) {
      if (!IndicationController::led_red_state) IndicationController::setYellow(state>0);
    }

    static void OuterState(byte state) {
      if (!IndicationController::led_red_state) IndicationController::setYellow(state>0);
    }

    static void ToneState(byte state) {
      IndicationController::setBlue(state>0);
    }

    static void SetProgLedState(byte state) {
      IndicationController::setBlue(state>0);
    }
};

bool IndicationController::led_red_state = false;
bool IndicationController::led_yell_state = false;
bool IndicationController::led_blue_state = false;

