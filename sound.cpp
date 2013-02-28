
#include <windows.h>
#include <stdio.h>

#include "log.h"
#include "sound.h"

// number of samples that can be played back simultaneously

#define NCHANNELS 8

// sample rate in hz

#define SAMPLERATE 44100

// these constants defined to avoid the use of magic numbers

#define MUSICPARTS 3
#define STEREOCHANNELS 2
#define LEFTCHANNEL 0
#define RIGHTCHANNEL 1

HWAVEOUT wo;
WAVEFORMATEX wf;
WAVEHDR wh;

struct s_buffer
{
	short *data;
	short *nofx;
	int length;
	int pos_last;
	int pos_cur;
} buffer;

struct s_channel
{
    int a;
    s_sample *sample;
    int position;
} channel[NCHANNELS];

struct s_music
{
    int length;
    short *data;
    int position;
    int mix;
} music;

int cchannel;

extern int g_danger;

/*void loadsample(s_sample &s, char *fn)
{
    FILE *f=fopen(fn,"rb");
    fseek(f,0,SEEK_END);
    s.length=ftell(f)-58;
    fseek(f,58,SEEK_SET);
    s.data=new unsigned char[s.length];
    fread(s.data,s.length,1,f);
    fclose(f);
}

void killsample(s_sample &s)
{
    delete []s.data;
}

void playsample(s_sample &s)
{
    channel[cchannel].a=1;
    channel[cchannel].position=0;
    channel[cchannel].sample=&s;
    cchannel++;
    cchannel&=NCHANNELS-1;
}
*/

void Sound::loadmusic(char *fn)
{
    FILE *f=fopen(fn,"rb");
    fseek(f,0,SEEK_END);
    music.length=ftell(f)-56;
    music.length/=MUSICPARTS;
    music.length/=sizeof(short)*STEREOCHANNELS;
    fseek(f,56,SEEK_SET);
    music.data=new short[music.length*MUSICPARTS*sizeof(short)];
    fread(music.data,music.length*MUSICPARTS,sizeof(int),f);
    fclose(f);
    music.position=0;
    music.mix=0;
}

void Sound::init(int bufferLength)
{
    cchannel=0;

    // initialize sampling buffer

	buffer.data = new short[bufferLength*STEREOCHANNELS];

	buffer.length = bufferLength;
	buffer.pos_cur = 0;
	buffer.pos_last = 0;

	memset(buffer.data,0,buffer.length*sizeof(short)*STEREOCHANNELS);

	// initialize windows wave format structure

	memset(&wf,0,sizeof(WAVEFORMATEX));

	wf.wFormatTag=WAVE_FORMAT_PCM;
	wf.nChannels=STEREOCHANNELS;
	wf.nSamplesPerSec=SAMPLERATE;
	wf.nAvgBytesPerSec=SAMPLERATE*sizeof(short)*STEREOCHANNELS;
	wf.nBlockAlign=STEREOCHANNELS*sizeof(short);
	wf.wBitsPerSample=16;

	// initialize windows wave header structure

	memset(&wh,0,sizeof(WAVEHDR));

	wh.lpData=(char*)buffer.data;
	wh.dwBufferLength=buffer.length*STEREOCHANNELS*sizeof(short);
	wh.dwFlags=WHDR_BEGINLOOP|WHDR_ENDLOOP;
	wh.dwLoops=-1;

	int ret;

	// do the necessary windows api calls to initialize sound playback

	ret=waveOutOpen(&wo,WAVE_MAPPER,&wf,0,0,CALLBACK_NULL|WAVE_ALLOWSYNC);
	if (ret!=MMSYSERR_NOERROR)
	{
		char errtext[500];
		waveOutGetErrorText(ret,errtext,500);
		Log::log() << errtext;

		Log::log() << "waveOutOpen() failed";
	}

	ret=waveOutPrepareHeader(wo,&wh,sizeof(WAVEHDR));
	if (ret!=MMSYSERR_NOERROR) Log::log() << "waveOutPrepareHeader() failed";

	ret=waveOutWrite(wo,&wh,sizeof(WAVEHDR));
	if (ret!=MMSYSERR_NOERROR) Log::log() << "waveOutWrite() failed";

    for (int i=0;i<4;i++) channel[i].a=0;

}

void Sound::update(int targetMusicMix)
{
    int i,j,outleft,outright;
	MMTIME t;
	t.wType=TIME_SAMPLES;
	waveOutGetPosition(wo,&t,sizeof(t));
	buffer.pos_last=buffer.pos_cur;
	buffer.pos_cur=t.u.sample;

    for (i=buffer.pos_last;i<buffer.pos_cur;i++)
    {
        outleft=0;
        outright=0;

        /*for (j=0;j<NCHANNELS;j++)
        {
            if (channel[j].a)
            {
                out+=(int)channel[j].sample->data[channel[j].position]-127;
                channel[j].position++;
                if (channel[j].position>=channel[j].sample->length) channel[j].a=0;
            }
        }
        out*=128;*/

        if ((i&3)==0) { if (targetMusicMix>music.mix) music.mix++; else music.mix--; }


        for(j=0;j<MUSICPARTS;j++)
        {
            int m =music.mix-j*8192;
            if(m>8192) m=8192*2-m;
            if(m<0) m=0;
            outleft+=(int)music.data[ (music.position+music.length*j)*STEREOCHANNELS+LEFTCHANNEL]*m/8192;
            outright+=(int)music.data[ (music.position+music.length*j)*STEREOCHANNELS+RIGHTCHANNEL]*m/8192;
        }

        music.position++;
        if (music.position>=music.length) music.position=0;

        buffer.data[(i*STEREOCHANNELS+LEFTCHANNEL)&(buffer.length*STEREOCHANNELS-1)]=outleft;
        buffer.data[(i*STEREOCHANNELS+RIGHTCHANNEL)&(buffer.length*STEREOCHANNELS-1)]=outright;
    }

	wh.dwLoops=-1;

}


void Sound::deinit()
{
	int ret;

	ret=waveOutPause(wo);
	ret=waveOutReset(wo);

	ret=waveOutUnprepareHeader(wo,&wh,sizeof(WAVEHDR));
	if (ret!=MMSYSERR_NOERROR) Log::log() << "waveOutUnprepareHeader() failed\n";

	ret=waveOutClose(wo);
	if (ret!=MMSYSERR_NOERROR) Log::log() << "waveOutClose() failed\n";

	if (buffer.data) delete []buffer.data;


}

