#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#ifndef LCD_I2C_ADDR
  #define LCD_I2C_ADDR 0x27
#endif

#define LCD_AUTO_TURNOFF_MSTIME 100000

LiquidCrystal_I2C lcd(LCD_I2C_ADDR, 16, 2);
bool fixed_text = false;
bool lcd_auto_state = true;
bool lcd_ison = true;
unsigned long int last_auto_state = 0;
char line1_fixed[17];
char line2_fixed[17];
char line1_dyn[17];
char line2_dyn[17];

void initLCD() 
{
  lcd_ison = true;
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.home();
  last_auto_state = millis();
  setLCDAutoState();
  if (fixed_text) {
    lcd.print(line1_fixed);
    lcd.setCursor(0, 1);
    lcd.print(line2_fixed);
  }
}

void setLCDState(bool ison)
{
  lcd_ison = ison;
  lcd_auto_state = false;
  if (lcd_ison) {
    lcd.backlight();
  } else {
    lcd.noBacklight();
  }
}

void setLCDAutoState()
{
  lcd_auto_state = true;
  updateLCDAutoState();
}

void updateLCDAutoState()
{
  if (lcd_auto_state) {
    last_auto_state = millis();
    if (!lcd_ison) {
      lcd.backlight();
      lcd_ison = true;
    }
  }
}

void checkLCDAutoState()
{
  if (lcd_auto_state) {
    if (lcd_ison && millis()-last_auto_state > LCD_AUTO_TURNOFF_MSTIME) {
      lcd.noBacklight();
      lcd_ison = false;
    }
  }
}

void setLCDFixed(const char* text) 
{
  strncpy(line1_fixed, text, 17);
  if (strlen(text)>16) {
    strncpy(line2_fixed, text+16, 17);
  } else {
    line2_fixed[0] = 0;
  }
  fixed_text = true;
  lcd.home(); 
  lcd.clear();
  lcd.print(line1_fixed);
  lcd.setCursor(0, 1);
  lcd.print(line2_fixed);
  checkLCDAutoState();
}

void resetLCDFixed()
{
  fixed_text = false;
  lcd.home(); 
  lcd.clear(); 
  lcd.print(line1_dyn);
  lcd.setCursor(0, 1);
  lcd.print(line2_dyn);
  checkLCDAutoState();
}

void setLCDText(const char* text) 
{
  if (fixed_text) return;
  lcd.home(); 
  lcd.clear(); 
  lcd.print(text);
  strncpy(line1_dyn, text, 17);
  line2_dyn[0] = 0;
  checkLCDAutoState();
}

void setLCDLines(const char* line1, const char* line2) 
{
  if (fixed_text) return;
  lcd.home(); 
  lcd.clear(); 
  lcd.print(line1);
  lcd.setCursor(0, 1);
  lcd.print(line2);
  strncpy(line1_dyn, line1, 17);
  strncpy(line2_dyn, line2, 17);
  checkLCDAutoState();
}
