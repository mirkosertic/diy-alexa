#include "AllHomeAssistantPipelineState.h"

AllHomeAssistantPipelineState::AllHomeAssistantPipelineState(Speaker *speaker, Recorder *recorder, Homeassistant *homeassistant)
{
    m_speaker = speaker;
    m_recorder = recorder;
    m_homeassistant = homeassistant;
    m_pipelinestarted = false;
}

AllHomeAssistantPipelineState::~AllHomeAssistantPipelineState()
{
}

void AllHomeAssistantPipelineState::enterState() 
{
    m_homeassistant->reset();
}

State* AllHomeAssistantPipelineState::run() 
{
    if (!m_pipelinestarted)
    {
        Serial.println("run() - Starting new pipeline");
        if (!m_homeassistant->startPipeline(true))
        {
            Serial.println("run() - Failed to start new pipeline");    
            return this;
        }
        m_pipelinestarted = true;
        return this;
    }

    if (m_homeassistant->state() == FINISHED) {
        if (m_speaker->finished())
        {
            Serial.println("run() - Finished.");
            m_pipelinestarted = false;

            m_recorder->init();
            m_homeassistant->reset();

            return this;
        }
        Serial.println("run() - Waiting to finish speaker output.");
        return this;
    }

    // get access to the samples that have been read in
    RingBufferAccessor *reader = m_recorder->getRingBufferReader();
    reader->rewind(1600);

    int16_t data[1600];
    for (int i = 0; i < 1600; i++) {
        data[i] = reader->getCurrentSample();
        reader->moveToNextSample();
    }

    delete reader;

    m_homeassistant->sendAudioData(data, 1600);
    
    return this;
}

void AllHomeAssistantPipelineState::exitState() 
{
}
