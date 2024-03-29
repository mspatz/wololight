#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "lpd6803.h"

int spiInit(void)
{
  // If you call this, it will not actually access the GPIO
  // Use for testing
  //        bcm2835_set_debug(1);
  if (!bcm2835_init())
    return 1;
  bcm2835_spi_begin();
  bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);      // Has no effect
  bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);                   // The default
  bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_64);    // The default
  bcm2835_spi_chipSelect(BCM2835_SPI_CS0);                      // The default
  bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS0, LOW);      // the default
  // Send a byte to the slave and simultaneously read a byte back from the slave
  printf("Initialized SPI\n");
  
  return 0;
}

void spiDeinit(void)
{
  bcm2835_spi_end();
  bcm2835_close();
  printf("Deinitialized SPI\n");
}


void setPixel(uint8_t red, uint8_t green, uint8_t blue, pixel_t *pixel)
{
  *pixel = 0x8000; //Set first bit, clear the rest
  *pixel |= ((red & 0x1F) << 10) + ((green & 0x1F)<<5) + ((blue & 0x1F)); //build the pixel
}

void setPixelHSV(float hue, float sat, float val, pixel_t *pixel)
{
  float C;
  float X;
  float M;
  uint8_t C5b;
  uint8_t X5b;
  uint8_t M5b;

  C = val*sat;
  M = val-C;
  hue /= 60;
  X = C*(1-fabs(fmod(hue,2)-1));

  C5b = (uint8_t)(C*31);
  X5b = (uint8_t)(X*31);
  M5b = (uint8_t)(M*31);

  if(0.0<=hue && hue<1.0) 
    {
      setPixel(C5b+M5b,X5b+M5b,M5b,pixel);
    }
  else if(1.0<=hue && hue<2.0)
    {
      setPixel(X5b+M5b,C5b+M5b,M5b,pixel);
    }
  else if(2.0<=hue && hue<3.0)
    {
      setPixel(M5b,C5b+M5b,X5b+M5b,pixel);
    }
  else if(3.0<=hue && hue<4.0) 
    {
      setPixel(M5b,X5b+M5b,C5b+M5b,pixel);
    }
  else if(4.0<=hue && hue<5.0)
    {
      setPixel(X5b+M5b,M5b,C5b+M5b,pixel);
    }
  else if(5.0<=hue && hue<6.0)
    {
      setPixel(C5b+M5b,M5b,X5b+M5b,pixel);
    }
  else
    {
      setPixel(0,0,0,pixel);
    }
}


void sendFrame(pixel_t *firstPixel, uint32_t numPixels)
{
  uint32_t numStopBytes = numPixels/8+1;
  static const uint32_t zeros[512] = {0}; //a lot of zeros.

  bcm2835_spi_writenb( (char*) zeros,4); //send 32 0 bits to start the frame
  for(uint8_t i = 0; i < numPixels; i++)
    {
      pixel_t *pixelAddress = firstPixel + i;
      bcm2835_spi_transfern((char*) pixelAddress+1,1); //Send out a pixel
      bcm2835_spi_transfern((char*) pixelAddress,1);
    }
  bcm2835_spi_writenb( (char*) zeros,numStopBytes); //send out a zero bit for each pixel
}
