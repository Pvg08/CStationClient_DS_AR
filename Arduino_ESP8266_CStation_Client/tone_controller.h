#ifndef TONE_CONTROLLER_H
#define TONE_CONTROLLER_H

#define TONE_PIN 7
#define MAX_TIMER_PERIOD 1000

#define MELODY_MAX_SIZE 512

#define CUSTOM_MELODY_ADDR 1024

const unsigned oct_freq[9][7] = {
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

const float diez_k = 1.059463;
const float bemol_k = 0.9438743;

byte melody_count = 17;

const char melody_1[] PROGMEM = "150:G4,E5,E5,D5,E5,C5,G4,G4,G4,E5,E5,F5,D5,G5,,G5,A4,A4,F5,F5,E5,D5,C5,G4,E5,E5,D5,E5,C5,,G5,A4,A4,F5,F5,E5,D5,C5,G4,E5,E5,D5,E5,C5";
const char melody_2[] PROGMEM = "220:B5b,F5,B5b,F5,B5b,A5,A5,,A5,F5,A5,F5,A5,B5b,B5b,,B5b,F5,B5b,F5,B5b,A5,A5,,A5,F5,A5,F5,A5,B5b,,,B5b,C6=6,p2,C6=3,p1,C6=3,p1,C6=6,p2,C6,C6#=6,p2,C6#=3,p1,C6#=3,p1,C6#=7,p1,C6#=7,p1,C6#,C6,B5b,A5,B5b,B5b,,,B5b,C6=6,p2,C6=3,p1,C6=3,p1,C6=6,p2,C6,C6#=6,p2,C6#=3,p1,C6#=3,p1,C6#=7,p1,C6#=7,p1,C6#,C6,B5b,A5,B5b";
const char melody_3[] PROGMEM = "140-2:B6b,F7,D7#,F7,G7#,F7,F7,,B6b,F7,D7#,F7,B7b,F7,F7,,,C8#,C8,B7b,G7#,B7,F7,F7,,,C8#,C8,B7b,G7#,C8,F7,F7,,,B6b,F7,D7#,F7,G7b,F7,F7,,,B6b,F7,D7#,F7,B7b,F7,F7,,,B7b,F7,F7";
const char melody_4[] PROGMEM = "160:F6#=3,p1,F6#=3,p1,F6#=3,p1,E6=3,p1,D6=3,p1,C6#=3,p1,B5,B5=3,p1,B5=3,p1,B5=3,p1,B5b=3,p1,F5#=3,p1,G5=3,p1,F5#=5,p3,F6#=3,p1,F6#=3,p1,F6#=3,p1,E6=3,p1,D6=3,p1,C6#=3,p1,B5=3,p5,B5=3,p1,B5=3,p1,B5=3,p1,B5b=3,p1,F5#=3,p1,G5=3,p1,F5#=5,p3,F6#=3,p1,F6#=3,p1,F6#=3,p1,E6=3,p1,D6=3,p1,C6#=3,p1,B5=3,p1,B5=5,p3,B5=3,p5,C6#=5,D6=3,p1,C6#=3,p1,B5=3,p8,p1,F6#=5,p3,E6=3,p1,D6=3,p1,C6#=3,p1,B5=3,p1,B5=5,p8,p3,C6#=3,p1,D6=3,p1,C6#=3,p1,B5=5";
const char melody_5[] PROGMEM = "200:D5#=6,p8,p8,p8,p2,F5=8,D5#=8,D5=8,p1,C5=4,D5#=5,p8,p8,p8,p7,F5=8,D5#=8,D5=8,p1,C5=4,F5=4,p8,p8,p8,p8,p1,G5=7,F5=8,D5#=8,p1,D5=5,F5=3,p8,p8,p8,p8,p1,G5=7,F5=8,D5#=8,p1,D5=4,D5#=3,p8,p8,p8,p8,F5=8,D5#=8,D5=8,p1,C5=4,D5#=3,p8,p8,p8,p8,p1,F5=7,D5#=8,D5=8,C5=4,D5=4,p8,p8,p8,p4,D5=4,D5#=8,p1,D5=8,C5=8,B4b=4,D5=4";
const char melody_6[] PROGMEM = "150:E5=4,F5,D5=4,E5,E5=4,F5,D5,E5,C5,B4,B4,D5,D5,,,p4,E5=4,F5,D5,E5=4,C5=4,D5,D5=4,B4,B4=4,A4=4,B4=4,C5,C5,p8,p8,p8,p4,E5=4,F5,D5=4,E5,E5=4,F5,F5=4,D5=4,E5,E5=4,C5=4,B4,B4,D5,D5=4,p8,p8,p4,A4=4,F5,F5=4,E5=4,E5,B4=4,D5,D5,C5=4,C5,B4=4,A4,A4,A4=4";
const char melody_7[] PROGMEM = "180:G4=4,F4=3,p1,F4,p8,p8,p8,A4=4,B4=4,C5#=4,D5=4,E5=3,p2,F5=3,p1,E5=6,p2,D5=2,p2,D5,D5,D5,D5=3,p1,D5=2,p2,D5=4,p1,C5=4,B4b=4,A4=4,G4=4,B4b=7,p1,A4=4,A4,A4=2,p7,G4=4,F4,A4=5,G4,G4=7,p1,D4=4,F4=7,p2,A4=3,p1,A4=6,p8,p6,G4=4,F4=3,p1,F4,p8,p8,p8,A4=4,B4=4,C5#=4,D5=4,E5=3,p2,F5=3,p1,E5=6,p2,D5=2,p2,D5,D5,D5,D5=3,p1,D5=2,p2,D5=4,p1,C5=4,B4b=4,A4=4,G4=4,B4b=7,p1,A4=4,A4,A4=2,p7,G4=4,F4,A4=5,G4,G4=7,p1,D4=4,F4=7,p2,A4=3,p1,A4=6";
const char melody_8[] PROGMEM = "160-1:E5=1,p3,A5,A5=2,p2,C6=1,p3,E6=2,p2,F6=1,p3,E6=2,p4,C6=1,p1,A5=4,,E6=2,p2,D6,D6=1,p3,C6=2,p2,B5=2,p2,A5=2,p2,E5=2,,,p2,E5=2,p2,A5,A5=1,p3,C6=1,p3,E6=2,p2,F6=1,p3,E6=2,p4,C6=2,A5=3,,p1,A5=2,p2,E6,E6=2,p2,D6=2,p2,C6=2,p2,B5=2,p2,A5,A5=4,p4,G5=6,p2,C6=5,p2,G5=5,p3,C6=2,p2,D6=2,p3,E6=3,p3,C6=1,p1,C6,C6=1,p3,E6=2,p2,G6,G6=3,F6=3,p2,E6=2,p2,D6=2,p2,E6,E6=3,p1,D6=3,p2,C6=3,p1,B5=2,p2,A5=2,p6,C6=2,p6,E6=2,p2,F6=2,p2,E6=3,p4,C6=1,A5=7,p5,A5=2,p2,E6,E6=2,p2,D6=2,p2,C6=4,B5=2,p2,A5,A5=6";
const char melody_9[] PROGMEM = "180:A5=3,p1,B5=1,p3,C6=4,A5=1,p3,B5=4,C6=2,p8,p8,p2,B5=4,A5=1,p3,B5=4,C6=2,p8,p8,p2,B5=4,A5=2,p2,C6=2,p2,C6=5,p3,A5,A5=1,p8,p8,p8,p3,A5=4,B5=1,p3,C6=3,p1,A5=1,p3,B5=4,C6=2,p8,p8,p2,B5=4,A5=1,p3,B5=4,C6=2,p8,p8,p2,B5=4,A5=1,p3,C6=1,p3,C6=7,p1,A5,A5=4,p8,p8,A5=4,F6,F6,F6=5,p3,G6=2,F6=2,D6=4,E6,E6,E6,E6=5,p8,p7,F6=3,p1,E6=1,p3,F6=3,p1,E6=1,p3,A5=4,D6,D6,p4,C6=5,p3,B5=5,p3,A5=3,p1,B5=2,p2,C6=3,p1,A5=2,p2,B5=4,C6=3,p8,p8,p1,B5=4,A5=3,p1,C6=1,p3,C6=7,p2,A5=5";
const char melody_10[] PROGMEM = "180:A5=7,p1,C6=6,p2,A5=2,p2,A5=2,p2,E5=7,p1,A5=8,C6=6,p2,A5=2,p2,A5=2,p2,F5=6,p2,D5=6,p2,F5=6,p2,E5=2,p2,E5=2,p2,C5=7,p1,E5=2,p2,D5=2,p2,C5=2,p2,D5=2,p2,E5=8,E5=5,p3,A5=6,p2,C6=7,p1,A5=2,p2,A5=2,p2,E5=6,p2,A5=7,p1,C6=6,p2,A5=2,p2,A5=1,p3,F5=6,p2,D5=6,p2,F5=6,p2,E5=2,p2,E5=2,p2,C5=6,p2,E5=4,D5=1,p3,C5=1,p3,B4=2,p2,A4=7";
const char melody_11[] PROGMEM = "145:G5#=4,C6#=4,E6=4,A6=6,G6#=2,G6#=8,p2,G6#=2,F6#=2,E6=2,D6#=4,C6#=4,A6=8,G6#=8,G6#=2,p2,G6#=4,G6=4,G6#=4,A6=6,G6#=2,G6#=8,p2,G6#=2,G6=2,G6#=2,C7#=4,A6=4,G6#=7,p1,F6#=8,,F6#=2,F6#=4,E6=4,D6#=4,C7#=8,C7#=4,B6=4,A6=4,G6#=8,E6=4,D6#=4,C6#=4,B6=4,A6=4,G6#=4,F6#=8,C6#=4,C6=4,C6=4,C6=4,C6=4,C6=4,C6#=2,C6=2,B5b=4,C6=4,C6#=8,C6#=8,C6#=4";
const char melody_12[] PROGMEM = "160:E6=3,p1,D6=3,p1,C6=3,p1,D6=8,D6=3,p1,E6=2,p2,F6=5,p1,E6=2,D6=3,p1,B5=2,C6=8,C6=7,p7,E6=3,p1,D6=3,p1,C6=2,p2,D6=8,D6=3,C6=3,p2,B5=3,C6=2,p1,B5=5,p1,E5=2,C6=8,C6=6,p8,G6=3,p1,F6=3,p1,E6=3,p1,D6=8,D6=4,p6,F6=1,p1,F6=1,p1,F6=1,p1,E6=3,p1,D6=2,p2,E6=8,E6=5,p8,p8,p2,E5=1,p1,E6=2,D6=2,p1,D6=3,p1,C6=5,p1,B5=3,p1,C6=2,p1,B5=5,p1,A5=2,p1,C6=8,C6=5";
const char melody_13[] PROGMEM = "160:C6=1,p3,C6=1,p3,C6=1,p3,C6=4,p4,B5=4,p4,D6=8,C6=4,G5#=5,p8,p8,p3,C6=1,p3,C6=1,p3,C6=4,D6#=8,D6=5,p3,D6=8,C6=3,p1,G5=5,p8,p8,p3,G5=1,p3,G5=1,p3,G5=1,p3,G5#=5,p3,G5=5,p3,D6=8,D6=4,B5=4,G5=4,p4,G5=8,F6=6,p2,F6=7,p1,D6#=8,D6=8,C6=2,p2,C6=1,p3,C6=2,p2,C6=1,p3,C6=5,p3,B5=5,p3,D6=8,C6=5,G5#=5,p8,p8,p3,C6=1,p3,C6=1,p3,C6=1,p3,D6#=5,p3,D6=5,p3,D6=8,C6=4,G5=5,p8,p8,p3,G5=1,p3,G5=1,p3,G5=1,p3,G5#=5,p3,G5=5,p3,D6=8,D6=4,B5=4,G5=4,p4,G5=8,F6=5,p3,F6=8,D6#=8,D6=8,C6=4";
const char melody_14[] PROGMEM = "160:F6#=2,p2,F6#=2,p2,F6#=2,p2,F6#=4,p1,B6=4,p3,B6=5,p2,B6=2,p3,B6=2,p2,B6=2,p2,B6=4,p1,B6b=3,p8,p4,F6#=2,p2,F6#=2,p2,F6#=2,p2,F6#=5,p1,C7#=4,p2,C7#=6,p2,C7#=3,p1,D7=3,p1,C7#=3,p2,B6=5,p8,p7,F6#=2,p1,F6#=2,p2,F6#=2,p2,F6#=4,p2,B6=4,p3,B6=6,p2,B6=2,p2,B6=2,p3,B6=2,p2,B6=5,B6b=3,p8,p4,F6#=2,p2,F6#=2,p2,F6#=2,p2,F6#=5,p1,C7#=3,p3,C7#=4,p4,C7#=4,D7=4,p1,C7#=3,p1,B6=5";
const char melody_15[] PROGMEM = "260:C4=3,C4=4,p1,A3=2,C4=3,p5,C4=3,p8,p2,C4=3,C4=5,A3=2,C4#=3,p4,C4#=3,p8,p3,C4#=3,C4#=4,p1,A3=2,p1,D4=3,p5,D4=3,p4,C4=3,p5,B3b=4,p1,A3=7,p8,p8,p1,A3=3,p1,B3b=4,p1,C4=3,D4=4,p4,D4=5,p8,D3=3,E3=5,F3=4,A3=4,p4,A3=4,p4,G3=4,p1,A3=5,p4,C4=7,p8,p4,E4=4,p1,E4=5,p3,C4=5,p8,p8,p2,C4=2,C4=4,p1,A3=3,C4=4,p4,C4=3,p8,p2,C4=2,p1,C4=4,p1,A3=3,C4#=4,p5,C4#=3,p8,p2,C4#=2,p1,C4#=4,A3=2,D4=3,p5,D4=3,p4,C4=3,p5,B3b=4,p1,A3=8,p8,p8,A3=3,B3b=5,C4=3,D4=4,p4,D4=4,p4,D3=6,E3=5,F3=4,p1,A3=5,p3,A3=4,p4,A3=6,G3#=3,G3=5,F3=7";
const char melody_16[] PROGMEM = "200-2:A6=6,p2,A6=2,p2,A6=2,p2,G6#=6,p2,A6=6,p2,C7=6,p2,B6b=8,B6b=4,p4,A6=6,p2,G6=6,p2,G6=2,p2,G6=2,p2,B6b=6,p2,A6=6,p2,G6=6,p2,F6=8,F6=2,p6,E6=6,p2,D6=6,p2,D6=4,p4,D6=8,C6=4,B5b=4,D6=6,p2,D6=6,p2,D6=8,E6=4,F6=4,G6=8,G6=8,G6=8,G6=8,p8,F6=8,E6=8,D6=4,C6=4,F6=6,p2,F6=6";
const char melody_17[] PROGMEM = "180-1:G6=4,C7=5,B6=2,C7=6,B6=2,A6=8,p4,G6=4,A6=6,G6#=2,A6=6,E6=2,F6=8,p4,G6=4,B6=6,B6b=2,B6=6,A6=2,G6=8,p4,G6#=4,A6=6,G6#=2,A6=6,F6=2,E6=8,p6,G6=2,C7=6,B6=2,C7=6,B6=2,A6=8,p6,G6=2,A6=2,p2,G6=2,p2,F6=2,p2,E6=2,p2,D6=8,p6,A6=1,p1,A6=2,p2,A6=2,p2,B6=2,p2,A6=3,p1,G6=6,p2,A6=2,p2,G6=3,p1,F6=6,p2,B5=6,p2,G6=8,p6,A6=1,p1,A6=2,p2,A6=2,p2,B6=2,p2,A6=3,p1,G6=6,p2,B6=2,p2,C7=2,p2,B6=6,p2,G6=6,p2,C7=8";

const char* const melody_list[] PROGMEM = {melody_1, melody_2, melody_3, melody_4, melody_5, melody_6, melody_7, melody_8, melody_9, melody_10, melody_11, melody_12, melody_13, melody_14, melody_15, melody_16, melody_17};

char melody_buffer[MELODY_MAX_SIZE+1];

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
    int sub_level;
    unsigned int melody_tempo;
    volatile unsigned int melody_pos;
    volatile byte melody_timer_counter;
    volatile byte melody_timer_counter_max;

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
	  if (melody_timer_counter<melody_timer_counter_max) {
		  melody_timer_counter++;
		  return;
	  }
	  melody_timer_counter = 0;
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
		if (bukv!='p') {
			melody_timer_counter_max = 8;
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
			if (c_pos>=sub_level) c_pos-=sub_level; else c_pos=0;
			unsigned int cfreq = oct_freq[c_pos][b_pos];
			if (melody[melody_pos+1]=='#') {
				melody_pos++;
				cfreq = cfreq * diez_k;
			}
			if (melody[melody_pos+1]=='b') {
				melody_pos++;
				cfreq = cfreq * bemol_k;
			}
			if (melody[melody_pos+1]=='=') {
				melody_pos+=2;
				if (melody[melody_pos]==0) {
				  tone_muted = true;
				  return;
				}
				char pcifr = melody[melody_pos];
				melody_timer_counter_max = pcifr - '0';
				if (melody_timer_counter_max>9) melody_timer_counter_max = 9;
			}
			tone(TONE_PIN, cfreq);
		} else {
			noTone(TONE_PIN);
			digitalWrite(TONE_PIN, HIGH);
			melody_pos++;
			if (melody[melody_pos]==0) {
			  tone_muted = true;
			  return;
			}
			char pcifr = melody[melody_pos];
			melody_timer_counter_max = pcifr - '0';
			if (melody_timer_counter_max>9) melody_timer_counter_max = 9;
		}
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
      melody_tempo = 600;
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
	    sub_level = 0;
      unsigned int pos;
      unsigned int melody_ctemp = StringHelper::readIntFromString(melody, 0, &pos);
  	  melody_timer_counter = 1;
  	  melody_timer_counter_max = 1;
      if (melody_ctemp) {
        melody_tempo = 7500 / melody_ctemp;
      } else {
        melody_tempo = 75;
      }
  	  if (melody[pos]=='-') {
    		pos++;
    		sub_level = StringHelper::readIntFromString(melody, pos, &pos);
  	  }
      if (melody[pos] == ':') {
        pos++;
      }
      melody += pos;
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

    void StartMelodyToneByIndex(byte index) {
      if (index == 0) {
        EEPROM_Helper::readStringFromEEPROM(CUSTOM_MELODY_ADDR, melody_buffer, MELODY_MAX_SIZE);
        StartMelodyTone(melody_buffer);
      } else if (index <= melody_count) {
        strlcpy_P(melody_buffer, (char*)pgm_read_word(&(melody_list[index-1])), MELODY_MAX_SIZE);
        StartMelodyTone(melody_buffer);
      }
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

    void RunMelodyCommand(char* param) 
    {
      bool to_buf = false;
      bool by_index = false;
      if (param[0] == 'B') {
        to_buf = true;
        param++;
        if (param[0] == ',') param++;
      }
      if (param[0] == 'I') {
        by_index = true;
        param++;
        if (param[0] == ',') param++;
      }
      if (param[0] == 'B') {
        to_buf = true;
        param++;
        if (param[0] == ',') param++;
      }

      if (by_index) {
        unsigned index = StringHelper::readIntFromString(param, 0);
        StartMelodyToneByIndex(index);
        if (to_buf) {
          EEPROM_Helper::writeStringToEEPROM(CUSTOM_MELODY_ADDR, melody_buffer, MELODY_MAX_SIZE);
        }
      } else {
        melody_buffer[0] = 0;
        strlcpy(melody_buffer, param, MELODY_MAX_SIZE);
        if (to_buf) {
          EEPROM_Helper::writeStringToEEPROM(CUSTOM_MELODY_ADDR, melody_buffer, MELODY_MAX_SIZE);
        }
        StartMelodyTone(melody_buffer);
      }
    }
};

ISR(timer5Event)
{
  resetTimer5();
  ToneController::Instance()->TonePeriodTimerSignal();
}

ToneController *ToneController::_self_controller = NULL;

#endif
