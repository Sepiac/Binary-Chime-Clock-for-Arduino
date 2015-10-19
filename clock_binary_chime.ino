/**
 * Arduino Binary Chime Clock
 * This sketch and circuit makes a simple clock that shows the time and the temperature.
 * On the hour, the clock will sound the time in binary. A long beep represents a zero and a short beep represents a one.
 * 
 * The parts:
 * 1. I2C LED screen (16x2)
 * 2. Active buzzer
 * 3. DS3231 Real Time Clock Module (RTC)
 * 4. Arduino Duemilanove/Uno/Other
 * 
 * The curcuit:
 * 1. Connect the LCD VCC and GND
 * 2. Connect the LCD SDA to analog pin 4 and the LCD SCL to analog pin 5
 * 3. Connect the RTC VCC and GND
 * 4. Connect the RTC SDA to analog pin 4 and the LCD SCL to analog pin 5
 * 5. Connect the ground pin of the buzzer to GND and the positive pin of the buzzer to digital pin 3
 * 
 */

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Streaming.h>
#include <Time.h>
#include <DS3232RTC.h>

#define BACK_LIGHT_SWITCH 2
#define SPEAKER_PIN 3
int backlightIsOn = 1;

LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

#define AM_CHAR (char)0
#define PM_CHAR (char)1

//init custom characters
byte amChar[8] = {
  0b01110,
  0b10001,
  0b11111,
  0b10001,
  0b00000,
  0b11011,
  0b10101,
  0b10001
};

byte pmChar[8] = {
  0b01110,
  0b10001,
  0b11110,
  0b10000,
  0b00000,
  0b11011,
  0b10101,
  0b10001
};

//end init custom characters

void setup() {
  Serial.begin(9600);
  // init the lcd
  lcd.begin(16, 2);
  lcd.backlight();
  lcd.home();

  //use the time module
  setSyncProvider(RTC.get);

  //init custom caracters
  lcd.createChar(0, amChar);
  lcd.createChar(1, pmChar);

  //set up the backlight button
  pinMode(BACK_LIGHT_SWITCH, INPUT);

  pinMode(SPEAKER_PIN, OUTPUT);
}

void loop() {
  //Time printing stuff
  //TODO abstract this out into a function
  lcd.clear();
  time_t currentTime = now();

  printHour(currentTime);

  printMinute(currentTime);

  printSecond(currentTime);

  lcd << " ";

  printAMPM();
  //end time printing stuff

  lcd.setCursor(0, 1);
  printTemperature('f');

  //backlight setting using time
  if((hour(currentTime) >= 1) && hour(currentTime) <= 7 ) {
    turnBacklightOff();
  } else {
    turnBacklightOn();
  }

  //hour chime
  if(minute(currentTime) == 0 && second(currentTime) == 0) {
    soundHoursInBinary(hourFormat12(currentTime));
  }


  delay(1000);
}

void setBacklightBySwitch() {
   if(digitalRead(BACK_LIGHT_SWITCH)) {
    turnBacklightOn();
  } else if(!digitalRead(BACK_LIGHT_SWITCH)){
    turnBacklightOff();
  }
}

void turnBacklightOn() {
   if(!backlightIsOn) {
     lcd.backlight();
      backlightIsOn = 1;
   }
}

void turnBacklightOff() {
  if(backlightIsOn) {
     lcd.noBacklight();
     backlightIsOn = 0;
  }
}

void soundHoursInBinary(int hour) {
  switch(hour) {
    case 1:
      beepBinary(0);
      beepBinary(0);
      beepBinary(0);
      beepBinary(1);
      break;
    case 2:
      beepBinary(0);
      beepBinary(0);
      beepBinary(1);
      beepBinary(0);
      break;
    case 3:
      beepBinary(0);
      beepBinary(0);
      beepBinary(1);
      beepBinary(1);
      break;
    case 4:
      beepBinary(0);
      beepBinary(1);
      beepBinary(0);
      beepBinary(0);
      break;
    case 5:
      beepBinary(0);
      beepBinary(1);
      beepBinary(0);
      beepBinary(1);
      break;
    case 6:
      beepBinary(0);
      beepBinary(1);
      beepBinary(1);
      beepBinary(0);
      break;
    case 7:
      beepBinary(0);
      beepBinary(1);
      beepBinary(1);
      beepBinary(1);
      break;
    case 8:
      beepBinary(1);
      beepBinary(0);
      beepBinary(0);
      beepBinary(0);
      break;
    case 9:
      beepBinary(1);
      beepBinary(0);
      beepBinary(0);
      beepBinary(1);
      break;
    case 10:
      beepBinary(1);
      beepBinary(0);
      beepBinary(1);
      beepBinary(0);
      break;
    case 11:
      beepBinary(1);
      beepBinary(0);
      beepBinary(1);
      beepBinary(1);
      break;
    case 12:
      beepBinary(1);
      beepBinary(1);
      beepBinary(0);
      beepBinary(0);
      break;
  }
}

void beepBinary(int value) {
  switch(value) {
    case 0:
      beep(150);
      delay(250);
      break;
    case 1:
      beep(50);
      delay(250);
      break;
  }
}

void beep(int durationMillis) {
  digitalWrite(SPEAKER_PIN, HIGH);
  delay(durationMillis);
  digitalWrite(SPEAKER_PIN, LOW);
}

void doubleBeep () {
  beep(50);
  delay(250);
  beep(50);
}

void printHour(time_t time) {
  int hr = hourFormat12(time);
  lcd << ((hr < 10)?"0":"") << hr << ":";
}

void printMinute(time_t time) {
  int min = minute(time);
  lcd << ((min < 10)?"0":"") << min << ":";
}

void printSecond(time_t time) {
  int sec = second(time);
  lcd << ((sec < 10)?"0":"") << sec;
}

void printAMPM() {
  if(isAM()) {
    lcd << AM_CHAR;
  }

  if(isPM()) {
    lcd << PM_CHAR;
  }

}

void printTemperature(char scale) {
  float c = RTC.temperature() / 4.;
  float f = c * 9. / 5. + 32.;
  if(scale == 'f') {
    lcd << f << char(223) << 'F';
  } else {
    lcd << c << char(223) << 'C';
  }
}

