#include <Wire.h>
#include <stdio.h>
#include <DS1302RTC.h>
#include <Time.h>
#include <Streaming.h>
#include <Adafruit_NeoPixel.h>
#include "WS2812_Definitions.h"


#define TUBEPIN 11
#define GOALPIN 12

static time_t tLast;
time_t t;
tmElements_t tm;

uint16_t rainbow1 = 0;
uint16_t rainbow2 = 0;

const int button1Pin = 4;
const int button2Pin = 3;

// Set pins:  CE, IO,CLK
DS1302RTC RTC(5, 6, 7);

//Pin connected to ST_CP of 74HC595
int latchPin = 9;
//Pin connected to SH_CP of 74HC595
int clockPin = 8 ;
////Pin connected to DS of 74HC595
int dataPin = 10;

int testpattern = 0;

int button1State = 0;
int button2State = 0;

unsigned long SecondCount = 0; // Second counter
unsigned long MinuteCount = 0; // Second counter
int randint;

byte DataOut1 = B11111111;
byte DataOut2 = B11111111;
byte DataOut3 = B11111111;

Adafruit_NeoPixel striptube = Adafruit_NeoPixel(11, GOALPIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip = Adafruit_NeoPixel(6, TUBEPIN, NEO_GRB + NEO_KHZ800);

void setup () {
  pinMode(button1Pin, INPUT);
  pinMode(button2Pin, INPUT);       
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  randomSeed(analogRead(0));

   // Setup Serial connection
  Serial.begin(115200);
  
  // Test RTC
  Serial.println("DS1302RTC Read Test");
  Serial.println("-------------------");
  
  delay(500);
  
  if (RTC.haltRTC()) {
    Serial.println("The DS1302 is stopped.  Please run the SetTime");
    Serial.println("example to initialize the time and begin running.");
    Serial.println();
  }
  if (!RTC.writeEN()) {
    Serial.println("The DS1302 is write protected. This normal.");
    Serial.println();
  }
  Wire.begin();
  
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'  
  striptube.begin();
  strip.show();
  delay(5000);
 //setSyncProvider() causes the Time library to synchronize with the
  //external RTC by calling RTC.get() every five minutes by default.
  setSyncProvider(RTC.get);

  Serial << F("RTC Sync");
  if (timeStatus() == timeSet)
    Serial << F(" Ok!");
  else
    Serial << F(" FAIL!");
  Serial << endl;
  
}


int jj = 0;
long redcolor = 100;
long greencolor = 0;
long bluecolor = 100;
void loop () {
 
  rainbowopen();
  
  for(int ii=0;ii<12;ii++){
    // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
    striptube.setPixelColor(ii, striptube.Color(redcolor,greencolor,bluecolor)); // purple.    
    if( jj < 5){
    striptube.setPixelColor(3, striptube.Color(0,0,100));
    striptube.setPixelColor(7, striptube.Color(0,0,100));} 
    else if (jj >=5 && jj<=10){
    striptube.setPixelColor(3, striptube.Color(0,0,0));
    striptube.setPixelColor(7, striptube.Color(0,0,0));}
    else if(jj  >10){
      jj = 0;}  
    }
    delay(1);
    striptube.show();
    jj++;

  //check for input to set the RTC, minimum length is 12, i.e. yy,m,d,h,m,s
    if (Serial.available() >= 12) {
        //note that the tmElements_t Year member is an offset from 1970,
        //but the RTC wants the last two digits of the calendar year.
        //use the convenience macros from Time.h to do the conversions.
        int y = Serial.parseInt();
        if (y >= 100 && y < 1000)
            Serial << F("Error: Year must be two digits or four digits!") << endl;
        else {
            if (y >= 1000)
                tm.Year = CalendarYrToTm(y);
            else    //(y < 100)
                tm.Year = y2kYearToTm(y);
            tm.Month = Serial.parseInt();
            tm.Day = Serial.parseInt();
            tm.Hour = Serial.parseInt();
            tm.Minute = Serial.parseInt();
            tm.Second = Serial.parseInt();
            t = makeTime(tm);
	    //use the time_t value to ensure correct weekday is set
            if(RTC.set(t) == 0) { // Success
              setTime(t);
              Serial << F("RTC set to: ");
              printDateTime(t);
              Serial << endl;
	    }
	    else
	      Serial << F("RTC set failed!") << endl;
            //dump any extraneous input
            while (Serial.available() > 0) Serial.read();
        }
    }
    
    //get time from the realtime clock
    t = now();
    
    //print the time to the serial display
    if (t != tLast) {
        tLast = t;
        printDateTime(t);
        Serial << endl;
    }
  
  // Every minute display a test pattern
  if (millis()-MinuteCount > 60000) {
    MinuteCount = millis();
    if (testpattern == 0) {
      testpattern = 1;
    } else if (testpattern == 1) {
      ScrollPattern();
      testpattern = 0;
    }
  } else if (millis()-SecondCount > 1000) {
    SecondCount = millis();
    GetTime();
    DispTime();
  }
  
  button1State = digitalRead(button1Pin);
  button2State = digitalRead(button2Pin);
  if (button1State == HIGH) {   
    // Display the temperature pattern
    MinuteCount = millis();
  } 
  if (button2State == HIGH) {   
    // Display the scrolling pattern
    redcolor = random(255);
    greencolor = random(255);
    bluecolor = random(255);
    MinuteCount = millis();
  } 
  
}



// Displays the time
void DispTime() {
  
 
  int hourdigit1 =hour(t)/10;
  int hourdigit2 =hour(t)%10;
  int mindigit1 =minute(t)/10;
  int mindigit2 =minute(t)%10;
  int secdigit1 =second(t)/10;
  int secdigit2 =second(t)%10;
  SetDigit(1, secdigit2);
  SetDigit(2, secdigit1);
  SetDigit(3, mindigit2);
  SetDigit(4, mindigit1);
  SetDigit(5, hourdigit2);
  SetDigit(6, hourdigit1);
  
 /*SetDigit(1, 6);
  SetDigit(2, 7);
  SetDigit(3, 8);
  SetDigit(4, 9);
  SetDigit(5, 4);
  SetDigit(6, 5);*/
  shiftOut();
}


void ScrollPattern() {
  // Blank the digits
  SetDigit(1, 10);
  SetDigit(2, 10);
  SetDigit(3, 10);
  SetDigit(4, 10);
  SetDigit(5, 10);
  SetDigit(6, 10);
  delay(250);
  //Scroll Loop
  for (int i = 1; i<=6; i++) {
    SetDigit(1, i);
    SetDigit(2, i-1);
    SetDigit(3, i-2);
    SetDigit(4, i-3);
    SetDigit(5, i-4);
    SetDigit(6, i-5);
    shiftOut();
    delay(200);
  }
  for (int i = 7; i<=39; i++) {
    SetDigit(1, (i)%10);
    SetDigit(2, (i-1)%10);
    SetDigit(3, (i-2)%10);
    SetDigit(4, (i-3)%10);
    SetDigit(5, (i-4)%10);
    SetDigit(6, (i-5)%10);
    shiftOut();
    delay(200);
  }
  for (int i = 10; i<=16; i++) {
    SetDigit(1, i);
    SetDigit(2, i-1);
    SetDigit(3, i-2);
    SetDigit(4, i-3);
    SetDigit(5, i-4);
    SetDigit(6, i-5);
    shiftOut();
    delay(200);
  }
}

// Sets a digit of the display
void SetDigit(int Digit, int Number) {
  
  byte BinNum;
  if ( Number == 1) {	
    BinNum= B0000;
  }
  else if ( Number == 2) {	
    BinNum= B1001;
  }
  else if ( Number == 3) {	
    BinNum= B1000;
  }
  else if ( Number == 4) {	
    BinNum= B0111;
  }
  else if ( Number == 5) {	
    BinNum= B0110;
  }
  else if ( Number == 6) {	
    BinNum= B0101;
  }
  else if ( Number == 7) {	
    BinNum= B0100;
  }
  else if ( Number == 8) {	
    BinNum= B0011;
  }
  else if ( Number == 9) {	
    BinNum= B0010;
  }
  else if ( Number == 0) {	
    BinNum= B0001;
  }
  else {
    BinNum= B1010;
  }
  

  
  if ( Digit == 1) {	
    DataOut1 &= B00001111;
    DataOut1 |= (BinNum << 4);
  }
  else if ( Digit == 2) {	
    DataOut1 &= B11110000;
    DataOut1 |= BinNum;
  }
  else if ( Digit == 3) {	
    DataOut2 &= B00001111;
    DataOut2 |= (BinNum << 4);
  }
  else if ( Digit == 4) {	
    DataOut2 &= B11110000;
    DataOut2 |= BinNum;
  }
  else if ( Digit == 5) {	
    DataOut3 &= B00001111;
    DataOut3 |= (BinNum << 4);
  }
  else if ( Digit == 6) {	
    DataOut3 &= B11110000;
    DataOut3 |= BinNum;
  }
  
}  

// Shifts out the data to the shift registers
void shiftOut() {
  // This shifts 8 bits out MSB first, 
  //on the rising edge of the clock,
  //clock idles low

  //internal function setup
  int i=0;
  int pinState;

  //clear everything out just in case to
  //prepare shift register for bit shifting
  //ground latchPin and hold low for as long as you are transmitting
  digitalWrite(latchPin, 0);
  digitalWrite(dataPin, 0);
  digitalWrite(clockPin, 0);

  //for each bit in the byte myDataOutï¿½
  //NOTICE THAT WE ARE COUNTING DOWN in our for loop
  //This means that %00000001 or "1" will go through such
  //that it will be pin Q0 that lights. 
  for (i=7; i>=0; i--)  {
    digitalWrite(clockPin, 0);

    //if the value passed to myDataOut and a bitmask result 
    // true then... so if we are at i=6 and our value is
    // %11010100 it would the code compares it to %01000000 
    // and proceeds to set pinState to 1.
    if ( DataOut1 & (1<<i) ) {
      pinState= 1;
    }
    else {	
      pinState= 0;
    }

    //Sets the pin to HIGH or LOW depending on pinState
    digitalWrite(dataPin, pinState);
    //register shifts bits on upstroke of clock pin  
    digitalWrite(clockPin, 1);
    //zero the data pin after shift to prevent bleed through
    digitalWrite(dataPin, 0);
  }
  
  //stop shifting
  digitalWrite(clockPin, 0);
  
  // Shift the second word
  for (i=7; i>=0; i--)  {
    digitalWrite(clockPin, 0);

    if ( DataOut2 & (1<<i) ) {
      pinState= 1;
    }
    else {	
      pinState= 0;
    }
    
    digitalWrite(dataPin, pinState);
    digitalWrite(clockPin, 1);
    digitalWrite(dataPin, 0);
  }

  digitalWrite(clockPin, 0);
  
  // Shift the third word
  for (i=7; i>=0; i--)  {
    digitalWrite(clockPin, 0);

    if ( DataOut3 & (1<<i) ) {
      pinState= 1;
    }
    else {	
      pinState= 0;
    }

    digitalWrite(dataPin, pinState);
    digitalWrite(clockPin, 1);
    digitalWrite(dataPin, 0);
  }

  digitalWrite(clockPin, 0);
  
  digitalWrite(latchPin, 1);
}

// Read from the real time clock
void GetTime () {
  
  t = now();
}






///////////////////////////////////////////////////////////
// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, c);
      strip.show();
      delay(wait);
  }
}

