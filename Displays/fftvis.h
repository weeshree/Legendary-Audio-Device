#ifndef fftvis_h
#define fftvis_h
//#include "Arduino.h"
#include "arduinoFFT.h"
//#include "math.h"
//#include <SPI.h>
#include "LedMatrix.h"

#define SAMPLES 256
#define SAMPLING_FREQUENCY 10000 // Hz

class VisualizerV
{
  private:
  
  unsigned int sampling_period_us;
  unsigned long timer;
  
  double vReal[SAMPLES];
  double vImag[SAMPLES];
  
  arduinoFFT FFT;
  LedMatrix ledMatrix;

  int pattern[16][32];

  int smol_pattern[8];
  
  const int AUDIO_IN = A0; //pin where microphone is connected

  const double MAX_AMPLITUDE = 40000;

  int zero_count;    
  int positions[SAMPLES/2];

  public:
    void updatePosArray();
    VisualizerV();
    void softMaxer(double *arr, int len);
    void VisLoop(int keypressed);
    void flash(boolean red);
    void setupMatrix();
};
#endif
