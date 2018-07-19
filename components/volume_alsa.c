#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <alsa/asoundlib.h>
#include <alsa/mixer.h>
#include <sys/ioctl.h>
#include <unistd.h>

static long alsa_min, alsa_max, alsa_vol;

void
alsa_set_vol(long volume, const char *c, const char *elem_name)
{
    long min, max;
    snd_mixer_t *handle;
    snd_mixer_selem_id_t *sid;
    const char *card = c;
    const char *selem_name = elem_name;
    snd_mixer_open(&handle, 0);
    snd_mixer_attach(handle, card);
    snd_mixer_selem_register(handle, NULL, NULL);
    snd_mixer_load(handle);

    snd_mixer_selem_id_alloca(&sid);
    snd_mixer_selem_id_set_index(sid, 0);
    snd_mixer_selem_id_set_name(sid, selem_name);
    snd_mixer_elem_t* elem = snd_mixer_find_selem(handle, sid);

    snd_mixer_selem_get_playback_volume_range(elem, &min, &max);
    snd_mixer_selem_set_playback_volume_all(elem, volume * max / 100);

    snd_mixer_close(handle);
}

int
//alsa_get_vol(const char *c, const char *el)
alsa_get_vol(const char *c)
{

    snd_mixer_t *handle;
    snd_mixer_selem_id_t *sid;
    const char *card = c;
    const char *selem_name = "Master";

    #ifdef DEBUG
        printf("[ALSA] Selected card %s\n", card);
    #endif
    snd_mixer_open(&handle, 0);
    snd_mixer_attach(handle, card);
    snd_mixer_selem_register(handle, NULL, NULL);
    snd_mixer_load(handle);

    snd_mixer_selem_id_alloca(&sid);
    snd_mixer_selem_id_set_index(sid, 0);
    snd_mixer_selem_id_set_name(sid, selem_name);

    //Get the device initialized ...
    snd_mixer_elem_t *elem = snd_mixer_find_selem(handle, sid);

    long val;

    if (snd_mixer_selem_is_playback_mono(elem)) {
        snd_mixer_selem_get_playback_dB(elem, SND_MIXER_SCHN_MONO, &val);
        #ifdef DEBUG
            printf("[ALSA:mono] Value (DB) is: %f\n", (float)(val/100));
            float cur =  pow(10, ((float)(val/100))/20);
            printf("[ALSA:mono] Value is: %f %\n", cur);
        #endif
        return (float)(val/100);

    } else {
        int c, n = 0;
        long sum = 0;
        for (c = 0; c <= SND_MIXER_SCHN_LAST; c++) {
            if (snd_mixer_selem_has_playback_channel(elem, c)) {
                snd_mixer_selem_get_playback_volume(elem, SND_MIXER_SCHN_FRONT_LEFT, &val);
                sum += val;
                n++;
            }
        }
        if (! n) {
            return 0;
        }
        val = sum / n;
        #ifdef DEBUG
            printf("New val is %d\n", (int)val);
        #endif
        sum = (long)((double)(alsa_vol * (alsa_max - alsa_min)) / 100. + 0.5);
        if (sum != val) {
           alsa_vol = (long)(((val * 100.) / (alsa_max - alsa_min)) + 0.5);
        }
        return alsa_vol;
    }
}



/*int
main (int argc, char **argv)
{
    alsa_get_vol("default");
    return 0;
}*/
