#include "pitches.h"
#include <Adafruit_NeoPixel.h>
#define PIN 6
Adafruit_NeoPixel strip = Adafruit_NeoPixel(60, PIN, NEO_GRB + NEO_KHZ800);

//global variables
byte Candle[9][6]; //2D 8x6 array of integers holding the on/off state. 0 is off, 1 is the color from CandleColor array, 3 is flame.  First value is the candle number, second value is the LED along the candle, starting from the bottom.
//Candle [0][x] is the Shamesh. There are a total of 9 candles for 8 days.
//Candles are initially 4 pixels long with 2 pixels for the flame.
short CandleState;//current length of the candles. 4 to start, 0 when done.
byte CandleColor[9][6];//a 2D array of bytes. First value is the candle number with the 3  parts of the color (R, G, B)
byte FlameColors[4][3] = {// Four colors, three RGB values.
  {239, 227, 53},
  {245, 200, 49},
  {255, 171, 44},
  {238, 120, 41}
};
byte Palette[8][3] = { // 7 colors
  {128, 0, 0},    //Red
  {128, 0, 128},  //Magenta
  {128, 0, 64},   //Pink
  {0, 0, 128},    //Blue
  {0, 128, 128},  //Cyan
  {0, 128, 0},    //Green
  {128, 128, 0},  //Yellow
  {128, 64, 0}    //Orange
};
#define RED 0
#define MAGENTA 1
#define PINK 2
#define BLUE 3
#define CYAN 4
#define GREEN 5
#define YELLOW 6
#define ORANGE 7

void setup() {
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  randomSeed(analogRead(0));
  Serial.begin(9600);
}

void loop() {
  Serial.println("Happy Hannukah");
  for (byte Day = 1; Day < 9; Day++) { //which day of Hannukah is it 1 - 8
    Serial.print("Day ");
    Serial.println(Day);
    CandleState = 4;//initialize all candles to be 4 pixels long
    SetCandleColor(Day);
    for (byte CandleNo = 0; CandleNo < Day + 1; CandleNo++) { //loop through each candle to turn on
      for (byte Pix = 0; Pix < CandleState; Pix++) { //should be 0 to 3, for a total of 4 values
        Candle[CandleNo][Pix] = 1; //1 means it's a part of a candle and on.
      }
    }
    SetupCandles(Day); //Turn on candles but not burning
    Serial.println("Candles On but not lit");
    PlaySong();
    delay(10 * 1000); //30 seconds ****debug***
    //delay (5000);//5 sec
    for (byte CandleNo = 0; CandleNo < Day + 1; CandleNo++) { //loop through and light candles
      //Set top 2 pixesl to flames
      Candle[CandleNo][4] = 2; // 2 is the number for flames
      Candle[CandleNo][5] = 2;
      SetupCandles(Day);
      delay(3 * 1000); //wait 3 sec between lighting each candle***debug***
      //delay(2000);
    }
    Serial.println("Candles lit");
    //candles are now on and lit. flicker and burn them down slowly
    unsigned long LTime = millis();
    const unsigned long BurnTime = 120000;//120,000 milliseconds = 120 seconds***debug***
    //Serial.println(LTime);
    while (CandleState > -1) { //***Main Loop
      Flicker(Day);
      //every 2 minutes burn down a pixel
      unsigned long CTime = millis();
      if (CTime - LTime > BurnTime) {
        Serial.println("burning down");
        Serial.print("CandleState = ");
        Serial.println(CandleState);
        //reset Ltime
        LTime = CTime;
        FlickerDown(Day);
        Serial.println("Flicker Down");
      }
      delay(100);// short delay so the flickering looks ok
    }
    Serial.print("End of day ");
    Serial.println(Day);
    delay(30000);
  }
  Serial.println("End of Hannukah");
  delay(30000);
}


void SetCandleColor(byte Day) {
  //TODO: make more interesting candle colors. Initially this is making them all green.
  //ideas - random colors from a set of good colors
  //pre-planned patterns like red and green alternating
  if (Day == 8) {
    for (byte i = 0; i < Day + 1 ; i++) { //candle number
      //Day 8 alternate red and green
      byte ColorNo;
      switch (i) {
        case 0:
        case 2:
        case 4:
        case 6:
        case 8:
          ColorNo = RED;
          break;
        default:
          ColorNo = GREEN;
      }
      Serial.print("Candle Color ");
      Serial.println(ColorNo);
      for (byte j = 0; j < 3; j++) { //r g b values
        CandleColor[i][j] = Palette[ColorNo][j];
      }
    }

  }
  byte PrevColor = 99;
  for (byte i = 0; i < Day + 1 ; i++) { //candle number
    //each candle with a random color
    byte ColorNo;
    do {//make sure two same color candles aren't next to each other
      ColorNo = random(8);
      //Serial.println(ColorNo);
    }
    while (ColorNo == PrevColor);
    PrevColor = ColorNo;
    Serial.print("Candle Color ");
    Serial.println(ColorNo);
    for (byte j = 0; j < 3; j++) { //r g b values
      CandleColor[i][j] = Palette[ColorNo][j];
    }
  }
}

