/**
The open night clock

INSTRUCTIONS: 
Upload the sketch. Etch the board. Slap on the parts and have fun.

Copyright (c) 2014 Ashwin Nayak www.ashwinnayak.com

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

*/

#include "Wire.h"
#define DS1307_ADDRESS 0x68

// Note all the pins mentioned below are the pins as displayed on the arduino board. The actul atmega pins are different. Refer the excel file for details.
// Display 1left most display Dipslay 4 is right most display

const byte disp_seg[4][8] = {
//A   B   C   D   E    F   G   DP
 {10, 9,  7,  8,  13, 11,  6,  12}, // Display 1
 {11, 12, 8,  7,  6,  10,  9,  13}, // Display 2
 {10, 9,  7,  8,  13, 11,  6,  12}, // Display 3
 {11, 12,  8, 7,  6,  10,  9,  13}  // Display 4
};


//Pins that the displays are connected to
const char disp_1 = 4;
const char disp_2 = 5;
const char disp_3 = 2;
const char disp_4 = 3;

boolean hour24 = false; // If true time format will be 24 hour format.
boolean amPmFlag = false; // If true its PM else its AM for 12 hour format.

boolean AdjustPinValue = false; // If true the time adjust push button is selected.

boolean currentHourChangePinValue, lastHourChangePinValue, currentMinuteChangePinValue, lastMinuteChangePinValue; // Will be used to debounce the push buttons

int hour;

char a, b, c, d, e, f, g, dp;

void setup(){

  // These pins connect up to the 7 segment led display segments
   pinMode(6, OUTPUT);
   pinMode(7, OUTPUT);
   pinMode(8, OUTPUT);
   pinMode(9, OUTPUT);
   pinMode(10, OUTPUT);
   pinMode(11, OUTPUT);
   pinMode(12, OUTPUT);
   pinMode(13, OUTPUT);
   
   // These pins connect up to the 4 transistors
   pinMode(disp_1, OUTPUT);
   pinMode(disp_2, OUTPUT);
   pinMode(disp_3, OUTPUT);
   pinMode(disp_4, OUTPUT);
   
   // These pins are used for adjusting the time and changing the hour format.
   pinMode(A0, INPUT_PULLUP); //The time adjust pin
   pinMode(A0, OUTPUT);
   pinMode(A1, INPUT_PULLUP); //The Hour Adjust pin
   pinMode(A1, OUTPUT);
   pinMode(A2, INPUT_PULLUP); //The minute Adjust pin. if the adjust in is not selected it will change the time format.
   pinMode(A2, OUTPUT);

  test_all_seg(); // Test sequence to check if all the sgments in the display are working and to show how multiplexing works.
  
}

void loop(){
  
  AdjustPinValue = digitalRead(A0); 
  
  if (AdjustPinValue == LOW) { // If the Adjust pin is pressed adjust the time
    captureHourChange();
    captureMinuteChange();
  }
  
  if (AdjustPinValue == HIGH){
   capture12to24HourChange(); 
  }
  
  if (hour24){
    hour = readCurrentHour();
  }
  else{
    hour = convert24to12(readCurrentHour());
  }
  
  display_all((hour * 100) + readCurrentMinute());
  
}

void display_all(int number){
  
 int units, tens, hundreds, thousands;
 units = number % 10;
 tens = (number/10)%10;
 hundreds = (number/100)%10;
 thousands = (number/1000)%10;
 
 number_display(units, 4);
 number_display(tens, 3);
 number_display(hundreds, 2);
 number_display(thousands, 1);
 
}

