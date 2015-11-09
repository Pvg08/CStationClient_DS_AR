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
};

#endif
