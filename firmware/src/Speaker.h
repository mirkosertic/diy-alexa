#ifndef _speaker_h_
#define _speaker_h_

#include "SPIFFS.h"
#include "AudioTools.h"

class Speaker
{
private:
    File m_ok;
    File m_ready_ping;

    URLStream *m_url;
    AudioStream *m_output;
    EncodedAudioStream *m_wavout;
    EncodedAudioStream *m_mp3out;
    StreamCopy *m_copier;

public:
    Speaker(AudioStream *output, AudioInfo info);
    ~Speaker();
    void playOK();
    void playReady();

    bool playAudioFromURL(const char* url);

    void loop();

    bool finished();
};

#endif