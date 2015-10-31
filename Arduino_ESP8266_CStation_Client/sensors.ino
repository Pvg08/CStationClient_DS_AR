#include <Wire.h>
#include <SFE_BMP180.h>
#include <DHT.h>
#include <BH1750.h>
#include <HMC5883L.h>

#define MAX_ERRORS 4

#define SENDING_INTERVAL 30000
#define ERROR_CHECK_INTERVAL 120000

#define DHTPIN 5
#define DHTTYPE DHT22

#define HC_PIN 18
#define HC_INTERRUPT 5
#define HC_INTERRUPT_MODE CHANGE

#define NS_PIN 19
#define NS_INTERRUPT 4
#define NS_INTERRUPT_MODE RISING

#define SENSOR_OUT_PIN 3
#define SENSOR_OUT_INTERRUPT 1
#define SENSOR_OUT_INTERRUPT_MODE CHANGE

volatile unsigned long int last_sending_millis, last_reset_millis;

SFE_BMP180 pressure;
DHT dht(DHTPIN, DHTTYPE);
BH1750 lightMeter;
HMC5883L magnetic_meter;

bool setH = false;
float oldH;
bool H_init = false;
bool MXYZ_init = false;

volatile bool hc_info_sended = false;
volatile bool hc_state = false;
volatile bool ns_info_sended = false;
volatile bool ns_state = false;
volatile bool sensor_outer_signal = false;
volatile bool sensor_outer_signal_sended = false;

void HC_State_Changed() 
{
  hc_state = digitalRead(HC_PIN) == HIGH;
  if (hc_state) {
    hc_info_sended = false;
    ON_PresenceDetected();
    IndicationController::PresenceState(hc_state);
  }
}

void NS_State_Rising()
{
  if (!tone_frequency || (tone_periodic && tone_state)) {
    ns_state = true;
    ns_info_sended = false;
    ON_PresenceDetected();
    IndicationController::PresenceState(ns_state);
  }
}

void SensorOuter_State_Changed()
{
  sensor_outer_signal = digitalRead(SENSOR_OUT_PIN) == HIGH;
  sensor_outer_signal_sended = false;
  IndicationController::OuterState(1);
}

void initSensors() 
{
  pinMode(HC_PIN, INPUT);
  pinMode(NS_PIN, INPUT);
  pinMode(SENSOR_OUT_PIN, INPUT);
  H_init = !!pressure.begin();
  if (!H_init) 
  {
    DEBUG_WRITELN("Error with bmp180 connection\r\n");
    setLCDText("BMP Sensor Error");
    delay(3000);
  }
  MXYZ_init = !!magnetic_meter.begin();
  if (!MXYZ_init) 
  {
    DEBUG_WRITELN("Error with HMC5883L connection\r\n");
    setLCDLines("HMC5883L Sensor", "Error");
    delay(3000);
  } else {
    magnetic_meter.setRange(HMC5883L_RANGE_1_3GA);
    magnetic_meter.setMeasurementMode(HMC5883L_CONTINOUS);
    magnetic_meter.setDataRate(HMC5883L_DATARATE_30HZ);
    magnetic_meter.setSamples(HMC5883L_SAMPLES_2);
  }
  dht.begin();
  lightMeter.begin();
  attachInterrupt(HC_INTERRUPT, HC_State_Changed, HC_INTERRUPT_MODE);
  attachInterrupt(NS_INTERRUPT, NS_State_Rising, NS_INTERRUPT_MODE);
  attachInterrupt(SENSOR_OUT_INTERRUPT, SensorOuter_State_Changed, SENSOR_OUT_INTERRUPT_MODE);
  last_sending_millis = last_reset_millis = millis();
}

