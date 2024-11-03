/*
 *  audio_sdl.cpp - Audio support, implementation for SDL
 *
 *  Basilisk II (C) 1997-2008 Christian Bauer
 *  SDL audio implementation (C) 2023 James Higgs
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "sysdeps.h"

#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>

#include "cpu_emulation.h"
#include "main.h"
#include "prefs.h"
#include "user_strings.h"
#include "audio.h"
#include "audio_defs.h"

#include <SDL2/SDL.h>

#define DEBUG 0
#include "debug.h"


// The currently selected audio parameters (indices in audio_sample_rates[] etc. vectors)
static int audio_sample_rate_index = 0;
static int audio_sample_size_index = 0;
static int audio_channel_count_index = 0;

// Global variables
static SDL_AudioDeviceID audio_dev_id = 0;          // SDL audio device ID
static sem_t audio_irq_done_sem;                    // Signal from interrupt to streaming thread: data block read
static bool sem_inited = false;                     // Flag: audio_irq_done_sem initialized
static int sound_buffer_size;                       // Size of sound buffer in bytes
static uint8 silence_byte;                          // Byte value to use to fill sound buffers with silence
static pthread_t stream_thread;                     // Audio streaming thread
static pthread_attr_t stream_thread_attr;           // Streaming thread attributes
static bool stream_thread_active = false;           // Flag: streaming thread installed
static volatile bool stream_thread_cancel = false;  // Flag: cancel streaming thread

// Prototypes
static void *stream_func(void *arg);


/*
 *  Initialization
 */

// Set AudioStatus to reflect current audio stream format
static void set_audio_status_format(void)
{
    AudioStatus.sample_rate = audio_sample_rates[audio_sample_rate_index];
    AudioStatus.sample_size = audio_sample_sizes[audio_sample_size_index];
    AudioStatus.channels = audio_channel_counts[audio_channel_count_index];
}


static bool open_audio(void)
{
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        fprintf(stderr, "WARNING: Cannot initialize SDL audio (%s)\n", SDL_GetError());
        return false;
    }

    SDL_AudioSpec desired, obtained;

    // Set audio parameters
    SDL_memset(&desired, 0, sizeof(desired));
    desired.freq = audio_sample_rates[audio_sample_rate_index] >> 16;
    desired.format = (audio_sample_sizes[audio_sample_size_index] == 8) ? AUDIO_U8 : AUDIO_S16SYS;
    desired.channels = audio_channel_counts[audio_channel_count_index];
    desired.samples = audio_frames_per_block;
    desired.callback = NULL; // use queue instead of callback

    audio_dev_id = SDL_OpenAudioDevice(NULL, 0, &desired, &obtained, 0);
    if (audio_dev_id == 0) {
        fprintf(stderr, "WARNING: Cannot open SDL audio device (%s)\n", SDL_GetError());
        return false;
    }

    printf("Using SDL audio output\n");

    // SDL supports a variety of audio formats
    if (audio_sample_sizes.empty()) {

        // The reason we do this here is that we don't want to add sample
        // rates etc. unless the SDL audio device could be opened
        audio_sample_rates.push_back(11025 << 16);
        audio_sample_rates.push_back(22050 << 16);
        audio_sample_rates.push_back(44100 << 16);
        audio_sample_sizes.push_back(8);
        audio_sample_sizes.push_back(16);
        audio_channel_counts.push_back(1);
        audio_channel_counts.push_back(2);

        // Default to highest supported values
        audio_sample_rate_index = audio_sample_rates.size() - 1;
        audio_sample_size_index = audio_sample_sizes.size() - 1;
        audio_channel_count_index = audio_channel_counts.size() - 1;
    }

    sound_buffer_size = (audio_sample_sizes[audio_sample_size_index] >> 3) * audio_channel_counts[audio_channel_count_index] * audio_frames_per_block;
    silence_byte = 0;  // Is this correct for 8-bit mode?
    set_audio_status_format();

    // Start streaming thread
    Set_pthread_attr(&stream_thread_attr, 0);
    stream_thread_active = (pthread_create(&stream_thread, &stream_thread_attr, stream_func, NULL) == 0);

    // Everything went fine
    audio_open = true;
    return true;
}


void AudioInit(void)
{
    // Init audio status (reasonable defaults) and feature flags
    AudioStatus.sample_rate = 44100 << 16;
    AudioStatus.sample_size = 16;
    AudioStatus.channels = 2;
    AudioStatus.mixer = 0;
    AudioStatus.num_sources = 0;
    audio_component_flags = cmpWantsRegisterMessage | kStereoOut | k16BitOut;

    // Sound disabled in prefs? Then do nothing
    if (PrefsFindBool("nosound"))
        return;

    // Init semaphore
    if (sem_init(&audio_irq_done_sem, 0, 0) < 0)
        return;
    sem_inited = true;

    // Open and initialize audio device
    open_audio();
}


/*
 *  Deinitialization
 */

static void close_audio(void)
{
    // Stop stream and delete semaphore
    if (stream_thread_active) {
        stream_thread_cancel = true;
#ifdef HAVE_PTHREAD_CANCEL
        pthread_cancel(stream_thread);
#endif
        pthread_join(stream_thread, NULL);
        stream_thread_active = false;
    }

    // Close SDL audio device
    if (audio_dev_id != 0) {
        SDL_CloseAudioDevice(audio_dev_id);
        audio_dev_id = 0;
    }

    audio_open = false;
}

