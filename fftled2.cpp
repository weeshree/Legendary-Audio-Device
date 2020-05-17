#include "fftled2.h"

#define NUMBER_OF_DEVICES 4 //number of led matrix connect in series
#define CS_PIN 12
#define CLK_PIN 32
#define MISO_PIN 2 //we do not use this pin just fill to match constructor
#define MOSI_PIN 33

#define SAMPLES 256
#define SAMPLING_FREQUENCY 10000 // Hz


Visualizer2::Visualizer2()
{
  ledMatrix = LedMatrix(NUMBER_OF_DEVICES, CLK_PIN, MISO_PIN, MOSI_PIN, CS_PIN);

  FFT = arduinoFFT();
  
  ledMatrix.init();
  
  updatePosArray();
  sampling_period_us = round(1000000 * 1./SAMPLING_FREQUENCY);

  updatePosArray();
}


void Visualizer2::updatePosArray()
{
  for(int i=2; i<SAMPLES/2; i++)
  {
    double frequency = i*SAMPLING_FREQUENCY*1. / SAMPLES;
    double bassCutoff = 5*SAMPLING_FREQUENCY*1./SAMPLES; // around 200Hz
    
    if(frequency<=bassCutoff) // separate the bass sounds from the rest
    {
      double offset = log(2.*SAMPLING_FREQUENCY / SAMPLES);
      double posOfBox = (log(frequency) - offset) / (log(bassCutoff) - offset) * 32. / 5.; // bass sounds take up 1/5 of the screen
      positions[i] = (int) posOfBox;
    }
    else 
    {
      double offset = log(6*SAMPLING_FREQUENCY*1./SAMPLES);
      double posOfBox = (log(frequency)-offset) / (log(1760.) - offset) * 32. * 4./5. + 32./5.; // rest of sounds take up 4/5 of the screen. 1760 denotes the max frequency we can display given the tiny size of LCD.
      positions[i] = (int) posOfBox;
    }
    Serial.printf("%d %d %f\n",i,positions[i],frequency);
  }
}

void Visualizer2::softMaxer(double *arr, int len)
{
  double sum = 0.0;
  for(int i=2; i<len; i++) sum += arr[i];
  for(int i=2; i<len; i++) arr[i] /= sum;

   sum = 0.0;
  for(int i=2; i<len; i++) sum+=arr[i]*(arr[i]);
  for(int i=2; i<len; i++) arr[i] = arr[i]*(arr[i])/sum;
}

void Visualizer2::clearScreen() {
  ledMatrix.clear();
  ledMatrix.commit();
}

void Visualizer2::VisLoop() {
  ledMatrix.clear();
//  ledMatrix2.clear();
//  ledMatrix2.setPixel(5,7);
//  ledMatrix.setPixel(5,7);

  for(int i=0; i < SAMPLES; i++)
  {
    timer = micros();
    vReal[i] = analogRead(AUDIO_IN);
    vImag[i] = 0;
    while(micros() < timer+sampling_period_us) {}
  }
  
  FFT.Windowing(vReal, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.Compute(vReal, vImag, SAMPLES, FFT_FORWARD);
  FFT.ComplexToMagnitude(vReal, vImag, SAMPLES); // this part updates array vReal of length SAMPLES with the amplitudes for each frequency, i/SAMPLES * SAMPLE_SIZE, i ranging from 1 to SAMPLE_SIZE. Only the first SAMPLES/2 amplitudes are usable though.
  double peak = FFT.MajorPeak(vReal, SAMPLES, SAMPLING_FREQUENCY);

  int indOfMax = 2;
  for(int i=2; i<SAMPLES/2; i++)
  {
    if (vReal[i] > vReal[indOfMax]) indOfMax = i;
  }
  
  double standard = vReal[indOfMax];
//  softMaxer(vReal,SAMPLES/2); // amplifies the loud frequencies and dulls the soft frequencies
  
  int blah[32];
  for(int i=0; i<32; i++) blah[i]=0;
  
  for(int i=2; i<SAMPLES/2; i++)
  {
    if(blah[positions[i]] < ((2.+i/4.)*(vReal[i]/max(standard,MAX_AMPLITUDE))))
      blah[positions[i]] = (int) ((2.+i/4.)*(vReal[i]/max(standard,MAX_AMPLITUDE)));
  }

  for(int i=2; i<SAMPLES/2; i++)
  {
    if(positions[i] < 32)
    {  
      if(blah[positions[i]] > 0) pattern[0][positions[i]] = 1;
    }
  }

  for(int i=15; i>=0; i--)
  {
    for(int j=0; j<32; j++)
    {
      if(pattern[i][j]==1 && 15-i<8) ledMatrix.setPixelAdj(j,15-i);
//      if(pattern[i][j]==1 && 15-i>=8) ledMatrix2.setPixelAdj(j,15-i-8);
    }
    if(i!=15)
    {
      for(int j=0; j<32; j++) pattern[i+1][j] = pattern[i][j];
    }
  }
  for(int j=0; j<32; j++) pattern[0][j] = 0;




  ledMatrix.commit();
//  ledMatrix2.commit();
}
