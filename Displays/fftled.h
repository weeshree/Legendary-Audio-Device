#ifndef fftled_h
#define fftled_h
#include "arduinoFFT.h"
#include "LedMatrix.h"

#define SAMPLES 256
#define SAMPLING_FREQUENCY 10000 // Hz

class Visualizer
{
  private:

  
  unsigned int sampling_period_us;
  unsigned long timer;
  const double MAX_AMPLITUDE = 40000;
  
  double vReal[SAMPLES];
  double vImag[SAMPLES];

  LedMatrix ledMatrix;
  arduinoFFT FFT;
  const int AUDIO_IN = A0; //pin where microphone is connected
    
  int positions[SAMPLES/2];

  public:
    void clearScreen();
    void updatePosArray();
    Visualizer();
    void softMaxer(double *arr, int len);
    void VisLoop();
};
#endif