void AudioExit(void)
{
    // Close audio device
    close_audio();

    // Delete semaphore
    if (sem_inited) {
        sem_destroy(&audio_irq_done_sem);
        sem_inited = false;
    }

    SDL_Quit();
}


/*
 *  First source added, start audio stream
 */

void audio_enter_stream()
{
    // Streaming thread is always running to avoid clicking noises
    SDL_PauseAudioDevice(audio_dev_id, 0);
}


/*
 *  Last source removed, stop audio stream
 */

void audio_exit_stream()
{
    // Streaming thread is always running to avoid clicking noises
    SDL_PauseAudioDevice(audio_dev_id, 1);
}


/*
 *  Streaming function
 */

static void *stream_func(void *arg)
{
    int16 *silent_buffer = new int16[sound_buffer_size / 2];
    memset(silent_buffer, silence_byte, sound_buffer_size);

    while (!stream_thread_cancel){
        if (AudioStatus.num_sources) {

            // Trigger audio interrupt to get new buffer
            D(bug("stream: triggering irq\n"));
            SetInterruptFlag(INTFLAG_AUDIO);
            TriggerInterrupt();
            D(bug("stream: waiting for ack\n"));
            sem_wait(&audio_irq_done_sem);
            D(bug("stream: ack received\n"));

            // Get size of audio data
            uint32 apple_stream_info = ReadMacInt32(audio_data + adatStreamInfo);
            if (apple_stream_info) {
                int work_size = ReadMacInt32(apple_stream_info + scd_sampleCount) * (AudioStatus.sample_size >> 3) * AudioStatus.channels;
                D(bug("stream: work_size %d\n", work_size));
                if (work_size > sound_buffer_size)
                    work_size = sound_buffer_size;
                if (work_size == 0)
                    goto silence;

                // Send data to SDL
                if (work_size == sound_buffer_size) {
                    SDL_QueueAudio(audio_dev_id, Mac2HostAddr(ReadMacInt32(apple_stream_info + scd_buffer)), sound_buffer_size);
                } else {
                    // Last buffer or size mismatch
                    uint8 *last_buffer = new uint8[sound_buffer_size];
                    Mac2Host_memcpy(last_buffer, ReadMacInt32(apple_stream_info + scd_buffer), work_size);
                    memset(last_buffer + work_size, silence_byte, sound_buffer_size - work_size);
                    SDL_QueueAudio(audio_dev_id, last_buffer, sound_buffer_size);
                    delete[] last_buffer;
                }
                D(bug("stream: data written\n"));
            } else
                goto silence;

        } else {

            // Audio not active, play silence
			silence:    
			SDL_QueueAudio(audio_dev_id, silent_buffer, sound_buffer_size);
        }
    }
    delete[] silent_buffer;
    return NULL;
}


/*
 *  MacOS audio interrupt, read next data block
 */

void AudioInterrupt(void)
{
    D(bug("AudioInterrupt\n"));

    // Get data from apple mixer
    if (AudioStatus.mixer) {
        M68kRegisters r;
        r.a[0] = audio_data + adatStreamInfo;
        r.a[1] = AudioStatus.mixer;
        Execute68k(audio_data + adatGetSourceData, &r);
        D(bug(" GetSourceData() returns %08lx\n", r.d[0]));
    } else
        WriteMacInt32(audio_data + adatStreamInfo, 0);

    // Signal stream function
    sem_post(&audio_irq_done_sem);
    D(bug("AudioInterrupt done\n"));
}


/*
 *  Set sampling parameters
 *  "index" is an index into the audio_sample_rates[] etc. vectors
 *  It is guaranteed that AudioStatus.num_sources == 0
 */

bool audio_set_sample_rate(int index)
{
    close_audio();
    audio_sample_rate_index = index;
    return open_audio();
}

bool audio_set_sample_size(int index)
{
    close_audio();
    audio_sample_size_index = index;
    return open_audio();
}

bool audio_set_channels(int index)
{
    close_audio();
    audio_channel_count_index = index;
    return open_audio();
}


/*
 *  Get/set volume controls (volume values received/returned have the left channel
 *  volume in the upper 16 bits and the right channel volume in the lower 16 bits;
 *  both volumes are 8.8 fixed point values with 0x0100 meaning "maximum volume"))
 */

bool audio_get_main_mute(void)
{
    return false;
}

uint32 audio_get_main_volume(void)
{
    // SDL doesn't have separate main/speaker volume controls
    return audio_get_speaker_volume();
}

bool audio_get_speaker_mute(void)
{
    return false;
}

uint32 audio_get_speaker_volume(void)
{
    // This is a dummy implementation for SDL, returning maximum volume
    return 0x01000100;
}

void audio_set_main_mute(bool mute)
{
}

void audio_set_main_volume(uint32 vol)
{
    // SDL doesn't have separate main/speaker volume controls
    audio_set_speaker_volume(vol);
}

void audio_set_speaker_mute(bool mute)
{
}

void audio_set_speaker_volume(uint32 vol)
{
    // This is a dummy implementation for SDL
}