#include <Arduino.h>

#include "DetectWakeWordState.h"

#include "AudioProcessor.h"
#include "NeuralNetwork.h"
#include "RingBuffer.h"
#include "RunPipelineState.h"

#define WINDOW_SIZE 320
#define STEP_SIZE 160
#define POOLING_SIZE 6
#define AUDIO_LENGTH 16000

DetectWakeWordState::DetectWakeWordState(Speaker *speaker, Recorder *recorder, Homeassistant *homeassistant)
{
    // save the sample provider for use later
    m_speaker = speaker;
    m_recorder = recorder;
    m_homeassistant = homeassistant;
    // some stats on performance
    m_average_detect_time = 0;
    m_number_of_runs = 0;
}

void DetectWakeWordState::enterState()
{
    // Create our neural network
    m_nn = new NeuralNetwork();
    Serial.println("enterState() - Created Neral Net");
    // create our audio processor
    m_audio_processor = new AudioProcessor(AUDIO_LENGTH, WINDOW_SIZE, STEP_SIZE, POOLING_SIZE);
    Serial.println("enterState() - Created audio processor");
}

State* DetectWakeWordState::run()
{
    //Serial.println("running in state");
    // time how long this takes for stats
    long start = millis();
    // get access to the samples that have been read in
    RingBufferAccessor *reader = m_recorder->getRingBufferReader();
    // rewind by 1 second
    reader->rewind(16000);
    // get hold of the input buffer for the neural network so we can feed it data
    float *input_buffer = m_nn->getInputBuffer();
    // process the samples to get the spectrogram
    m_audio_processor->get_spectrogram(reader, input_buffer);
    // finished with the sample reader
    delete reader;
    // get the prediction for the spectrogram
    float output = m_nn->predict();
    long end = millis();
    // compute the stats
    m_average_detect_time = (end - start) * 0.1 + m_average_detect_time * 0.9;
    m_number_of_runs++;
    // log out some timing info
    if (m_number_of_runs == 20)
    {
        m_number_of_runs = 0;
        Serial.printf("run() - Average detection time %.fms\n", m_average_detect_time);
    }

    // use quite a high threshold to prevent false positives
    if (output > 0.90)
    {
    //        Serial.printf("run() - detect %10.5f\n", output);
        Serial.printf("run() - P(%.2f): Here I am, brain the size of a planet...\n", output);
        m_recorder->wipe();

        Serial.println("run() - Switching to RunPipelineState");
        return new RunPipelineState(m_speaker, m_recorder, m_homeassistant);
    }
    // nothing detected stay in the current state
    return this;
}

void DetectWakeWordState::exitState()
{
    // Create our neural network
    delete m_nn;
    m_nn = NULL;
    delete m_audio_processor;
    m_audio_processor = NULL;
    uint32_t free_ram = esp_get_free_heap_size();
    Serial.printf("exitState() - Free ram after DetectWakeWord cleanup %d\n", free_ram);
}