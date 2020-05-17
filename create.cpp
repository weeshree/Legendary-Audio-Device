#include "create.h"
#include "notes.h"
#include "808.h"

Creator::Creator():
  DacAudio(25, 0), eight(eightarr)
{
  noteInd = noteInd2 = notetimer = 0;
  duration = standard = 200.;
  lastPress = lastLoop = lastUnpress = 0;
  prevBar = curBar = 0;
  boomed = false;
  loopin = false;
  bassDropCt = 0;
  bassDropInc = 0;
  drawCt = 0;
  pinMode(BUT_PIN, INPUT_PULLUP);
  pinMode(BUT_PIN2, INPUT_PULLUP);
}

void Creator::setupMatrix()
{
  visV.setupMatrix();
}

void Creator::build(TFT_eSPI tft)
{
  drawCt+=1;
  bassDropCt+=1;
  loopin = true;

  if(bassDropCt == 4)
  {
    bassDropCt = 0;
    bassDropInc = 0;
    loopin = false;
    DacAudio.Play(&eight, false);    
  }
}

void Creator::CreateLoop(TFT_eSPI tft)
{  
  if(digitalRead(BUT_PIN2))
  {
    if(millis() - lastUnpress > 700 && bassDropCt == 0 && millis()>2000 && millis()-lastLoop < 1000) 
    {
      lastUnpress = millis();
      build(tft);    
    }
    
    lastUnpress = millis();
  }
  if(bassDropCt == 0)
  {
    if(!digitalRead(BUT_PIN) && millis() - lastPress > 400)
    {
      lastPress = millis();
      DacAudio.Play(&eight, false);
      if(loopin) loopin = false;
    }
    if(eight.Playing && millis() - lastPress > 50)
    {
      DacAudio.DacVolume=50;
      DacAudio.FillBuffer();
    }
    else DacAudio.DacVolume = 10;
  
    if(!digitalRead(BUT_PIN2) && millis() - lastLoop > 400)
    {
      loopin = !loopin;
      lastLoop = millis();
    }    
  }
  else if(!digitalRead(BUT_PIN2) && millis()-lastLoop > 400 && lastUnpress > lastLoop) 
  {
    lastLoop = millis();
    build(tft);
  }

  if(!eight.Playing && notetimer + 1000.*duration/(pow(2.,1.*bassDropCt)) < micros() && noteInd2 < noteInd)
  {
    int note_number = 0;
    
    notetimer = max( notetimer + 1000.*duration/pow(2.,1.*bassDropCt), (double)(micros()) );
    String str = notes[noteInd2];

    for(int i=0; i<str.length(); i++)
      if(letterNotes.indexOf(str.charAt(i)) >= 0)
        note_number = letterNotes.indexOf(str.charAt(i))+49;

    for(int i=0; i<str.length(); i++)
    {
      if(str.charAt(i) == ',') note_number-=12;
      if(str.charAt(i) == '\'') note_number+=12;
      if(str.charAt(i) == '^' && str.indexOf('=') < 0) note_number+=1;
      if(str.charAt(i) == '_' && str.indexOf('=') < 0) note_number-=1;
      if(str.charAt(i) == '#' && str.indexOf('=') < 0) note_number+=1;
    }

    int add = (int) ((4095. - analogRead(POT_PIN3))*20./4095. - 10);

    if(bassDropCt == 0) visV.VisLoop(note_number-40);

//    tft.drawString(String(note_number), 40, 40, 4);
    
    note_number += add;
    if(bassDropCt > 0) note_number += bassDropInc;
    
    note_number = min(max(note_number,0), 88);
    

    if(str.indexOf('-') < 0) DacAudio.Play(&list[note_number]);  
    
    duration = standard * pow(2,(analogRead(POT_PIN2))*6./4095. - 2);

    String digs = "0123456789";
    boolean slashEncountered = false;
    for(int i=0; i<str.length(); i++)
    {
      if(digs.indexOf(str.charAt(i)) >= 0)
      {
        if(!slashEncountered) duration*=digs.indexOf(str.charAt(i));
        else duration /= digs.indexOf(str.charAt(i));
      }
      if(str.charAt(i) == '/') slashEncountered = true;
    }  

    tft.drawString(String(notetimer), 20,20,2);
    
    if(str.indexOf('z') >= 0) DacAudio.DacVolume = 0;

    else if(str.indexOf('@') >= 0) 
    {
      DacAudio.DacVolume = (int) (30 * (4095. - analogRead(POT_PIN)) / 4095.);
      if(!loopin || noteInd2 > curBar) 
      {
        prevBar = curBar;
        curBar = noteInd2;        
      }
    }
    
    else DacAudio.DacVolume = (int) (10 * (4095. - analogRead(POT_PIN)) / 4095.);      

    if(loopin)
    {
      if(noteInd2 > curBar) noteInd2+=1; // if in middle of measure, continue it
      else if(noteInd2 == curBar || noteInd2 == curBar-1) 
      {
        noteInd2 = prevBar; // loop 
        bassDropInc += 1;
        visV.flash(true);
      }
      else noteInd2+=1;
    }
    else noteInd2+=1;    
    
    DacAudio.FillBuffer();
  }
  else if(!eight.Playing && bassDropCt == 0)
  {
    visV.VisLoop(0);
  }
  else // flash on bass drops
  {
    visV.flash(true);
  }

  
  char inString[200] = "";
  while (Serial.available() > 0)
  {
    char dat = Serial.read();

    if(dat == '&')
    {
      if(strlen(inString) > 0)
      {
        strcpy(notes[noteInd], inString);
        noteInd += 1;
      }
      inString[0] = '\0';
    }
    else strncat(inString,&dat, 1);
  }

  
}
