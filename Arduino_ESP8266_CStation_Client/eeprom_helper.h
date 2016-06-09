#ifndef EEPROM_HELPER_H
#define EEPROM_HELPER_H

class EEPROM_Helper 
{
  public:
    static void readStringFromEEPROM(int addr, char* string, int string_maxlen) 
    {
      int i;
      string_maxlen--;
      for(i = 0; i<string_maxlen; i++) {
        string[i] = EEPROM.read(addr+i);
      }
      string[i] = 0;
    }
    
    static void writeStringToEEPROM(int addr, char* string, int string_maxlen) 
    {
      int i;
      string_maxlen--;
      for(i = 0; i<string_maxlen && string[i]; i++) {
        EEPROM.write(addr+i,string[i]);
      }
      EEPROM.write(addr+i,0);
    }

    static byte readByte(unsigned addr)
    {
      return EEPROM.read(addr);
    }

    static void writeByte(unsigned addr, byte wrbyte)
    {
      EEPROM.write(addr, wrbyte);
    }

    static void readAutoState(unsigned addr, bool* is_auto, bool* is_on)
    {
      byte saved_state = EEPROM.read(addr);
      if (saved_state != 1 && saved_state != 2 && saved_state != 3) saved_state = 3;
      *is_auto = (saved_state==3);
      if (saved_state!=3) *is_on = (saved_state==2);
    }

    static byte writeAutoState(unsigned addr, bool is_auto, bool is_on)
    {
      byte saved_state = is_auto ? 3 : (is_on ? 2 : 1);
      EEPROM.write(addr, saved_state);
    }
};

#endif
