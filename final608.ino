#include "create.h" // live playing
#include "fftled.h" // central bulge
#include "fftled2.h" // waterfall
#include "fftlcd.h" // histogram
#include "fftlcd2.h" // circles

TFT_eSPI tft = TFT_eSPI();

Creator tyler;
Visualizer vis;
Visualizer2 vis2;
VisualizerL visL(tft);
VisualizerL2 visL2(tft);

int state = 0;

int but_pin = 16;

int lastUnpress = 0;
int lastPress = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  pinMode(but_pin, INPUT_PULLUP);
  
  tft.init();
  tft.setRotation(2);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);

  tyler.setupMatrix();
}

void loop() {
  // put your main code here, to run repeatedly:
  if(digitalRead(but_pin))
  {
    if(millis() - lastUnpress > 500)
    {
      tft.fillScreen(TFT_BLACK);
      vis.clearScreen();
      state+=1;
      state%=5;
    }
    lastUnpress = millis();
  }
  else lastPress = millis();

  if(state == 0)
  {
    tyler.CreateLoop(tft); 
  }
  else if(state==1)
  {
    vis.VisLoop();
  }
  else if(state==2)
  {
    vis2.VisLoop();
  }  
  else if(state==3)
  {
    visL.VisLoop();
  }
  else if(state==4)
  {
    visL2.VisLoop();
  }
}
