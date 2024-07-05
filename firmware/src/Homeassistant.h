#ifndef _homeassistant_h_
#define _homeassistant_h_

#include <string>

#include <ArduinoJson.h>
#include <WebSocketsClient.h>
#include "Speaker.h"

enum HAState {
    DISCONNECTED, CONNECTED, AUTHREQUESTED, AUTHENTICATED, AUTHENTICATIONERROR, STARTED, FINISHED, RECORDING, STTFINISHED
};

class Homeassistant
{
private:
    Speaker *m_speaker;
    WebSocketsClient *m_webSocket;
    HAState m_state;
    uint8_t m_binaryHandler;
    std::string m_urlToSpeak;
    int m_commandid;

public:
    Homeassistant(Speaker *speaker);
    ~Homeassistant();

    void webSocketEvent(WStype_t type, uint8_t *payload, size_t length);

    void start();

    void sendAuthentication();

    bool startPipeline(bool includeWakeWordDetection);

    void sendAudioData(int16_t *data, int length);

    void loop();

    void reset();

    HAState state(); 
};

#endif