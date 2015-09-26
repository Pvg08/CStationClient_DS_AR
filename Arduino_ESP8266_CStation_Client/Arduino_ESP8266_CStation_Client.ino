
#include <TimerOne.h>

#define BAUD_RATE 38400

//#define CSTATION_DEBUG

#ifdef CSTATION_DEBUG
  #define DEBUG_WRITE(...) { Serial.print(__VA_ARGS__); }
  #define DEBUG_WRITELN(...) { Serial.println(__VA_ARGS__); }
  #define DEBUG_LINE_SEPARATOR "-----\r\n"
#else
  #define DEBUG_WRITE(...) {}
  #define DEBUG_WRITELN(...) {}
  #define DEBUG_LINE_SEPARATOR
#endif

#define MAX_ATTEMPTS 5

#define TONE_PIN 7
#define PROG_LED_PIN 22

#define RESET_BTN_PIN 48
#define CONFIG_BTN_PIN 50
#define SIGNAL_BTN_PIN 52

#define CONTROL_BTN_PIN 2
#define CONTROL_BTN_INTERRUPT 0
#define CONTROL_BTN_INTERRUPT_MODE RISING

//#define LCD_I2C_ADDR 0x3F
#define LCD_I2C_ADDR 0x27

byte errors_count = 0;

volatile unsigned tone_frequency;
volatile bool tone_state;
volatile bool reset_btn_pressed = false;
volatile bool config_btn_pressed = false;
volatile bool signal_btn_pressed = false;
volatile bool signal_btn_sended = false;
volatile bool prog_led_state = false;
volatile bool prog_led_tone_control = false;

void setup()
{
  Serial.begin(BAUD_RATE);
  pinMode(RESET_BTN_PIN, INPUT);
  pinMode(CONFIG_BTN_PIN, INPUT);
  pinMode(SIGNAL_BTN_PIN, INPUT);
  pinMode(CONTROL_BTN_PIN, INPUT);
  pinMode(TONE_PIN, OUTPUT);
  pinMode(PROG_LED_PIN, OUTPUT);
  
  initESP();
  initLCD();
  delay(100);
  initSensors();
  delay(100);
  DEBUG_WRITELN("Start\r\n");
  delay(100);
  StartConnection(true);
  attachInterrupt(CONTROL_BTN_INTERRUPT, ControlBTN_Rising, CONTROL_BTN_INTERRUPT_MODE);
  reset_btn_pressed = false;
  config_btn_pressed = false;
  signal_btn_pressed = false;
  signal_btn_sended = true;
}

void loop()
{
  if (config_btn_pressed) {
    DEBUG_WRITELN("Config BTN pressed. Entering configuration mode\r\n");
    StartConfiguringMode();
    config_btn_pressed = false;
    reset_btn_pressed = true;
    return;
  }
  if (reset_btn_pressed) {
    DEBUG_WRITELN("Reset BTN pressed. Resetting\r\n");
    StartConnection(true);
    reset_btn_pressed = false;
    config_btn_pressed = false;
    return;
  }
  
  executeCommands();
  sensorsSending();
}

void ControlBTN_Rising() 
{
  if (!reset_btn_pressed) reset_btn_pressed = digitalRead(RESET_BTN_PIN) == HIGH;
  if (!config_btn_pressed) config_btn_pressed = digitalRead(CONFIG_BTN_PIN) == HIGH;
  if (!signal_btn_pressed) signal_btn_pressed = digitalRead(SIGNAL_BTN_PIN) == HIGH;
  signal_btn_sended = !signal_btn_pressed;
}

void tone_period() 
{
  if (tone_state) {
    tone(TONE_PIN, tone_frequency);
  } else {
    noTone(TONE_PIN);
  }
  tone_state = !tone_state;
  if (prog_led_tone_control) {
    if (prog_led_state) {
      digitalWrite(PROG_LED_PIN, HIGH);
    } else {
      digitalWrite(PROG_LED_PIN, LOW);
    }
    prog_led_state = !prog_led_state;
  }
}

