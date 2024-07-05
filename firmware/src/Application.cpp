#include <Arduino.h>
#include "Application.h"

// This task does all the heavy lifting for our application
void applicationTask(void *param)
{
  Application *application = static_cast<Application *>(param);

  const TickType_t xMaxBlockTime = pdMS_TO_TICKS(100);
  while (true)
  {
    // wait for some audio samples to arrive
    uint32_t ulNotificationValue = ulTaskNotifyTake(pdTRUE, xMaxBlockTime);
    if (ulNotificationValue > 0)
    {
      application->run();
    }
  }
}

Application::Application(Recorder *recorder, Speaker *speaker, IndicatorLight *indicator_light, Homeassistant *homeassistant, State *initialState)
{
    m_recorder = recorder;
    m_speaker = speaker;
    m_indicatorLight = indicator_light;
    m_homeassistant = homeassistant;
    // detect wake word state - waits for the wake word to be detected
    m_current_state = initialState;
    // start off in the detecting wakeword state
    m_current_state->enterState();
}

void Application::start()
{
    xTaskCreatePinnedToCore(applicationTask, "Application Task", 8192, this, 1, &m_taskHandle, 1);
    //m_speaker->start();
    //m_indicatorLight->start();
    m_recorder->start(&m_taskHandle);
    m_homeassistant->start();
}

void Application::loop()
{
    m_speaker->loop();
    m_homeassistant->loop();
}

// process the next batch of samples
void Application::run()
{
    State* nextstate = m_current_state->run();
    if (m_current_state != nextstate)
    {
        m_current_state->exitState();
        delete m_current_state;

        m_current_state = nextstate;

        // switch to the next state - very simple state machine so we just go to the other state...
        m_current_state->enterState();
    }
    vTaskDelay(10);
}
