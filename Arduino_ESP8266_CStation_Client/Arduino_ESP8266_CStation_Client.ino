
#include "Arduino.h"
#include <EEPROM.h>
#include <Timer5.h>
#include <Time.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "eeprom_helper.h"
#include "string_helper.h"
#include <avr/pgmspace.h>

enum StateQueryCode 
{ 
  STATE_NONE, 
  STATE_LED, 
  STATE_TONE, 
  STATE_FAN,
  STATE_LIGHTG4 
};

unsigned long getState(StateQueryCode state_code);

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

#define TIME_SYNC_INTERVAL 1861
#define SENDING_INTERVAL 30000
#define ERROR_CHECK_INTERVAL 120000

#define RESET_BTN_PIN 48
#define CONFIG_BTN_PIN 50
#define SIGNAL_BTN_PIN 52

#define CONTROL_BTN_PIN 2
#define CONTROL_BTN_INTERRUPT 0
#define CONTROL_BTN_INTERRUPT_MODE RISING

#include "indication_controller.h"
IndicationController *ind_controller;
#include "tone_controller.h"
ToneController *tone_controller;
#include "lcd_controller.h"
LCDController *lcd_controller;

byte errors_count = 0;

const byte controls_count = 11;
const char control_0[] PROGMEM = "DC_INFO={'CODE':'tone','PREFIX':'TONE','PARAM':[{'NAME':'Led indication','SKIP':1,'VALUE':'L','TYPE':'BOOL'},{'NAME':'Frequency','TYPE':'UINT','DEFAULT':500},{'NAME':'Period','TYPE':'UINT'}],'BUTTONS':[{'NAME':'Reset','PARAMSET':['0']}]}";
const char control_1[] PROGMEM = "DC_INFO={'CODE':'melody','PREFIX':'MEL','PARAM':[{'NAME':'Write to buffer','SKIP':1,'VALUE':'B','TYPE':'BOOL'},{'NAME':'Code as index','SKIP':1,'VALUE':'I','TYPE':'BOOL'},{'NAME':'Code','TYPE':'STRING'}],'BUTTONS':[{'NAME':'Reset','PARAMSET':['0']}]}";
const char control_2[] PROGMEM = "DC_INFO={'CODE':'led','PREFIX':'LED_SET','PARAM':[{'NAME':'Led state','TYPE':'BOOL'}]}";
const char control_3[] PROGMEM = "DC_INFO={'CODE':'state','PREFIX':'STATES_REQUEST','LISTEN':1,'PARAM':[{'VALUE':1,'SKIP':1}]}";
const char control_4[] PROGMEM = "DC_INFO={'CODE':'reset','PREFIX':'SERV_RST','PARAM':[{'VALUE':1,'SKIP':1}]}";
const char control_5[] PROGMEM = "DC_INFO={'CODE':'config','PREFIX':'SERV_CONF','PARAM':[{'VALUE':1,'SKIP':1}]}";
const char control_6[] PROGMEM = "DC_INFO={'CODE':'displaystate','PREFIX':'SET_DISPLAY_ST','PARAM':[{'NAME':'Display state ON','TYPE':'BOOL'}],'BUTTONS':[{'NAME':'Set auto','PARAMSET':['2']}]}";
const char control_7[] PROGMEM = "DC_INFO={'CODE':'fanstate','PREFIX':'SET_FAN_ST','PARAM':[{'NAME':'Fan state ON','TYPE':'BOOL'}],'BUTTONS':[{'NAME':'Set auto','PARAMSET':['2']}]}";
const char control_8[] PROGMEM = "DC_INFO={'CODE':'lightstate','PREFIX':'SET_LIGHT_ST','PARAM':[{'NAME':'Light state ON','TYPE':'BOOL'}],'BUTTONS':[{'NAME':'Set auto','PARAMSET':['2']}]}";
const char control_9[] PROGMEM = "DC_INFO={'CODE':'settime','PREFIX':'SET_TIME','PARAM':[{'NAME':'Timestamp','TYPE':'TIMESTAMP'}],'BUTTONS':[{'NAME':'Request','PARAMSET':['R']}]}";
const char control_10[] PROGMEM = "DC_INFO={'CODE':'lcd','PREFIX':'SERV_LT','PARAM':[{'NAME':'Display text','TYPE':'STRING'}],'BUTTONS':[{'NAME':'Reset','PARAMSET':['']}]}";
const char* const controls_list[] PROGMEM = {control_0, control_1, control_2, control_3, control_4, control_5, control_6, control_7, control_8, control_9, control_10};

