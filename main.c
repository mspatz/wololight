#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "lpd6803.h"

int main(int argc, char **argv)
{
  uint8_t j = 0;
  pixel_t *buffer = malloc(sizeof(pixel_t)*NUMPIXELS);
  spiInit();
  while(1)
    {
      for(uint8_t i = 0;i<NUMPIXELS;i++)
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
      if(j>=NUMPIXELS) j = 0;
      sendFrame(buffer,NUMPIXELS);
      usleep(1000);
   }  


  spiDeinit();
}
