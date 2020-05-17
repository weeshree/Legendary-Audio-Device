#include "fftlcd2.h"


#define SAMPLES 256
#define SAMPLING_FREQUENCY 10000 // Hz


VisualizerL2::VisualizerL2(TFT_eSPI thatTft)
{

  FFT = arduinoFFT();
  
  tft = thatTft;

  sampling_period_us = round(1000000 * 1./SAMPLING_FREQUENCY);
  pinMode(PIN_LCD, OUTPUT);
  digitalWrite(PIN_LCD, HIGH);
  

  tft.fillScreen(TFT_BLACK); //fill background

  
  bunny = true; // ensures we don't run multiple loops simultaneously

  startColor = 255 << 11 | 0 << 5 | 31;
  endColor = 255 << 11 | 63 << 5 | 0;

  prevNote = "";
  textColor = 0;
}

void VisualizerL2::softMaxer(double *arr, int len)
{
  double sum = 0.0;
  for(int i=2; i<len; i++) sum += arr[i];
  for(int i=2; i<len; i++) arr[i] /= sum;

   sum = 0.0;
  for(int i=2; i<len; i++) sum+=arr[i]*(arr[i]);
  for(int i=2; i<len; i++) arr[i] = arr[i]*(arr[i])/sum;
}



int VisualizerL2::getColor(double frac)
{
  
  byte newRed = ((startColor & 255<<11)>>11) * frac + ((endColor & 255<<11)>>11) * (1-frac);
  byte newGreen = ((startColor & 63<<5)>>5) * frac + ((endColor & 63<<5)>>5) * (1-frac);
  byte newBlue = (startColor & 31) * frac + (endColor & 31) * (1-frac);
  return newRed << 11 | newGreen << 5 | newBlue;

}

void VisualizerL2::draw() {

    for(int i=0; i<SAMPLES/2; i++){
    int newRadius = (int) (readings[i]/6.);
//    if(newRadius < ra[i])
      tft.drawCircle(xs[i], ys[i], ra[i], TFT_BLACK);
      tft.drawCircle(xs[i], ys[i], ra[i]-5, TFT_BLACK);

//    step(i);
    ra[i] = newRadius;
    
    if(ra[i]>0) {
      times[i] = millis();
      if(xs[i]<ra[i])
      {
        xs[i]+=ra[i];
      }
      double d = (1-i*1./(SAMPLES/2.)/.15);
      Serial.println(d);
      tft.drawCircle(xs[i], ys[i], ra[i], getColor(d));
      tft.drawCircle(xs[i], ys[i], ra[i]-5, getColor(d));
    }
    else
    {
      xs[i] = random(SCREEN_XMAX-10)+10;
      double fracUpScreen = (random(SCREEN_YMAX)/1./SCREEN_YMAX); 
      double frac2 = (i*1./(SAMPLES/2.)/.15 * 2);
      
      ys[i] = (int) (SCREEN_YMAX * pow(fracUpScreen, frac2));
    }
  }
}

void VisualizerL2::VisLoop() {
  digitalWrite(PIN_LCD, HIGH);
  if(!bunny) return;
  bunny = false; 
  
  // put your main code here, to run repeatedly:
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
  
  double standard = vReal[indOfMax]; // store some standardization number before shifting the amplitudes
  softMaxer(vReal,SAMPLES/2); // amplifies the loud frequencies and dulls the soft frequencies

  boolean loud = false;
  for(int i=2; i<SAMPLES/2; i++)
  {
    double frequency = i*10000 / SAMPLES;
    
    readings[i] = (SCREEN_YMAX * vReal[i]/vReal[indOfMax] * standard/MAX_AMPLITUDE); 
    readings[i] = (readings[i] - readings[i]%5);
    if(readings[i] <= 10) // ignore really low amplitude frequencies
      readings[i] = 0;
    else
      loud=true;
    
  }

  draw();
  
  bunny=true;
}
