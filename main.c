// spi.c
//
// Example program for bcm2835 library
// Shows how to interface with SPI to transfer a byte to and from an SPI device
//
// After installing bcm2835, you can build this 
// with something like:
// gcc -o spi spi.c -l bcm2835
// sudo ./spi
//
// Or you can test it before installing with:
// gcc -o spi -I ../../src ../../src/bcm2835.c spi.c
// sudo ./spi
//
// Author: Mike McCauley
// Copyright (C) 2012 Mike McCauley
// $Id: RF22.h,v 1.21 2012/05/30 01:51:25 mikem Exp $
#include <bcm2835.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "main.h"

static inline int spiInit(void)
{
  // If you call this, it will not actually access the GPIO
  // Use for testing
  //        bcm2835_set_debug(1);
  if (!bcm2835_init())
    return 1;
  bcm2835_spi_begin();
  bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);      // The default
  bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);                   // The default
  bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_16
); // The default
  bcm2835_spi_chipSelect(BCM2835_SPI_CS0);                      // The default
  bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS0, LOW);      // the default
  // Send a byte to the slave and simultaneously read a byte back from the slave
  printf("Initialized SPI\n");
}
static inline int spiDeinit(void)
{
  bcm2835_spi_end();
  bcm2835_close();
  printf("Deinitialized SPI\n");
}

void setPixel(uint8_t red, uint8_t green, uint8_t blue, pixel_t *pixel)
{
  *pixel = 0x0080; //Set first bit, clear the rest
  *pixel |= ((blue & 0x1F) << 2) + ((red & 0x18)>>3) + ((red & 7) << 13) + ((green & 0x1F)<<8); //build the pixel
}

void sendFrame(pixel_t *firstPixel, uint32_t numPixels)
{
  uint32_t numStopBytes = numPixels/8+1;
  const uint32_t zeros[512] = {0};

  bcm2835_spi_writenb( (char*) zeros,4); //send 32 0 bits to start the frame
  for(uint8_t i = 0; i < numPixels; i++)
    {
      pixel_t *pixelAddress = firstPixel + i;
      bcm2835_spi_writenb((char*) pixelAddress,2); //Send out a pixel
    }
  bcm2835_spi_writenb( (char*) &zeros,numStopBytes); //send out a zero bit for each pixel
}

int main(int argc, char **argv)
{
  uint8_t j = 0;
  pixel_t *buffer = malloc(sizeof(pixel_t)*253);
  spiInit();
  while(1)
    {
      for(uint8_t i = 0;i<253;i++)
	{
	  if(i == j)
	    {
	      setPixel(0x00,0x00,0xFF,buffer + i);
	    }
	  else
	    {
	      setPixel(0,0,0,buffer+i);
	    }
	}
      j++;
      if(j>=253) j = 0;
      sendFrame(buffer,250);
      //sleep(1);
   }  


  spiDeinit();
}
