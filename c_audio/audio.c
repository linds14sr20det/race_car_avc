#include <alsa/asoundlib.h>
#include <alsa/pcm.h>
#include <math.h>
#define BUFFER_LEN 48000

static char *device = "default";                       //soundcard
snd_output_t *output = NULL;
float buffer [BUFFER_LEN];

int main(void)
{

}