void SetupCandles(byte Day) {
  //turns on pixels on LED strip based on Candle, CandleColor, and FlameColor arrays
  for (byte i = 0; i < Day + 1; i++) { //candle number
    for (byte j = 0; j < 6; j++) { //pixels
      //Serial.println(i * 6 + j);
      if (Candle[i][j] == 1) {
        //it's a candle pixel
        strip.setPixelColor(i * 6 + j, CandleColor[i][0], CandleColor[i][1], CandleColor[i][2], CandleColor[i][3]);
      }
      else if (Candle[i][j] == 2) {
        //it's a flame pixel
        byte FC;
        FC = random(4);
        strip.setPixelColor(i * 6 + j, FlameColors[FC][0] / 2, FlameColors[FC][1] / 2, FlameColors[FC][2] / 2);
      }
    }
  }
  strip.show();
}
void Flicker(byte Day) {
  //run through Candle[][] and find the flame and change the intensity and colors
  for (byte i = 0; i < Day + 1; i++) { //candle number
    for (byte j = 0; j < 6; j++) { //pixels
      if (Candle[i][j] == 2) {
        //it's a flame pixel
        byte FC;
        FC = random(4);
        byte Intensity;
        Intensity = random(250) + 1;
        byte r;
        byte g;
        byte b;
        r = FlameColors[FC][0] / Intensity;
        g = FlameColors[FC][1] / Intensity;
        b = FlameColors[FC][2] / Intensity;
        strip.setPixelColor(i * 6 + j, r, g, b);
      }
    }
  }
  strip.show();
}

void FlickerDown(byte Day)
{

  byte r;
  byte g;
  byte b;

  //delay 100 each loop. Run through 100 iterations. Each time turn it down a notch
  byte Intensity = 200;
  for (byte n = 1; n < 100; n++) {
    Intensity = n + 2;
    //Serial.print("Intensity = ");
    //Serial.println(Intensity);
    for (byte i = 0; i < Day + 1; i++) { //candle number
      // CandleState + 1 and CandleState + 2 are flame pixels
      byte FC;
      FC = random(4);
      r = FlameColors[FC][0] / Intensity;
      g = FlameColors[FC][1] / Intensity;
      b = FlameColors[FC][2] / Intensity;
      strip.setPixelColor(i * 6 + CandleState, r, g, b);
      strip.setPixelColor(i * 6 + CandleState + 1, r, g, b);
    }
    strip.show();
    delay(100);
  }
  //now move candles down a pixel. After decrementing CandleState, set CandleState+1 and +2 to Flame. CandleState+3 to off
  if (CandleState > 0) {
    CandleState--;
    for (byte i = 0; i < Day + 1; i++) { //candle number
      Candle[i][CandleState] = 2;
      Candle[i][CandleState + 1] = 2;
      Candle[i][CandleState + 2] = 0;
      strip.setPixelColor(i * 6 + CandleState, r, g, b);
      strip.setPixelColor(i * 6 + CandleState + 2, 0);
    }
  }
  else
  { //flicker out last pixel
    CandleState--;//should be -1 now
    for (byte i = 0; i < Day + 1; i++) { //candle number
      Candle[i][CandleState + 1] = 2;
      Candle[i][CandleState + 2] = 0;
      strip.setPixelColor(i * 6, r, g, b);
      strip.setPixelColor(i * 6 + 1, 0);
      strip.setPixelColor(i * 6  + 2, 0);
    }
    strip.show();
    //flicker last pixel for a while
    for (byte n = 1; n < 100; n++) {
      Intensity = n + 2;
      //Serial.print("Intensity = ");
      //Serial.println(Intensity);
      for (byte i = 0; i < Day + 1; i++) { //candle number
        byte FC;
        FC = random(4);
        r = FlameColors[FC][0] / Intensity;
        g = FlameColors[FC][1] / Intensity;
        b = FlameColors[FC][2] / Intensity;
        strip.setPixelColor(i * 6, r, g, b);
      }
      strip.show();
      delay(100);
    }
    for (byte i = 0; i < Day + 1; i++) { //candle number
      strip.setPixelColor(i * 6, 0);//turn last pixel off completely
    }
    strip.show();
  }
}
void PlaySong() {
  int melody[] = {
    NOTE_F3, NOTE_C3, NOTE_F3, NOTE_AS3, NOTE_A3, NOTE_G3, NOTE_F3,
    NOTE_C4, NOTE_D4, NOTE_G3, NOTE_A3, NOTE_AS3, NOTE_A3, NOTE_G3, NOTE_F3
  };
  // note durations: 4 = quarter note, 8 = eighth note, etc.:
  byte noteDurations[] = {
    4, 4, 4, 4, 4, 4, 2,
    4, 4, 4, 8, 8, 4, 4, 2
  };
  //play twice
  for (int n = 0; n < 2 ; n++) {
    // iterate over the notes of the melody:
    for (int thisNote = 0; thisNote < 16; thisNote++) {

      // to calculate the note duration, take one second
      // divided by the note type.
      //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
      int noteDuration = 1000 / noteDurations[thisNote];
      tone(8, melody[thisNote], noteDuration);

      // to distinguish the notes, set a minimum time between them.
      // the note's duration + 30% seems to work well:
      int pauseBetweenNotes = noteDuration * 1.30;
      delay(pauseBetweenNotes);
      // stop the tone playing:
      noTone(8);
    }
  }
}

