#include "RunPipelineState.h"
#include "DetectWakeWordState.h"

RunPipelineState::RunPipelineState(Speaker *speaker, Recorder *recorder, Homeassistant *homeassistant)
{
    m_speaker = speaker;
    m_recorder = recorder;
    m_homeassistant = homeassistant;
    m_pipelinestarted = false;
}

RunPipelineState::~RunPipelineState()
{
}

void RunPipelineState::enterState() 
{
    m_homeassistant->reset();
}

State* RunPipelineState::run() 
{
    if (!m_pipelinestarted)
    {
        Serial.println("run() - Starting new pipeline");
        if (!m_homeassistant->startPipeline(false))
        {
            Serial.println("run() - Failed to start new pipeline");
            return this;
        }
        m_pipelinestarted = true;
        return this;
    }

    if (m_homeassistant->state() == FINISHED) {
        Serial.println("run() - Finished. Switching to DetectWakeWordState");   
        m_homeassistant->reset();       
        return new DetectWakeWordState(m_speaker, m_recorder, m_homeassistant);
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

void RunPipelineState::exitState() 
{
}