volatile bool reset_btn_pressed = false;
volatile bool config_btn_pressed = false;
volatile bool signal_btn_pressed = false;
volatile bool signal_btn_sended = false;
volatile bool need_auto_state_lcd_update = false;
volatile bool time_return_wait = true;

time_t time_sync_provider()
{
  time_return_wait = true;
  return 0;
}

void setup()
{
  Serial.begin(BAUD_RATE);
  pinMode(TONE_PIN, OUTPUT);
  digitalWrite(TONE_PIN, HIGH);
  pinMode(RESET_BTN_PIN, INPUT);
  pinMode(CONFIG_BTN_PIN, INPUT);
  pinMode(SIGNAL_BTN_PIN, INPUT);
  pinMode(CONTROL_BTN_PIN, INPUT);
  attachInterrupt(CONTROL_BTN_INTERRUPT, ControlBTN_Rising, CONTROL_BTN_INTERRUPT_MODE);
  setSyncInterval(TIME_SYNC_INTERVAL);
  setSyncProvider(time_sync_provider);
  lcd_controller = LCDController::Instance();
  lcd_controller->initLCD();
  ind_controller = IndicationController::Instance();
  tone_controller = ToneController::Instance();
  initESP();
  initSensors();
  DEBUG_WRITELN("Starting...\r\n");
  reset_btn_pressed = true;
  config_btn_pressed = false;
  signal_btn_pressed = false;
  signal_btn_sended = true;
  need_auto_state_lcd_update = false;
  time_return_wait = true;
}

void loop()
{
  lcd_controller->timerProcess();
  tone_controller->timerProcess();
  ind_controller->timerProcess();

  if (config_btn_pressed) {
    DEBUG_WRITELN("Config BTN pressed. Entering configuration mode\r\n");
    StartConfiguringMode();
    config_btn_pressed = false;
    reset_btn_pressed = true;
    signal_btn_pressed = false;
    signal_btn_sended = true;
    return;
  }
  if (reset_btn_pressed) {
    DEBUG_WRITELN("Reset initiated. Resetting...\r\n");
    StartConnection(true);
    reset_btn_pressed = false;
    config_btn_pressed = false;
    signal_btn_pressed = false;
    signal_btn_sended = true;
    return;
  }
  if (need_auto_state_lcd_update) {
    need_auto_state_lcd_update = false;
    lcd_controller->updateLCDAutoState();
  }
  
  executeCommands();
  sensorsSending();

  if (time_return_wait) {
    time_return_wait = false;
    delay(100);
    if (sendTimeRequestSignal()) {
      delay(500);
      executeCommands();
    }
  }
}

void ControlBTN_Rising() 
{
  if (digitalRead(CONTROL_BTN_PIN) == HIGH) {
    if (!reset_btn_pressed) reset_btn_pressed = digitalRead(RESET_BTN_PIN) == HIGH;
    if (!config_btn_pressed) config_btn_pressed = digitalRead(CONFIG_BTN_PIN) == HIGH;
    if (!signal_btn_pressed) signal_btn_pressed = digitalRead(SIGNAL_BTN_PIN) == HIGH;
    signal_btn_sended = !signal_btn_pressed;
  }
}

bool sendControlsInfo(unsigned connection_id)
{
  char* reply;
  bool rok;
  char bufr[256];

  for(byte i=0; i<controls_count; i++) {
    strlcpy_P(bufr, (char*)pgm_read_word(&(controls_list[i])), 256);
    reply = sendMessage(connection_id, bufr, MAX_ATTEMPTS);
    rok = StringHelper::replyIsOK(reply);
    if (!rok) return rok;
  }

  return rok;
}

unsigned long getState(StateQueryCode state_code)
{
  switch(state_code) {
    case STATE_LED:      return ind_controller->getProgLedState();
    case STATE_TONE:     return tone_controller->isToneRunning();
    case STATE_FAN:      return ind_controller->getFanState();
    case STATE_LIGHTG4:  return ind_controller->getLightG4State();
  }
  return 0;
}

