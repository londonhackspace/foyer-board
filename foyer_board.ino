#include <avr/pgmspace.h>
#include "font.h"

/*
green -> a
green stripe -> b
blue stripe ->c
blue -> g1
brown -> st
brown stripe -> oe
orange stripe -> r1
orange -> clk
*/


#define PIN_CLOCK  8
#define PIN_LATCH  3
#define PIN_A      4  
#define PIN_B      5
#define PIN_C      6
//data
#define PIN_COL1   7


#define PANELS  3
unsigned char buffer[8][PANELS * 8]; 



int pos = 0;
int dir = 1;

char st[40];
int stringLen = 4;
int displayWidth = 0;

char serialBuffer[40];
int serialPtr =0;

void setup(){
  Serial.begin(9600);
  //clear the frame buffer
  for(int r = 0; r < 8; r++){
    for (int c = 0; c < PANELS * 8 ; c++){
      buffer[r][c] = 0xFF;

    }
  }
 
 //configure pins
  pinMode(PIN_CLOCK, OUTPUT);
  pinMode(PIN_LATCH, OUTPUT);
  pinMode(PIN_A, OUTPUT);
  pinMode(PIN_B, OUTPUT);
  pinMode(PIN_C, OUTPUT);
  pinMode(PIN_COL1, OUTPUT);
  //second colour pin is tied high by wiring to prevent it lighting
    
  //set startup text and text length
  serialPtr = 20;
  setText((char*)"--LONDON HACKSPACE--");

}

void clearPixel(int x, int y){
  buffer[y][x / 8] |= 1 << (x %8);  

}

void setPixel(int x, int y){
  if( x < 0 || x > PANELS * 64 || y > 8 || y < 0) return;
  buffer[y][x/8] &= ~(1 << (x % 8));

}

//set the text to be displayed
void setText(char* text){
  Serial.println("setting text");
  strncpy(st, text, serialPtr);
  stringLen = serialPtr;
  displayWidth = 0;
  serialPtr = 0;
}

void loop(){
  //read serial port
  while(Serial.available()){
    char c = Serial.read();
    if(c == '\r'){
      setText(serialBuffer);
      
      
    } else {
      serialBuffer[serialPtr] = c;
      serialPtr++;
    }
  }
  
  //pass current frame buffer out to the board, clearing it as we go
  for(uint8_t r = 0; r < 8; r++){

    digitalWrite(PIN_A, r & 0x01);    
    digitalWrite(PIN_B, r & 0x02);    
    digitalWrite(PIN_C, r & 0x04);    

    digitalWrite(PIN_LATCH, LOW);
    for (int c = 0; c < PANELS * 8; c++){

      shiftOut(PIN_COL1, PIN_CLOCK, LSBFIRST,  buffer[r][c]);
      buffer[r][c] = 0XFF;
    }
    digitalWrite(PIN_LATCH, HIGH);

    

  }

  //scroll base text position to the left
  pos --;
  if (pos <= -displayWidth) pos = PANELS * 64;
  
  //draw the chars into the buffer for next frames
  //keeping track of the cursor position as chars are variable width
  int workingPos = pos;
  displayWidth = 0;
  for(int charInd = 0; charInd < stringLen; charInd++){
    if( st[charInd] == ' '){
      workingPos += 4;
      displayWidth += 4;
    } else {
      int curChar = st[charInd] - 33;
      FONT_CHAR_INFO inf = charMap[curChar];
  
  
      
      for(int row = 0; row < 8; row ++){
        int charData = pgm_read_byte_near(bitmaps + inf.offset + row);
        for(int xp = inf.widthBits; xp >=0; xp--){
          if((charData >> (8 - xp) ) & 0x1){
            setPixel(workingPos + xp, row);
    
          }
        }
      }
      workingPos += inf.widthBits + 1;
      displayWidth += inf.widthBits + 1;
    }
  }



  

}