bool sendControlsInfo(unsigned connection_id)
{
  char* reply;
  bool rok;
  
  reply = sendMessage(connection_id, "DC_INFO=Tone", MAX_ATTEMPTS);
  rok = replyIsOK(reply);
  if (!rok) return rok;

  reply = sendMessage(connection_id, "DC_INFO=Led", MAX_ATTEMPTS);
  rok = replyIsOK(reply);
  if (!rok) return rok;

  reply = sendMessage(connection_id, "DC_INFO=State", MAX_ATTEMPTS);
  rok = replyIsOK(reply);
  if (!rok) return rok;

  reply = sendMessage(connection_id, "DC_INFO=Reset", MAX_ATTEMPTS);
  rok = replyIsOK(reply);
  if (!rok) return rok;

  reply = sendMessage(connection_id, "DC_INFO=Config", MAX_ATTEMPTS);
  rok = replyIsOK(reply);
  if (!rok) return rok;

  reply = sendMessage(connection_id, "DC_INFO=LCD", MAX_ATTEMPTS);
  rok = replyIsOK(reply);
  
  return rok;
}

void executeInputMessage(char *input_message)
{
  char *message;
  unsigned connection_id = 0;
  message = readTCPMessage( 1000, &connection_id, true, input_message);

  if (message && !config_btn_pressed && !reset_btn_pressed) {
    DEBUG_WRITELN("Query found. Executing...");

    char* param;
    if ((param = getMessageParam(message, "SERV_RST=1", true))) 
    {
      StartConnection(true);
      reset_btn_pressed = false;
      config_btn_pressed = false;
      delay(1000);
    } else if ((param = getMessageParam(message, "SERV_CONF=1", true))) {
      StartConfiguringMode();
      reset_btn_pressed = false;
      config_btn_pressed = false;
      delay(1000);
    } else if ((param = getMessageParam(message, "STATES_REQUEST=1", true))) {
      String states_str = "DS_STATE={";
      states_str = states_str + "\"LED\":\""+(prog_led_state ? "on" : "off")+"\", ";
      states_str = states_str + "\"TONE\":\""+(tone_state ? "on" : "off")+"\"";
      states_str += "}";
      delay(50);
      sendMessage(connection_id, states_str, MAX_ATTEMPTS);
      delay(100);
    } else if ((param = getMessageParam(message, "LED_SET=", true))) {
      byte led_s = readIntFromString(param, 0);
      prog_led_tone_control = false;
      if (led_s) {
        digitalWrite(PROG_LED_PIN, HIGH);
        prog_led_state = true;
      } else {
        digitalWrite(PROG_LED_PIN, LOW);
        prog_led_state = false;
      }
    } else if ((param = getMessageParam(message, "TONE=", true))) {
      if (param[0] == 'L') {
        prog_led_tone_control = true;
        param++;
        if (param[0] == ',') param++;
      }
      tone_frequency = readIntFromString(param, 0);
      Timer1.detachInterrupt();
      Timer1.stop();
      if (tone_frequency) {
        unsigned long int period = readIntFromString(param, String(tone_frequency).length()+1);
        if (period>0) {
          DEBUG_WRITE("Starting tone. F="); DEBUG_WRITE(tone_frequency); DEBUG_WRITE(" P="); DEBUG_WRITELN(period);
          tone_state = false;
          if (prog_led_tone_control) {
            digitalWrite(PROG_LED_PIN, LOW);
            prog_led_state = false;
          }
          Timer1.initialize(period*1000);
          Timer1.attachInterrupt(tone_period);
        } else {
          DEBUG_WRITE("Starting tone. F="); DEBUG_WRITELN(tone_frequency);
          tone(TONE_PIN, tone_frequency);
          if (prog_led_tone_control) {
            digitalWrite(PROG_LED_PIN, HIGH);
            prog_led_state = true;
          }
          tone_state = true;
        }
      } else {
        DEBUG_WRITELN("Stopping tone");
        noTone(TONE_PIN);
        tone_state = false;
        if (prog_led_tone_control) {
          digitalWrite(PROG_LED_PIN, LOW);
          prog_led_tone_control = false;
          prog_led_state = false;
        }
      }
    } else if ((param = getMessageParam(message, "SERV_LT=", true))) {
      setLCDFixed(param);
    } else if ((param = getMessageParam(message, "SERV_LR=1", true))) {
      resetLCDFixed();
    }
    delay (100);
  }
}

void executeCommands() 
{
  executeInputMessage(NULL);
}