void number_display(int number, int display){
  
  //Set pins to segmets
  switch(display){
   case 1: 
     a = disp_seg[0][0];
     b = disp_seg[0][1];
     c = disp_seg[0][2];
     d = disp_seg[0][3];
     e = disp_seg[0][4];
     f = disp_seg[0][5];
     g = disp_seg[0][6];
     dp = disp_seg[0][7];
     break;
   case 2: 
     a = disp_seg[1][0];
     b = disp_seg[1][1];
     c = disp_seg[1][2];
     d = disp_seg[1][3];
     e = disp_seg[1][4];
     f = disp_seg[1][5];
     g = disp_seg[1][6];
     dp = disp_seg[1][7];
     break;
   case 3: 
     a = disp_seg[2][0];
     b = disp_seg[2][1];
     c = disp_seg[2][2];
     d = disp_seg[2][3];
     e = disp_seg[2][4];
     f = disp_seg[2][5];
     g = disp_seg[2][6];
     dp = disp_seg[2][7];
     break;
   case 4: 
     a = disp_seg[3][0];
     b = disp_seg[3][1];
     c = disp_seg[3][2];
     d = disp_seg[3][3];
     e = disp_seg[3][4];
     f = disp_seg[3][5];
     g = disp_seg[3][6];
     dp = disp_seg[3][7];
     break;
  }
  
  set_all_seg_low();
   
  switch (display){
   case 1:
     digitalWrite(disp_2, LOW);
     digitalWrite(disp_3, LOW);
     digitalWrite(disp_4, LOW);
     digitalWrite(disp_1, HIGH);
     break;
   case 2:
     digitalWrite(disp_1, LOW);
     digitalWrite(disp_3, LOW);
     digitalWrite(disp_4, LOW);
     digitalWrite(disp_2, HIGH);
     break;
   case 3: 
     digitalWrite(disp_1, LOW);
     digitalWrite(disp_2, LOW);
     digitalWrite(disp_4, LOW);
     digitalWrite(disp_3, HIGH);
     break;
   case 4:
     digitalWrite(disp_1, LOW);
     digitalWrite(disp_2, LOW);
     digitalWrite(disp_3, LOW);
     digitalWrite(disp_4, HIGH);
     break;
  }  
  
  if(display == 2 || display == 3 || (display == 1 && hour24 == false && amPmFlag == true)){
    blink_seg(dp);
  }
  
  if (display == 1 && hour24 == false && number == 0){
    // Will be used later for some other processing
  }
  else{
    //Set segments to numbers
    switch (number){
     case 1:       
        blink_seg(b);     
        blink_seg(c);    
        break;
     case 2:
        blink_seg(a);       
        blink_seg(b); 
        blink_seg(d);            
        blink_seg(e);   
        blink_seg(g);  
        break;
      case 3:
        blink_seg(a);       
        blink_seg(b);      
        blink_seg(c);       
        blink_seg(d);  
        blink_seg(g); 
        break;
      case 4:
        blink_seg(b);        
        blink_seg(c);   
        blink_seg(f);      
        blink_seg(g);  
        break;
      case 5:
        blink_seg(a); 
        blink_seg(c);
        blink_seg(d);
        blink_seg(f);
        blink_seg(g);
        break;
      case 6:
        blink_seg(a);   
        blink_seg(c);     
        blink_seg(d);     
        blink_seg(e);     
        blink_seg(f);     
        blink_seg(g);
        break;
      case 7:
        blink_seg(a);
        blink_seg(b);
        blink_seg(c);
        break;
      case 8:
        blink_seg(a); 
        blink_seg(b);
        blink_seg(c);
        blink_seg(d);
        blink_seg(e);
        blink_seg(f);
        blink_seg(g);
        break;
      case 9:
        blink_seg(a); 
        blink_seg(b);
        blink_seg(c);
        blink_seg(d);
        blink_seg(f);
        blink_seg(g);
        break;
      case 0:
        blink_seg(a);      
        blink_seg(b);      
        blink_seg(c);      
        blink_seg(d);     
        blink_seg(e);      
        blink_seg(f); 
        break;
    }
  }
  
}

void set_all_seg_low(){
  
    digitalWrite(a, LOW); 
    digitalWrite(b, LOW);
    digitalWrite(c, LOW);
    digitalWrite(d, LOW);
    digitalWrite(e, LOW);
    digitalWrite(f, LOW);
    digitalWrite(g, LOW);
    digitalWrite(dp, LOW);
    
}

void blink_seg(char x){
  
  digitalWrite(x, HIGH);
  delayMicroseconds(100);
  digitalWrite(x, LOW);
  
}

void blink_seg_test(char x){
  
  digitalWrite(x, HIGH);
  delay(100);
  digitalWrite(x, LOW);
  
}

void test_all_seg(){
  
  set_all_seg_low();

   digitalWrite(disp_2, LOW);
   digitalWrite(disp_3, LOW);
   digitalWrite(disp_4, LOW);
   digitalWrite(disp_1, HIGH);
   a = disp_seg[0][0];
   b = disp_seg[0][1];
   c = disp_seg[0][2];
   d = disp_seg[0][3];
   e = disp_seg[0][4];
   f = disp_seg[0][5];
   g = disp_seg[0][6];
   dp = disp_seg[0][7];
   blink_seg_test(a);
   blink_seg_test(b);
   blink_seg_test(c);
   blink_seg_test(d);
   blink_seg_test(e);
   blink_seg_test(f);
   blink_seg_test(g);
   blink_seg_test(dp);
   digitalWrite(disp_1, LOW);
   digitalWrite(disp_3, LOW);
   digitalWrite(disp_4, LOW);
   digitalWrite(disp_2, HIGH);
   a = disp_seg[1][0];
   b = disp_seg[1][1];
   c = disp_seg[1][2];
   d = disp_seg[1][3];
   e = disp_seg[1][4];
   f = disp_seg[1][5];
   g = disp_seg[1][6];
   dp = disp_seg[1][7];
   blink_seg_test(a);
   blink_seg_test(b);
   blink_seg_test(c);
   blink_seg_test(d);
   blink_seg_test(e);
   blink_seg_test(f);
   blink_seg_test(g);
   blink_seg_test(dp);
   digitalWrite(disp_1, LOW);
   digitalWrite(disp_2, LOW);
   digitalWrite(disp_4, LOW);
   digitalWrite(disp_3, HIGH);
   a = disp_seg[2][0];
   b = disp_seg[2][1];
   c = disp_seg[2][2];
   d = disp_seg[2][3];
   e = disp_seg[2][4];
   f = disp_seg[2][5];
   g = disp_seg[2][6];
   dp = disp_seg[2][7];
   blink_seg_test(a);
   blink_seg_test(b);
   blink_seg_test(c);
   blink_seg_test(d);
   blink_seg_test(e);
   blink_seg_test(f);
   blink_seg_test(g);
   blink_seg_test(dp);
   digitalWrite(disp_1, LOW);
   digitalWrite(disp_2, LOW);
   digitalWrite(disp_3, LOW);
   digitalWrite(disp_4, HIGH);
   a = disp_seg[3][0];
   b = disp_seg[3][1];
   c = disp_seg[3][2];
   d = disp_seg[3][3];
   e = disp_seg[3][4];
   f = disp_seg[3][5];
   g = disp_seg[3][6];
   dp = disp_seg[3][7];
   blink_seg_test(a);
   blink_seg_test(b);
   blink_seg_test(c);
   blink_seg_test(d);
   blink_seg_test(e);
   blink_seg_test(f);
   blink_seg_test(g);
   blink_seg_test(dp);
   
}

