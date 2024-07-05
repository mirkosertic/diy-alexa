#include <Arduino.h>
#include <WiFi.h>
#include <esp_task_wdt.h>
#include "SPIFFS.h"

#include "AudioTools.h"
#include "AudioLibs/AudioBoardStream.h"

#include "Application.h"
#include "Recorder.h"
#include "Speaker.h"
#include "IndicatorLight.h"

//#include "state_machine/DetectWakeWordState.h"
#include "state_machine/AllHomeAssistantPipelineState.h"

AudioInfo info(16000, 1, 16);
AudioBoardStream kit(AudioKitEs8388V1); // Access I2S as stream

Application *application;
Speaker *speaker;

// Initialize the client library
WiFiClient client;

void setup()
{
  Serial.begin(115200);
  Serial.println("setup() - Starting up");
  delay(1000);
  // start up wifi
  // launch WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin("WLANOMATC", "je5jsVtxnwxj");
  while (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    Serial.println("setup() - Waiting for WiFi connection...");
    delay(100);
  }
  Serial.printf("setup() - Total heap: %d\n", ESP.getHeapSize());
  Serial.printf("setup() - Free heap: %d\n", ESP.getFreeHeap());

  esp_task_wdt_init(10, false);

  Serial.println("setup() - Initialiting audio library");
  AudioLogger::instance().begin(Serial, AudioLogger::Warning);

  auto cfg = kit.defaultConfig(RXTX_MODE);
  cfg.copyFrom(info);
  cfg.sd_active = false;
  cfg.input_device = ADC_INPUT_LINE2; // input from microphone
  kit.setVolume(1.0);
  kit.begin(cfg);

  // SPIFFS must be mounted to get files for speaker output
  Serial.println("setup() - Initializing SPIFFS");
  SPIFFS.begin();

  // Recorder creates audio recordings and passes them down the pipeline
  Serial.println("setup() - Initializing Recorder");
  Recorder *recorder = new Recorder(&kit);
  recorder->init();

  // Speaker class is responsible for audio feedback
  Serial.println("setup() - Initializing Speaker");
  speaker = new Speaker(&kit, info);

  // indicator light to show when we are listening
  Serial.println("setup() - Initializing IndicatorLight");
  IndicatorLight *indicator_light = new IndicatorLight();

  Serial.println("setup() - Initializing Connector to Home Assistant");
  Homeassistant *assistant = new Homeassistant(speaker);

  State* initialState = new AllHomeAssistantPipelineState(speaker, recorder, assistant);

  // Heavy lifting goes into the application class
  Serial.println("setup() - Initializing Application");
  application = new Application(recorder, speaker, indicator_light, assistant, initialState);

  Serial.println("setup() - Starting Application");
  application->start();

  Serial.printf("setup() - Total heap: %d\n", ESP.getHeapSize());
  Serial.printf("setup() - Free heap: %d\n", ESP.getFreeHeap());

  Serial.println("setup() - Complete!");

  //speaker->playAudioFromURL("http://192.168.0.159:8192/api/tts_proxy/e8ffeefc6c0a3b1e46c04cadcfb3c303a62dd89a_de-de_4a1f90fc7a_tts.piper.mp3");
  //speaker->playAudioFromURL("http://stream.srg-ssr.ch/m/rsj/mp3_128");
}

void loop()
{
  //speaker->loop();
  application->loop();
  vTaskDelay(10);
}