bool sendSensorsInfo(unsigned connection_id) 
{
  char* reply;
  bool rok;
  
  reply = sendMessage(connection_id, "DS_INFO={'CODE':'A','NAME':'Activity','TIMEOUT':60,'TYPE':'ENUM','ENUMS':['off','on']}", MAX_ATTEMPTS);
  rok = StringHelper::replyIsOK(reply);
  if (!rok) return rok;

  reply = sendMessage(connection_id, "DS_INFO={'CODE':'E','NAME':'Errors','TYPE':'INT','MIN':0,'MAX':100000}", MAX_ATTEMPTS);
  rok = StringHelper::replyIsOK(reply);
  if (!rok) return rok;

  reply = sendMessage(connection_id, "DS_INFO={'CODE':'T','NAME':'Temperature','TYPE':'FLOAT','MIN':-100,'MAX':100,'EM':'°C'}", MAX_ATTEMPTS);
  rok = StringHelper::replyIsOK(reply);
  if (!rok) return rok;
  
  reply = sendMessage(connection_id, "DS_INFO={'CODE':'P','NAME':'Pressure','TYPE':'FLOAT','MIN':500,'MAX':1000,'EM':'mm'}", MAX_ATTEMPTS);
  rok = StringHelper::replyIsOK(reply);
  if (!rok) return rok;

  if (H_init) {
    reply = sendMessage(connection_id, "DS_INFO={'CODE':'H','NAME':'Humidity','TYPE':'FLOAT','MIN':0,'MAX':100,'EM':'%'}", MAX_ATTEMPTS);
    rok = StringHelper::replyIsOK(reply);
    if (!rok) return rok;
  }
  
  reply = sendMessage(connection_id, "DS_INFO={'CODE':'L','NAME':'Illuminance','TYPE':'FLOAT','MIN':0,'MAX':200000,'EM':'lux'}", MAX_ATTEMPTS);
  rok = StringHelper::replyIsOK(reply);
  if (!rok) return rok;
  
  reply = sendMessage(connection_id, "DS_INFO={'CODE':'R','NAME':'Presence','TIMEOUT':10,'TYPE':'ENUM','ENUMS':['no','yes']}", MAX_ATTEMPTS);
  rok = StringHelper::replyIsOK(reply);
  if (!rok) return rok;

  if (MXYZ_init) {
    reply = sendMessage(connection_id, "DS_INFO={'CODE':'Mx','NAME':'Magnetic field Vector X','TYPE':'FLOAT','MIN':-10000,'MAX':10000,'EM':'deg'}", MAX_ATTEMPTS);
    rok = StringHelper::replyIsOK(reply);
    if (!rok) return rok;
    reply = sendMessage(connection_id, "DS_INFO={'CODE':'My','NAME':'Magnetic field Vector Y','TYPE':'FLOAT','MIN':-10000,'MAX':10000,'EM':'deg'}", MAX_ATTEMPTS);
    rok = StringHelper::replyIsOK(reply);
    if (!rok) return rok;
    reply = sendMessage(connection_id, "DS_INFO={'CODE':'Mz','NAME':'Magnetic field Vector Z','TYPE':'FLOAT','MIN':-10000,'MAX':10000,'EM':'deg'}", MAX_ATTEMPTS);
    rok = StringHelper::replyIsOK(reply);
    if (!rok) return rok;
  }

  reply = sendMessage(connection_id, "DS_INFO={'CODE':'N','NAME':'Noise','TIMEOUT':5,'TYPE':'ENUM','ENUMS':['no','yes']}", MAX_ATTEMPTS);
  rok = StringHelper::replyIsOK(reply);
  if (!rok) return rok;

  reply = sendMessage(connection_id, "DS_INFO={'CODE':'O','NAME':'Outer signal','TYPE':'ENUM','ENUMS':['no','yes']}", MAX_ATTEMPTS);
  rok = StringHelper::replyIsOK(reply);
  if (!rok) return rok;

  reply = sendMessage(connection_id, "DS_INFO={'CODE':'B','NAME':'Signal button','TIMEOUT':5,'TYPE':'ENUM','ENUMS':['no','yes']}", MAX_ATTEMPTS);
  rok = StringHelper::replyIsOK(reply);
  if (!rok) return rok;

  reply = sendMessage(connection_id, "DS_V={'A':'on'}", MAX_ATTEMPTS);
  rok = StringHelper::replyIsOK(reply);

  last_sending_millis = 0;

  return rok;
}