byte decToBcd(byte number){
  
  return ( (number / 10 * 16) + (number % 10) );
  
}

byte bcdToDec(byte number)  {
  
  return ( (number / 16 * 10) + (number % 16) );
  
}

int convert24to12(int number){
  
  if (number > 11){
    amPmFlag = true;
  }
  else{
    amPmFlag = false;
  }

  if (number > 12){
    number-=12;
  }
  
  if (number == 0){
   number = 12; 
  }
  
  return number;
  
}

int readCurrentMinute(){
  
  //Set the address to be read rom to the Minute Adddress
  Wire.beginTransmission(DS1307_ADDRESS);
  Wire.write(0x01);
  Wire.endTransmission();
  
  //Read 1 byte from the minute address
  Wire.requestFrom(DS1307_ADDRESS, 1);
  return (bcdToDec(Wire.read()));
  
}

int readCurrentHour(){
  
  //Set the address to be read rom to the Minute Adddress  
  Wire.beginTransmission(DS1307_ADDRESS);
  Wire.write(0x02);
  Wire.endTransmission();
  
  //Read 1 byte from the hour address
  Wire.requestFrom(DS1307_ADDRESS, 1);
  return (bcdToDec(Wire.read() & 0b00111111)); 
  
}

//Genric function to perform software debounce
boolean debounce(boolean last, int button){
  boolean current = digitalRead(button);
  if (last != current){
   delay(20);
   current = digitalRead(button);  
  }
  return current;
}

void captureHourChange(){
  
  currentHourChangePinValue = debounce(lastHourChangePinValue, A1);
  if(lastHourChangePinValue == HIGH && currentHourChangePinValue == LOW){ // When the hour change Pin goes from HIGH to LOW increment the hour value
     incrementHour();
  }
  lastHourChangePinValue = currentHourChangePinValue; 
  
}

void captureMinuteChange(){
  
  currentMinuteChangePinValue = debounce(lastMinuteChangePinValue, A2);
  if(lastMinuteChangePinValue == HIGH && currentMinuteChangePinValue == LOW){ // When the miunte change Pin goes from HIGH to LOW increment the minute value
     incrementMinute();
  }
  lastMinuteChangePinValue = currentMinuteChangePinValue; 
  
}

void capture12to24HourChange(){
  
  currentMinuteChangePinValue = debounce(lastMinuteChangePinValue, A2);
  if(lastMinuteChangePinValue == HIGH && currentMinuteChangePinValue == LOW){ // When the miunte change Pin goes from HIGH to LOW flip the 12/24 hour flag
     
     if (hour24 == true){
      hour24 = false;
     }
     else{
      hour24 = true;
     }
  }
  lastMinuteChangePinValue = currentMinuteChangePinValue;
  
}

void incrementHour(){
  
  int currentHour = readCurrentHour();
  currentHour++; //After Reading hour increment it by 1
  
  if (currentHour > 23) currentHour = 0;
  
  //Reset Seconds to Zero
  Wire.beginTransmission(DS1307_ADDRESS);
  Wire.write(0x00);
  Wire.write(0x00);
  Wire.endTransmission();
  
  //Update Hour value
  Wire.beginTransmission(DS1307_ADDRESS);
  Wire.write(0x02);
  Wire.write(decToBcd(currentHour));
  Wire.endTransmission();
  
}

void incrementMinute(){
  
  int currentMinute = readCurrentMinute();
  currentMinute++; //After Reading minute increment it by 1
  
  if (currentMinute > 59) currentMinute = 0;
  
  //Reset Seconds to Zero followed by updating the Minutes
  Wire.beginTransmission(DS1307_ADDRESS);
  Wire.write(0x00);
  Wire.write(0x00);
  Wire.write(decToBcd(currentMinute));
  Wire.endTransmission();
  
}
