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
  pixel_t *buffer = malloc(sizeof(pixel_t)*NUMPIXELS);
  int pid;
  messenger = mmap(NULL,sizeof(float),PROT_WRITE|PROT_READ,MAP_SHARED|MAP_ANONYMOUS,-1,0);
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
      //fprintf(stderr,"prnt envelope= %f\n",*messenger);
      float val = *messenger*100;
      if(val > 1.0) val = 1.0;
      for(uint8_t i = 0;i<NUMPIXELS;i++)
	{
	  if(1)
	    {
	      setPixelHSV(fmod((float)(i+j)/(float)NUMPIXELS,1)*360,1,val,buffer + i);
	    }
	  else
	    {
	      setPixel(0,0,0,buffer+i);
	    }
	}
      j++;
      if(j>=NUMPIXELS) j = 0;
      sendFrame(buffer,NUMPIXELS);
      usleep(100);
   }  


  spiDeinit();
}
