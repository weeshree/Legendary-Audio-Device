#include "LedMatrix.h"

/**
 * Heavily influenced by the code and the blog posts from https://github.com/nickgammon/MAX7219_Dot_Matrix
 */
LedMatrix::LedMatrix(byte numberOfDevices, int8_t sck, int8_t miso, int8_t mosi, byte slaveSelectPin)
//  LedSPI(sck,miso,mosi,slaveSelectPin)
{
    myNumberOfDevices = numberOfDevices;
    mySlaveSelectPin = slaveSelectPin;
    cols = new byte[numberOfDevices * 8];
    _sck = sck;
    _miso = miso;
    _mosi = mosi;
}

LedMatrix::LedMatrix()
{
  
}

/**
 *  numberOfDevices: how many modules are daisy changed togehter
 *  slaveSelectPin: which pin is controlling the CS/SS pin of the first module?
 */
void LedMatrix::init() {
    pinMode(mySlaveSelectPin, OUTPUT);
    
    LedSPI.begin ( _sck,  _miso,  _mosi,  mySlaveSelectPin);
    LedSPI.setDataMode(SPI_MODE0);
    LedSPI.setClockDivider(SPI_CLOCK_DIV128);
    for(byte device = 0; device < myNumberOfDevices; device++) {
        sendByte (device, MAX7219_REG_SCANLIMIT, 7);   // show all 8 digits
        sendByte (device, MAX7219_REG_DECODEMODE, 0);  // using an led matrix (not digits)
        sendByte (device, MAX7219_REG_DISPLAYTEST, 0); // no display test
        sendByte (device, MAX7219_REG_INTENSITY, 0);   // character intensity: range: 0 to 15
        sendByte (device, MAX7219_REG_SHUTDOWN, 1);    // not in shutdown mode (ie. start it up)
    }
}

void LedMatrix::sendByte (const byte device, const byte reg, const byte data) {
    digitalWrite(15, HIGH);
    
    int offset=device;
    int maxbytes=myNumberOfDevices;
    
    for(int i=0;i<maxbytes;i++) {
        spidata[i] = (byte)0;
        spiregister[i] = (byte)0;
    }
    // put our device data into the array
    spiregister[offset] = reg;
    spidata[offset] = data;
    // enable the line
    digitalWrite(mySlaveSelectPin,LOW);
    // now shift out the data
    for(int i=0;i<myNumberOfDevices;i++) {
        LedSPI.transfer (spiregister[i]);
        LedSPI.transfer (spidata[i]);
    }
    digitalWrite (mySlaveSelectPin, HIGH);
    digitalWrite(15, LOW);
    
}

void LedMatrix::sendByte (const byte reg, const byte data) {
    for(byte device = 0; device < myNumberOfDevices; device++) {
        sendByte(device, reg, data);
    }
}


void LedMatrix::clear() {
    for (byte col = 0; col < myNumberOfDevices * 8; col++) {
        cols[col] = 0;
    }
    
}

void LedMatrix::commit() {
    for (byte col = 0; col < myNumberOfDevices * 8; col++) {
        sendByte(col / 8, col % 8 + 1, cols[col]);
    }
}

void LedMatrix::setColumn(int column, byte value) {
    if (column < 0 || column >= myNumberOfDevices * 8) {
        return;
    }
    cols[column] = value;
}

void LedMatrix::flash(boolean red)
{
  for(byte i=12; i<19; i++)
  {
    for(byte j=3; j<5; j++)
      setPixelAdj(i,j);
  }
}

void LedMatrix::fillColumn(byte x, byte y) {
  // have to map 0-31, 0-8 to pos
  
  byte ypos = x%8;
  byte xpos = 8*((31-x)/8);

  for(byte i=0; i<y; i++)
    bitWrite(cols[xpos+i], ypos, true);
}

void LedMatrix::setPixelAdj(byte x, byte y) {
  // have to map 0-31, 0-8 to pos
  
  byte ypos = x%8;
  byte xpos = 8*((31-x)/8);

  bitWrite(cols[xpos+y], ypos, true);
}

void LedMatrix::setPixel(byte x, byte y) {
    bitWrite(cols[x], y, true);
}

void LedMatrix::clearPixel(byte x, byte y) {
    bitWrite(cols[x], y, false);
}
