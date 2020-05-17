#include "fftled.h"

#define NUMBER_OF_DEVICES 4 //number of led matrix connect in series
#define CS_PIN 12
#define CLK_PIN 32
#define MISO_PIN 2 //we do not use this pin just fill to match constructor
#define MOSI_PIN 33

#define SAMPLES 256
#define SAMPLING_FREQUENCY 1000 // Hz


Visualizer::Visualizer()
{
  ledMatrix =LedMatrix(NUMBER_OF_DEVICES, CLK_PIN, MISO_PIN, MOSI_PIN, CS_PIN);
  FFT = arduinoFFT();

  ledMatrix.init();
  
  sampling_period_us = round(10.);

  updatePosArray();
}


void Visualizer::updatePosArray()
{
  for(int i=2; i<SAMPLES/2; i++)
  {
    double frequency = i*SAMPLING_FREQUENCY*1. / SAMPLES;
    double bassCutoff = -3*SAMPLING_FREQUENCY*1./SAMPLES; // around 200Hz
    
    if(frequency<=bassCutoff) // separate the bass sounds from the rest
    {
      double offset = log(2.*SAMPLING_FREQUENCY / SAMPLES);
      double posOfBox = (log(frequency) - offset) / (log(bassCutoff) - offset) * 32. / 5.; // bass sounds take up 1/5 of the screen
      positions[i] = (int) posOfBox;
    }
    else 
    {
      double offset = log(2*SAMPLING_FREQUENCY*1./SAMPLES);
      double posOfBox = (log(frequency)-offset) / (log(750.) - offset) * 32.; // rest of sounds take up 4/5 of the screen. 1760 denotes the max frequency we can display given the tiny size of LCD.
      positions[i] = (int) posOfBox;
    }
    Serial.printf("%d %d %f\n",i,positions[i],frequency);
  }
}

void Visualizer::softMaxer(double *arr, int len)
{
  double sum = 0.0;
  for(int i=2; i<len; i++) sum += arr[i];
  for(int i=2; i<len; i++) arr[i] /= sum;

   sum = 0.0;
  for(int i=2; i<len; i++) sum+=arr[i]*(arr[i]);
  for(int i=2; i<len; i++) arr[i] = arr[i]*(arr[i])/sum;
}

void Visualizer::clearScreen() {
  ledMatrix.clear();
  ledMatrix.commit();
}
void Visualizer::VisLoop() {
  ledMatrix.clear();


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
    if(blah[positions[i]] < (8.*(vReal[i]/max(standard,MAX_AMPLITUDE))))
      blah[positions[i]] = (int) (8.*(vReal[i]/max(standard,MAX_AMPLITUDE)));
  }

  for(int i=2; i<SAMPLES/2; i++)
  {
    if(positions[i] < 32)
    {  
      blah[positions[i]] = blah[31-positions[i]] = max(blah[positions[i]], blah[31-positions[i]]);
    }
  }

  int avg[32];
  avg[0] = blah[0];
  for(int i=1; i<16; i++)
  {
    avg[i] = blah[i]; //max(blah[i], (blah[i-1]+blah[i+1])/2);
  }
  avg[1] = (avg[0]+avg[2])/2;
  avg[3] = (avg[2]+avg[4])/2;
  avg[5] = (avg[4]+avg[6])/2;
  for(int i=0; i<16; i++)
  {
    ledMatrix.fillColumn(15-i, avg[i]);
    ledMatrix.fillColumn(16+i, avg[i]);
  }




  ledMatrix.commit();
}
