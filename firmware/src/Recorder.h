#ifndef __recorder_h__
#define __recorder_h__

#include "AudioTools.h"

#include "RingBuffer.h"

#define AUDIO_BUFFER_COUNT 11

class Recorder
{
private:
    AudioStream *m_source;
    // audio buffers
    AudioBuffer *m_audio_buffers[AUDIO_BUFFER_COUNT];
    RingBufferAccessor *m_write_ring_buffer_accessor;
    // current audio buffer
    int m_current_audio_buffer;
    // stream reader task
    TaskHandle_t m_taskHandle;

    TaskHandle_t *m_applicationTaskHandle;

    void processStreamData(uint8_t *data, size_t bytesRead);

public:
    Recorder(AudioStream *source);
    ~Recorder();

    void init();

    void start(TaskHandle_t *applicationTaskHandle);

    friend void recorderTask(void *param);

    RingBufferAccessor *getRingBufferReader();

    void wipe();
};

#endif