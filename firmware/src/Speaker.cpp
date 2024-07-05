#include "Speaker.h"

#include "AudioCodecs/CodecMP3Helix.h"
//#include <HTTPClient.h>

Speaker::Speaker(AudioStream *output, AudioInfo info)
{
    m_output = output;
    m_url = new URLStream();
    //m_url->addRequestHeader("Authorization", "Bearer eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiIxMjA0ZmJmMGY5NTk0MWI5YmU3MGQ0YjVmMzk5YzRmZiIsImlhdCI6MTcxOTQyNjM3NywiZXhwIjoyMDM0Nzg2Mzc3fQ.Rg2tQuuEstv9HG80iqiHnUr3KmrQGvtOJWQb1qQ28M0");

    m_ok = SPIFFS.open("/ok.wav", "r");
    m_ready_ping = SPIFFS.open("/ready_ping.wav", "r");

    m_wavout = new EncodedAudioStream(m_output, new WAVDecoder());
    m_mp3out = new EncodedAudioStream(m_output, new MP3DecoderHelix());
    
    m_copier = new StreamCopy();

    m_wavout->begin(info);
    m_mp3out->begin(info);
}

Speaker::~Speaker()
{
    delete m_copier;
    delete m_wavout;
    delete m_mp3out;
}

void Speaker::playOK()
{
    m_ok.seek(0);
    m_copier->begin(*m_wavout, m_ok);
}

bool Speaker::playAudioFromURL(const char* url)
{  
    Serial.printf("playAudioFromURL() - Downloading data from %s\n", url);
    /*HTTPClient client;
    client.addHeader("Authorization", "Bearer eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiIxMjA0ZmJmMGY5NTk0MWI5YmU3MGQ0YjVmMzk5YzRmZiIsImlhdCI6MTcxOTQyNjM3NywiZXhwIjoyMDM0Nzg2Mzc3fQ.Rg2tQuuEstv9HG80iqiHnUr3KmrQGvtOJWQb1qQ28M0");
    if (client.begin(url))
    {
        int responseCode = client.GET();
        Serial.printf("playAudioFromURL() - Response code is: %u\n", responseCode);    
        if (responseCode == 200) {
        }
    }
    else {
        Serial.printf("playAudioFromURL() - Connection failed\n");            
    }*/
    //if (m_url->begin("http://stream.srg-ssr.ch/m/rsj/mp3_128", "*/*"))
    if (m_url->begin(url, "*/*"))
    {
        Serial.printf("playAudioFromURL() - Started\n");
        m_copier->begin(*m_mp3out, *m_url);
        return true;
    }
    return false;
}

void Speaker::playReady()
{
    m_ready_ping.seek(0);
    m_copier->begin(*m_wavout, m_ready_ping);
}

bool Speaker::finished()
{
    return m_copier->available() == 0;
}

void Speaker::loop()
{
    m_copier->copy();
}
