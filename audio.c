#include <jack/jack.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
//#include <errno.h>
#include <unistd.h>
#include <sys/mman.h>
#include "main.h"
#include "audio.h"


jack_port_t *input_port;
jack_client_t *client;

//volatile jack_default_audio_sample_t envelope = 0;

void jack_shutdown (void *arg)
{
	exit (1);
}

static jack_default_audio_sample_t lowPass(jack_default_audio_sample_t x0)
{
  //Called at 1kHz, cutoff at 120Hz
 static double xHist[3] = {0,0,0};
 static double yHist[3] = {0,0,0};
 const double bCoeffs[3] = {0.468952844493525 ,0.937905688987051 ,0.468952844493525};
 const double aCoeffs[3] = {1.000000000000000,-0.982405793108395,0.347665394851723};

 xHist[2] = xHist[1];
 xHist[1] = xHist[0];
 xHist[0] = x0;
 
 yHist[2] = yHist[1];
 yHist[1] = yHist[0];
 
 yHist[0] = bCoeffs[2]*xHist[2] + bCoeffs[1] * xHist[1] + bCoeffs[0]*xHist[0] - aCoeffs[2]*yHist[2] - aCoeffs[1]*yHist[1];
 return (jack_default_audio_sample_t)yHist[0];
}

static jack_default_audio_sample_t envelopeFollow(jack_default_audio_sample_t sample)
{
  //rectify  then low pass with cutoff at 5Hz
 static double xHist[3] = {0,0,0};
 static double yHist[3] = {0,0,0};
 const double bCoeffs[3] = {0.000537169774812,0.001074339549624,0.000537169774812};
 const double aCoeffs[3] = { 1.000000000000000,-1.933380225879930,0.935528904979178};

 sample = fabs(sample);

 xHist[2] = xHist[1];
 xHist[1] = xHist[0];
 xHist[0] = sample;
 
 yHist[2] = yHist[1];
 yHist[1] = yHist[0];
 
 yHist[0] = bCoeffs[2]*xHist[2] + bCoeffs[1] * xHist[1] + bCoeffs[0]*xHist[0] - aCoeffs[2]*yHist[2] - aCoeffs[1]*yHist[1];
 return (jack_default_audio_sample_t)yHist[0];
}

static jack_default_audio_sample_t highPass(jack_default_audio_sample_t x0)
{
  //Called at 1kHz, cutoff at 1Hz
 static double xHist[3] = {0,0,0};
 static double yHist[3] = {0,0,0};
 const double bCoeffs[3] = {0.995566972017647,-1.991133944035294,0.995566972017647};
 const double aCoeffs[3] = {1.000000000000000,-1.991114292201654,.991153595868935};

 xHist[2] = xHist[1];
 xHist[1] = xHist[0];
 xHist[0] = x0;
 
 yHist[2] = yHist[1];
 yHist[1] = yHist[0];
 
 yHist[0] = bCoeffs[2]*xHist[2] + bCoeffs[1] * xHist[1] + bCoeffs[0]*xHist[0] - aCoeffs[2]*yHist[2] - aCoeffs[1]*yHist[1];
 return (jack_default_audio_sample_t)yHist[0];
}

static jack_default_audio_sample_t bandPass(jack_default_audio_sample_t x0)
{
 //Called at 1kHz, cutoffs at 110Hz and 190Hz
 static double xHist[5] = {0,0,0};
 static double yHist[5] = {0,0,0};
 const double bCoeffs[5] = {0.046131802093313,0,-0.092263604186626,0,0.046131802093313};
 const double aCoeffs[5] = {1.000000000000000,-2.007027835193962,2.338101932353617,-1.390239758870166,0.491812237222577};

 xHist[4] = xHist[3];
 xHist[3] = xHist[2];
 xHist[2] = xHist[1];
 xHist[1] = xHist[0];
 xHist[0] = x0;
 
 yHist[4] = yHist[3];
 yHist[3] = yHist[2];
 yHist[2] = yHist[1];
 yHist[1] = yHist[0];
 
 yHist[0] = bCoeffs[4]*xHist[4] + bCoeffs[3]*xHist[3] + bCoeffs[2]*xHist[2] + bCoeffs[1] * xHist[1] + bCoeffs[0]*xHist[0] - aCoeffs[4]*yHist[4] - aCoeffs[3]*yHist[3] - aCoeffs[2]*yHist[2] - aCoeffs[1]*yHist[1];
 return (jack_default_audio_sample_t)yHist[0];
}

