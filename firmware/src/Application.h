#ifndef _application_h_
#define _applicaiton_h_

#include "Recorder.h"
#include "Speaker.h"
#include "IndicatorLight.h"
#include "Homeassistant.h"
#include "state_machine/State.h"

class Application
{
private:
    State *m_current_state;
    Recorder *m_recorder;
    Speaker *m_speaker;
    Homeassistant *m_homeassistant;
    IndicatorLight *m_indicatorLight;
    TaskHandle_t m_taskHandle;
public:
    Application(Recorder *recorder, Speaker *speaker, IndicatorLight *indicator_light, Homeassistant *homeassistant, State *initialState);
    ~Application();

    void start();

    void loop();

    void run();

    TaskHandle_t *getTaskHandle();
};

#endif