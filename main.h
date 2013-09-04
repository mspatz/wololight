#include <jack/jack.h>

extern float *messenger;

typedef struct
{
  time_t timestamp;
  jack_default_audio_sample_t level;
} peak_t;;
