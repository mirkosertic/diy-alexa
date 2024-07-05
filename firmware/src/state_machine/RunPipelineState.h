#ifndef _runpipelinestate_h_
#define _runpipelinestate_h_

#include "Speaker.h"
#include "Recorder.h"
#include "State.h"
#include "Homeassistant.h"

class RunPipelineState : public State
{
private:
    Speaker *m_speaker;
    Recorder *m_recorder;
    Homeassistant *m_homeassistant;
    bool m_pipelinestarted;
public:
    RunPipelineState(Speaker *speaker, Recorder *recorder, Homeassistant *homeassistant);
    ~RunPipelineState();

    void enterState();
    State* run();
    void exitState();
};

#endif