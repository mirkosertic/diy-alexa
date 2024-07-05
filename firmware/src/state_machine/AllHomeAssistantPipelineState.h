#ifndef _allhomeassistantpipelinestate_h_
#define _allhomeassistantpipelinestate_h_

#include "Speaker.h"
#include "Recorder.h"
#include "State.h"
#include "Homeassistant.h"

class AllHomeAssistantPipelineState : public State
{
private:
    Speaker *m_speaker;
    Recorder *m_recorder;
    Homeassistant *m_homeassistant;
    bool m_pipelinestarted;
public:
    AllHomeAssistantPipelineState(Speaker *speaker, Recorder *recorder, Homeassistant *homeassistant);
    ~AllHomeAssistantPipelineState();

    void enterState();
    State* run();
    void exitState();
};

#endif