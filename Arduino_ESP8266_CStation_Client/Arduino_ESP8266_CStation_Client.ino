
#include "Arduino.h"
#include <EEPROM.h>
#include <TimerOne.h>
#include "eeprom_helper.h"
#include "string_helper.h"

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

#define RESET_BTN_PIN 48
#define CONFIG_BTN_PIN 50
#define SIGNAL_BTN_PIN 52

#define CONTROL_BTN_PIN 2
#define CONTROL_BTN_INTERRUPT 0
#define CONTROL_BTN_INTERRUPT_MODE RISING

//#define LCD_I2C_ADDR 0x3F
#define LCD_I2C_ADDR 0x27

#include "indication_controller.h"
IndicationController *ind_controller;
#include "tone_controller.h"
ToneController *tone_controller;

byte errors_count = 0;

unsigned long int timestamp = 0;
unsigned long int timestamp_sync = 0;

volatile bool reset_btn_pressed = false;
volatile bool config_btn_pressed = false;
volatile bool signal_btn_pressed = false;
volatile bool signal_btn_sended = false;
volatile bool need_auto_state_lcd_update = false;

unsigned long int getCurrentTimestamp()
{
  if (timestamp && timestamp_sync)
    return timestamp + (millis()-timestamp_sync) / 1000;
  else
    return 0;
}

void setup()
{
  Serial.begin(BAUD_RATE);
  pinMode(RESET_BTN_PIN, INPUT);
  pinMode(CONFIG_BTN_PIN, INPUT);
  pinMode(SIGNAL_BTN_PIN, INPUT);
  pinMode(CONTROL_BTN_PIN, INPUT);
  pinMode(TONE_PIN, OUTPUT);
  digitalWrite(TONE_PIN, HIGH);

  ind_controller = IndicationController::Instance();
  tone_controller = ToneController::Instance();

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
  need_auto_state_lcd_update = false;
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
  if (need_auto_state_lcd_update) {
    need_auto_state_lcd_update = false;
    updateLCDAutoState();
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

bool sendControlsInfo(unsigned connection_id)
{
  char* reply;
  bool rok;
  
  reply = sendMessage(connection_id, "DC_INFO={'CODE':'tone','PREFIX':'TONE','PARAM':[{'NAME':'Led indication','SKIP':1,'VALUE':'L','TYPE':'BOOL'},{'NAME':'Frequency','TYPE':'UINT','DEFAULT':500},{'NAME':'Period','TYPE':'UINT'}],'BUTTONS':[{'NAME':'Reset','PARAMSET':['0']}]}", MAX_ATTEMPTS);
  rok = StringHelper::replyIsOK(reply);
  if (!rok) return rok;

  reply = sendMessage(connection_id, "DC_INFO={'CODE':'led','PREFIX':'LED_SET','PARAM':[{'NAME':'Led state','TYPE':'BOOL'}]}", MAX_ATTEMPTS);
  rok = StringHelper::replyIsOK(reply);
  if (!rok) return rok;

  reply = sendMessage(connection_id, "DC_INFO={'CODE':'state','PREFIX':'STATES_REQUEST','LISTEN':1,'PARAM':[{'VALUE':1,'SKIP':1}]}", MAX_ATTEMPTS);
  rok = StringHelper::replyIsOK(reply);
  if (!rok) return rok;

  reply = sendMessage(connection_id, "DC_INFO={'CODE':'reset','PREFIX':'SERV_RST','PARAM':[{'VALUE':1,'SKIP':1}]}", MAX_ATTEMPTS);
  rok = StringHelper::replyIsOK(reply);
  if (!rok) return rok;

  reply = sendMessage(connection_id, "DC_INFO={'CODE':'config','PREFIX':'SERV_CONF','PARAM':[{'VALUE':1,'SKIP':1}]}", MAX_ATTEMPTS);
  rok = StringHelper::replyIsOK(reply);
  if (!rok) return rok;

  reply = sendMessage(connection_id, "DC_INFO={'CODE':'displaystate','PREFIX':'SET_DISPLAY_ST','PARAM':[{'NAME':'Display state','TYPE':'BOOL'}],'BUTTONS':[{'NAME':'Set auto','PARAMSET':['2']}]}", MAX_ATTEMPTS);
  rok = StringHelper::replyIsOK(reply);
  if (!rok) return rok;

  reply = sendMessage(connection_id, "DC_INFO={'CODE':'settime','PREFIX':'SET_TIME','PARAM':[{'NAME':'Timestamp','TYPE':'UINT'}]}", MAX_ATTEMPTS);
  rok = StringHelper::replyIsOK(reply);
  if (!rok) return rok;

  reply = sendMessage(connection_id, "DC_INFO={'CODE':'lcd','PREFIX':'SERV_LT','PARAM':[{'NAME':'Display text','TYPE':'STRING'}],'BUTTONS':[{'NAME':'Reset','PARAMSET':['']}]}", MAX_ATTEMPTS);
  rok = StringHelper::replyIsOK(reply);
  
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
      states_str = states_str + "\"LED\":\""+(ind_controller->getProgLedState() ? "on" : "off")+"\", ";
      states_str = states_str + "\"TONE\":\""+(tone_controller->getToneState() ? "on" : "off")+"\", ";
      states_str = states_str + "\"TIME\":\""+String(getCurrentTimestamp())+"\", ";
      states_str = states_str + "\"TIME_SYNC\":\""+String(timestamp_sync)+"\"";
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
      tone_controller->RunProcess(param);
    } else if ((param = StringHelper::getMessageParam(message, "SERV_LT=", true))) {
      if (param[0]) setLCDFixed(param); else resetLCDFixed();
    } else if ((param = StringHelper::getMessageParam(message, "SET_DISPLAY_ST=", true))) {
      byte new_d_state = StringHelper::readIntFromString(param, 0);
      if (new_d_state<2) setLCDState(new_d_state!=0); else setLCDAutoState();
    } else if ((param = StringHelper::getMessageParam(message, "SET_TIME=", true))) {
      timestamp = StringHelper::readIntFromString(param, 0);
      timestamp_sync = millis();
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
