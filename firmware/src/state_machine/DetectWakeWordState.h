#ifndef _detect_wake_word_state_h_
#define _detect_wake_word_state_h_

#include "Speaker.h"
#include "Recorder.h"
#include "State.h"
#include "AudioProcessor.h"
#include "NeuralNetwork.h"
#include "Homeassistant.h"


class DetectWakeWordState : public State
{
private:
    Speaker* m_speaker;
    Recorder *m_recorder;
    Homeassistant *m_homeassistant;
    NeuralNetwork *m_nn;
    AudioProcessor *m_audio_processor;
    float m_average_detect_time;
    int m_number_of_runs;

public:
    DetectWakeWordState(Speaker* speaker, Recorder *recorder, Homeassistant *homeassistant);
    void enterState();
    State* run();
    void exitState();
};

#endif