void rainbowopen() {

  if(rainbow1<256){ 
    
    for(rainbow2=0; rainbow2<strip.numPixels(); rainbow2++) {
      strip.setPixelColor(rainbow2, Wheel((rainbow2+rainbow1) & 255));
    }
    rainbow1++;
  }
  else {
    rainbow1=0;
  }
  strip.show();
  delay(100);  
}


void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  if(WheelPos < 85) {
   return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if(WheelPos < 170) {
   WheelPos -= 85;
   return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
   WheelPos -= 170;
   return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}




//FUNCTIONS TO SET TIME FROM SERIAL PORT
//print date and time to Serial
void printDateTime(time_t t)
{
    printDate(t);
    Serial << ' ';
    printTime(t);
}

//print time to Serial
void printTime(time_t t)
{
    printI00(hour(t), ':');
    printI00(minute(t), ':');
    printI00(second(t), ' ');
}

//print date to Serial
void printDate(time_t t)
{
    printI00(day(t), 0);
    Serial << monthShortStr(month(t)) << _DEC(year(t));
}

//Print an integer in "00" format (with leading zero),
//followed by a delimiter character to Serial.
//Input value assumed to be between 0 and 99.
void printI00(int val, char delim)
{
    if (val < 10) Serial << '0';
    Serial << _DEC(val);
    if (delim > 0) Serial << delim;
    return;
}
