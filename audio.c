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
  //Called at 1kHz, cutoff at 160Hz
 static double xHist[3] = {0,0,0};
 static double yHist[3] = {0,0,0};
 const double bCoeffs[3] = { 0.145323883877042,0.290647767754085,0.145323883877042};
 const double aCoeffs[3] = {1.00000000000000, -0.671029090774096,0.252324626282266};

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
 static double xHist[3] = {0,0,0};
 static double yHist[3] = {0,0,0};
 const double bCoeffs[3] = { .000155148423475721,.000310296846951441,.000155148423475721};
 const double aCoeffs[3] = {1.000000000000000,-1.964460580205232,0.965081173899135};

 sample = fabs(sample);

 xHist[2] = xHist[1];
 xHist[1] = xHist[0];
 xHist[0] = sample;
 
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
 jack_default_audio_sample_t lowPassed;
 jack_nframes_t frm;

 
  for (frm = 0; frm < nframes; frm++)
    {
      if(count++==48)
	{
	  lowPassed = lowPass(*in++);
	  *messenger = envelopeFollow(lowPassed);
	  /* level = (uint32_t)50*fabs(envelope*50); */
	  /* for(uint32_t i = 0; i < level; i++) */
	  /*   { */
	  /*     printf("."); */
	  /*   } */
	  /* printf("\n"); */
	  count = 0;
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