bool sensorsSending() 
{
  if ((hc_state && !hc_info_sended) || (ns_state && !ns_info_sended) || !sensor_outer_signal_sended || (signal_btn_pressed && !signal_btn_sended)) {
    IndicationController::SensorsSendingSignalState(1);
  }
  
  bool result = false;
  unsigned long int curr_millis = millis();
  unsigned long int millis_sum_delay = 0;
  unsigned long int millis_sum_reset_delay = 0;
  
  if (curr_millis > last_reset_millis) {
    millis_sum_reset_delay = curr_millis - last_reset_millis;
  }

  if (millis_sum_reset_delay > ERROR_CHECK_INTERVAL) 
  {
    if (errors_count>MAX_ERRORS) 
    {
      DEBUG_WRITELN("Too much errors. Restarting...");
      StartConnection(true);
      errors_count = 0;
      last_reset_millis = last_sending_millis = millis();
      return false;
    }
    errors_count = 0;
    last_reset_millis = curr_millis;
  }

  if (curr_millis > last_sending_millis) 
  {
    millis_sum_delay = curr_millis - last_sending_millis;
  }

  if (!last_sending_millis || millis_sum_delay > SENDING_INTERVAL) 
  {
    IndicationController::SensorsSendingState(1);
    
    char status;
    double T,P;

    String send_str = "'A':'on'";
    String lcd1 = "";
    String lcd2 = "";
    
    if (send_str.length()>0) send_str = send_str + ",";
    send_str = send_str + "'E':" + String(errors_count);

    status = pressure.startTemperature();
    if (status != 0)
    {
      delay(status);
      status = pressure.getTemperature(T);
      if (status != 0)
      {
        if (send_str.length()>0) send_str = send_str + ",";
        send_str = send_str + "'T':" + String(T, 2);
        lcd1 = "T="+String(T, 1)+"\337C ";
        
        status = pressure.startPressure(3);
        if (status != 0)
        {
          delay(status);
          status = pressure.getPressure(P, T);
          if (status != 0)
          {
            P = P*0.750063755;
            if (send_str.length()>0) send_str = send_str + ",";
            send_str = send_str + "'P':" + String(P, 3);
            lcd2 = "P="+String(P, 2)+"mm";
          }
        }
      }
    }
    
    if (H_init) {
      float H = dht.readHumidity();
      if (!isnan(H)) {
        setH = true;
        oldH = H;
        if (send_str.length()>0) send_str = send_str + ",";
        send_str = send_str + "'H':" + String(H, 1);
        lcd1 = lcd1 + "H="+String(H, H>99.9 ? 0 : 1)+"%";
      } else if (setH) {
        lcd1 = lcd1 + "H="+String(oldH, oldH>99.9 ? 0 : 1)+"%";
      }
    }

    uint16_t lux = lightMeter.readLightLevel();
    if (send_str.length()>0) send_str = send_str + ",";
    send_str = send_str + "'L':" + String(lux);

    if (send_str.length()>0) send_str = send_str + ",";
    send_str = send_str + "'R':'" + (digitalRead(HC_PIN) == HIGH ? "yes" : "no") + "'";

    if (ns_state && !ns_info_sended) {
      if (send_str.length()>0) send_str = send_str + ",";
      send_str = send_str + "'N':'yes'";
    }

    if (MXYZ_init) {
      Vector norm = magnetic_meter.readNormalize();
      if (send_str.length()>0) send_str = send_str + ",";
      send_str = send_str + "'Mx':" + String(norm.XAxis, 0) + ",'My':" + String(norm.YAxis, 0) + ",'Mz':" + String(norm.ZAxis, 0);
    }

    setLCDLines(lcd1.c_str(), lcd2.c_str());
    
    if (send_str.length()>0) {
      send_str = "DS_V={" + send_str + "}";
      sendMessage(connection_id, send_str, 0);
      result = true;
    }
    
    if (!hc_info_sended || !ns_info_sended) {
      IndicationController::SensorsSendingSignalState(0);
    }
    
    hc_info_sended = true;
    ns_info_sended = true;

    last_sending_millis = millis();

    IndicationController::SensorsSendingState(0);

  } else if ((hc_state && !hc_info_sended) || (ns_state && !ns_info_sended) || !sensor_outer_signal_sended || (signal_btn_pressed && !signal_btn_sended)) {
    
    delay(50);
    
    String send_str = "'A':'on'";
    
    if (hc_state && !hc_info_sended) {
      if (send_str.length()>0) send_str = send_str + ",";
      send_str = send_str + "'R':'yes'";
    }
    if (ns_state && !ns_info_sended) {
      if (send_str.length()>0) send_str = send_str + ",";
      send_str = send_str + "'N':'yes'";
    }
    if (signal_btn_pressed && !signal_btn_sended) {
      if (send_str.length()>0) send_str = send_str + ",";
      send_str = send_str + "'B':'yes'";
    }
    if (!sensor_outer_signal_sended) {
      if (send_str.length()>0) send_str = send_str + ",";
      send_str = send_str + "'O':'"+(sensor_outer_signal ? "yes" : "no")+"'";
    }
    
    char* reply = sendMessage(connection_id, "DS_V={"+send_str+"}", 1);
    bool info_sended = StringHelper::replyIsOK(reply);

    if (hc_state && !hc_info_sended) hc_info_sended = info_sended;
    if (ns_state && !ns_info_sended) ns_info_sended = info_sended;
    if (signal_btn_pressed && !signal_btn_sended) signal_btn_sended = info_sended;
    if (!sensor_outer_signal_sended) sensor_outer_signal_sended = info_sended;

    IndicationController::SensorsSendingSignalState(0);
  }

  return result;
}
