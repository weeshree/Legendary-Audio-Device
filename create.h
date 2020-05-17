#ifndef create_h
#define create_h

#include <Arduino.h>
#include "XT_DAC_Audio.h"
#include "fftvis.h"

#include <TFT_eSPI.h> // for debugging

class Creator {
  private:
    const boolean goodMods[11] = {1,0,1,0,1,1,0,1,0,1,1};
    const String letterNotes = "C0D0EF0G0A0Bc0d0ef0g0a0b";
    const int POT_PIN = 13;
    const int POT_PIN2 = 14;
    const int POT_PIN3 = 26;
    const int BUT_PIN = 16;
    const int BUT_PIN2 = 5;

    int noteInd;
    int noteInd2;
    int notetimer;
    double duration;
    double standard;
    char notes[500][10];

    boolean boomed;
    boolean loopin;
    int bassDropCt; 
    int bassDropInc;
    int curBar;
    int prevBar;
    int lastPress;
    int lastLoop;
    int lastUnpress;

    int drawCt;

  public:
    Creator();
    XT_DAC_Audio_Class DacAudio;
    XT_Wav_Class eight;
    VisualizerV visV;
    void CreateLoop(TFT_eSPI tft);
    void build(TFT_eSPI tft);
    void setupMatrix();
};

#endif