void executeInputMessage(char *input_message)
{
  char *message;
  unsigned connection_id = 0;
  message = readTCPMessage( 1000, &connection_id, true, input_message);

  if (message && !config_btn_pressed && !reset_btn_pressed) {
    DEBUG_WRITELN("Query found. Executing...");

    char* param;
    if ((param = StringHelper::getMessageParam(message, "SERV_RST=1", true))) 
    {
      StartConnection(true);
      reset_btn_pressed = false;
      config_btn_pressed = false;
      delay(1000);
    } else if ((param = StringHelper::getMessageParam(message, "SERV_CONF=1", true))) {
      StartConfiguringMode();
      reset_btn_pressed = false;
      config_btn_pressed = false;
      delay(1000);
    } else if ((param = StringHelper::getMessageParam(message, "STATES_REQUEST=1", true))) {
      String states_str = "DS_STATE={";
      states_str = states_str + "\"LED\":\""+(getState(STATE_LED) ? "on" : "off")+"\", ";
      states_str = states_str + "\"TONE\":\""+(getState(STATE_TONE) ? "on" : "off")+"\", ";
      states_str = states_str + "\"FAN\":\""+(getState(STATE_FAN) ? "on" : "off")+"\", ";
      states_str = states_str + "\"G4_LIGHT\":\""+(getState(STATE_LIGHTG4) ? "on" : "off")+"\", ";
      states_str = states_str + "\"TIME\":\""+String(now())+"\", ";
      states_str = states_str + "\"SYNC_INTERVAL\":\""+String(TIME_SYNC_INTERVAL)+"\", ";
      states_str = states_str + "\"SENDING_INTERVAL\":\""+String(SENDING_INTERVAL)+"\", ";
      states_str = states_str + "\"ERROR_CHECK_INTERVAL\":\""+String(ERROR_CHECK_INTERVAL)+"\", ";
      states_str = states_str + "\"TIME_STATUS\":\""+String(timeStatus())+"\"";
      states_str += "}";
      delay(50);
      sendMessage(connection_id, states_str, MAX_ATTEMPTS);
      delay(100);
    } else if ((param = StringHelper::getMessageParam(message, "LED_SET=", true))) {
      byte led_s = StringHelper::readIntFromString(param, 0);
      tone_controller->setLedControl(false);
      if (led_s) {
        ind_controller->SetProgLedState(1);
      } else {
        ind_controller->SetProgLedState(0);
      }
    } else if ((param = StringHelper::getMessageParam(message, "TONE=", true))) {
      tone_controller->RunCommand(param);
    } else if ((param = StringHelper::getMessageParam(message, "MEL=", true))) {
      tone_controller->RunMelodyCommand(param);
    } else if ((param = StringHelper::getMessageParam(message, "SERV_LT=", true))) {
      if (param[0]) {
        lcd_controller->setLCDText(param, LCD_PAGE_OUTER);
        lcd_controller->fixPage(LCD_PAGE_OUTER);
      } else {
        lcd_controller->unfixPage();
        lcd_controller->clearLCDText(LCD_PAGE_OUTER);
      }
    } else if ((param = StringHelper::getMessageParam(message, "SET_DISPLAY_ST=", true))) {
      byte new_d_state = StringHelper::readIntFromString(param, 0);
      if (new_d_state<2) lcd_controller->setLCDState(new_d_state!=0); else lcd_controller->setLCDAutoState();
    } else if ((param = StringHelper::getMessageParam(message, "SET_FAN_ST=", true))) {
      byte new_d_state = StringHelper::readIntFromString(param, 0);
      if (new_d_state<2) ind_controller->setFanState(new_d_state!=0); else ind_controller->setFanAutoState();
    } else if ((param = StringHelper::getMessageParam(message, "SET_LIGHT_ST=", true))) {
      byte new_d_state = StringHelper::readIntFromString(param, 0);
      if (new_d_state<2) ind_controller->setLightG4State(new_d_state!=0); else ind_controller->setLightG4AutoState();
    } else if ((param = StringHelper::getMessageParam(message, "SET_TIME=", true))) {
      if (param[0]=='R') {
        time_return_wait = true;
      } else {
        time_t timestamp = StringHelper::readIntFromString(param, 0);
        setTime(timestamp);
        lcd_controller->redrawTimePage();
      }
    }
    delay (100);
  }
}

void executeCommands() 
{
  executeInputMessage(NULL);
}

void ON_PresenceDetected()
{
  need_auto_state_lcd_update = true;
}

