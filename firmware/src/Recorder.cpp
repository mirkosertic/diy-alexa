#include <Arduino.h>
#include "Recorder.h"

Recorder::Recorder(AudioStream *source)
{
    Serial.println("Recorder() - Initializing with audio source stream");

    m_source = source;

    // allocate the audio buffers
    for (int i = 0; i < AUDIO_BUFFER_COUNT; i++)
    {
        m_audio_buffers[i] = new AudioBuffer();
    }
    m_write_ring_buffer_accessor = new RingBufferAccessor(m_audio_buffers, AUDIO_BUFFER_COUNT);
}

Recorder::~Recorder()
{
    delete m_write_ring_buffer_accessor;
    for (int i = 0; i < AUDIO_BUFFER_COUNT; i++)
    {
        delete m_audio_buffers[i];
    }
}

void Recorder::processStreamData(uint8_t *data, size_t bytesRead)
{
    int16_t *samples = (int16_t *)data;
    for (int i = 0; i < bytesRead / 2; i++)
    {
        m_write_ring_buffer_accessor->setCurrentSample(samples[i]);
        if (m_write_ring_buffer_accessor->moveToNextSample())
        {
            // trigger the processor task as we've filled a buffer
            xTaskNotify(*m_applicationTaskHandle, 1, eSetBits);
        }
    }
}

void recorderTask(void *param)
{
    const int BUFFER_LENGTH = 800;
    uint8_t buffer[BUFFER_LENGTH];

    Recorder *recorder = (Recorder *)param;
    while (true)
    {
        size_t read = recorder->m_source->readBytes(&buffer[0], BUFFER_LENGTH);
        if (read > 0)
        {
            recorder->processStreamData(&buffer[0], read);
            //recorder->m_source->write(&buffer[0], read);
        }
    }
}

void Recorder::start(TaskHandle_t *applicationTaskHandle)
{
    m_applicationTaskHandle = applicationTaskHandle;
    // start a task to read samples
    xTaskCreatePinnedToCore(recorderTask, "Stream Reader Task", 4096, this, 1, &m_taskHandle, 0);
}

RingBufferAccessor *Recorder::getRingBufferReader()
{
    RingBufferAccessor *reader = new RingBufferAccessor(m_audio_buffers, AUDIO_BUFFER_COUNT);
    // place the reaader at the same position as the writer - clients can move it around as required
    reader->setIndex(m_write_ring_buffer_accessor->getIndex());
    return reader;
}

void Recorder::init()
{
    //m_source->begin();
}

void Recorder::wipe()
{
    for (int i = 0; i < AUDIO_BUFFER_COUNT; i++)
    {
        m_audio_buffers[i]->wipe();
    }
}
