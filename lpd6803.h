#include <stdint.h>
#include <bcm2835.h>
#define NUMPIXELS 187

typedef uint16_t pixel_t;

int spiInit(void);
void spiDeinit(void);
void setPixel(uint8_t,uint8_t,uint8_t,pixel_t*);
void setPixelHSV(float, float, uint8_t, pixel_t*);
void sendFrame(pixel_t*,uint32_t);
