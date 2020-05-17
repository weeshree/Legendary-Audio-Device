#ifndef fftled2_h
#define fftled2_h
#include "arduinoFFT.h"
#include "LedMatrix.h"

#define SAMPLES 256
#define SAMPLING_FREQUENCY 10000 // Hz

class Visualizer2
{
  private:
  
  unsigned int sampling_period_us;
  unsigned long timer;
  
  double vReal[SAMPLES];
  double vImag[SAMPLES];
  
  arduinoFFT FFT;
  LedMatrix ledMatrix;

  int pattern[16][32];

  const int AUDIO_IN = A0; //pin where microphone is connected

  const double MAX_AMPLITUDE = 40000;

    
  int positions[SAMPLES/2];

  public:
    void updatePosArray();
    Visualizer2();
    void softMaxer(double *arr, int len);
    void clearScreen();
    void VisLoop();
};
#endif
