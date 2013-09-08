#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <math.h>
#include "main.h"
#include "lpd6803.h"
#include "audio.h"

float *messenger;

int main(int argc, char **argv)
{
  uint8_t j = 0;
  uint8_t k = 0;
  pixel_t *buffer = malloc(sizeof(pixel_t)*NUMPIXELS);
  int pid;
  messenger = mmap(NULL,sizeof(float),PROT_WRITE|PROT_READ,MAP_SHARED|MAP_ANONYMOUS,-1,0);
  time_t timestamp;
  peak_t lastPeak = {0,.00013};

  pid = fork();
  if(pid == 0)//child
    {
      setgid(1000);
      setuid(1000);
      connectJack();
    }

  spiInit();

  while(1)
    {
      timestamp = time(NULL);
      float val = *messenger;

      if(val >= lastPeak.level)
	{
	  // printf("new peak\n");
	  lastPeak.timestamp = timestamp;
	  lastPeak.level = val;
	}
      else if (timestamp - lastPeak.timestamp >= 10 && lastPeak.level > .00013)
	{
	  // printf("reducing threshold\n");
	  lastPeak.level *= .999723;
	}
      //printf("%f\n",lastPeak.level);
      //normalize
      val = val/lastPeak.level;

      if(val > 1.0) val = 1.0;
      else if(val < 0) val = 0;
      
      val = 1;
      for(uint8_t i = 0;i<NUMPIXELS;i++)
	{
	  if(1)
	    {
	      setPixelHSV(fmod(((float)(i+k)/NUMPIXELS),1)*360,1,val,buffer + i);
	    }
	  else
	    {
	      setPixel(0,0,0,buffer+i);
	    }
	}

      j++;
      if(j>=6)
	{
	  j = 0;
	  k++;
	  if(k>=NUMPIXELS) k = 0;
	}

      sendFrame(buffer,NUMPIXELS);
      usleep(1000);
   }  


  spiDeinit();
}
