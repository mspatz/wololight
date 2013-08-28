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
	  if(1)
	    {
	      setPixelHSV(fmod((float)(i+j)/(float)NUMPIXELS,1)*360,1,1,buffer + i);
	    }
	  else
	    {
	      setPixel(0,0,0,buffer+i);
	    }
	}
      j++;
      if(j>=NUMPIXELS) j = 0;
      sendFrame(buffer,NUMPIXELS);
      //usleep(100);
   }  


  spiDeinit();
}
