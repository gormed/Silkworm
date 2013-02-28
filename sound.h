
#ifndef SOUND_H
#define SOUND_H

struct s_sample
{
    int length;
    unsigned char *data;
};

class Sound
{

    //void loadsample(s_sample &s, char *fn);
    //void killsample(s_sample &s);
    //void playsample(s_sample &s);

public:

    static void loadmusic(char *fn);

    static void init(int bufferLength);
    static void deinit();

    static void update(int targetMusicMix);

};


#endif