static jack_default_audio_sample_t bandStop(jack_default_audio_sample_t x0)
{

 static double xHist[3] = {0,0,0};
 static double yHist[3] = {0,0,0};
 const double bCoeffs[3] = {0.940809296181594,-1.752943756671321,0.940809296181594};
 const double aCoeffs[3] = {1.000000000000000,-1.752943756671323,0.881618592363189};

 xHist[2] = xHist[1];
 xHist[1] = xHist[0];
 xHist[0] = x0;
 
 yHist[2] = yHist[1];
 yHist[1] = yHist[0];
 
 yHist[0] = bCoeffs[2]*xHist[2] + bCoeffs[1] * xHist[1] + bCoeffs[0]*xHist[0] - aCoeffs[2]*yHist[2] - aCoeffs[1]*yHist[1];
 return (jack_default_audio_sample_t)yHist[0];
}


int process (jack_nframes_t nframes, void *arg)
{

 static uint32_t count = 0;
 uint32_t level;

 jack_default_audio_sample_t *in = (jack_default_audio_sample_t *) jack_port_get_buffer (input_port, nframes);
 jack_default_audio_sample_t val;
 jack_default_audio_sample_t accum;
 jack_nframes_t frm;

 
  for (frm = 0; frm < nframes; frm++)
    {
      accum += *in++;
      if(count++==48)
	{
	  val = accum/48;
	  accum = 0;
	  count = 0;
	  val = bandPass(val);
	  val = envelopeFollow(val);
	  val = highPass(val);
	  *messenger = val;
	}
    }

  return 0;
}


void connectJack(void)
{
  const char *client_name = "wolo";
  const char *server_name = NULL;
  const char **capturePorts;
  jack_options_t options = JackNullOption;
  jack_status_t status;
  
  /* open a client connection to the JACK server */
  
  client = jack_client_open (client_name, options, &status, server_name);
  if (client == NULL) {
    fprintf (stderr, "jack_client_open() failed, status = 0x%2.0x\n", status);
    if (status & JackServerFailed) {
      fprintf (stderr, "Unable to connect to JACK server\n");
    }
    exit (1);
  }
  if (status & JackServerStarted) {
    fprintf (stderr, "JACK server started\n");
  }
  if (status & JackNameNotUnique) {
    client_name = jack_get_client_name(client);
    fprintf (stderr, "unique name `%s' assigned\n", client_name);
  }

  jack_set_process_callback (client, process, 0);
  jack_on_shutdown (client, jack_shutdown, 0);

  input_port = jack_port_register (client, "input",JACK_DEFAULT_AUDIO_TYPE,JackPortIsInput, 0);
  
  if (input_port == NULL) {
    fprintf(stderr, "no more JACK ports available\n");
    exit (1);
  }
  
  /* Tell the JACK server that we are ready to roll.  Our
   * process() callback will start running now. */
  
  if (jack_activate (client)) {
    fprintf (stderr, "cannot activate client");
    exit (1);
  }


  /*Connect to the first availible capture port*/

  capturePorts = jack_get_ports (client, NULL, NULL,JackPortIsPhysical|JackPortIsOutput);
  if (capturePorts == NULL) {
    fprintf(stderr, "no physical capture ports\n");
    exit (1);
  }
  else
    {
      fprintf(stderr,"attempting to connect to %s",capturePorts[0]);
    }

  if (jack_connect (client, capturePorts[0], jack_port_name (input_port))) {
    fprintf (stderr, "cannot connect input ports\n");
  }
  
  free (capturePorts);
  sleep(-1);
}
