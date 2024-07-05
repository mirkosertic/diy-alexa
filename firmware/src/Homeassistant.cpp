#include "Homeassistant.h"

void Homeassistant::webSocketEvent(WStype_t type, uint8_t *payload, size_t length)
{
  switch (type)
  {
  case WStype_DISCONNECTED:
  {
    Serial.printf("webSocketEvent() - Disconnected!\n");
    m_state = DISCONNECTED;
    break;
  }
  case WStype_CONNECTED:
  {
    Serial.printf("webSocketEvent() - Connected to url: %s\n", payload);
    m_state = CONNECTED;
    break;
  }
  case WStype_TEXT:
  {

    Serial.printf("webSocketEvent() - Event Debug: %s\n", payload);

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, payload);
    if (error)
    {
      Serial.printf("webSocketEvent() - Failed to parse: %s with %s\n", payload, error.f_str());
    }
    else
    {
      const char *type = doc["type"];
      Serial.printf("webSocketEvent() - Got event of type: %s\n", type);

      if (strcmp(type, "auth_required") == 0)
      {
        Serial.printf("webSocketEvent() - Sending authentication token\n");
        sendAuthentication();
      }
      if (strcmp(type, "auth_ok") == 0)
      {
        const char *haversion = doc["ha_version"];
        Serial.printf("webSocketEvent() - Authenticated against Homeassistant %s\n", haversion);
        m_state = AUTHENTICATED;
      }
      if (strcmp(type, "auth_invalid") == 0)
      {
        const char *message = doc["message"];
        Serial.printf("webSocketEvent() - Authenticated failure : %s\n", message);
        m_state = AUTHENTICATIONERROR;
      }
      if (strcmp(type, "event") == 0)
      {
        if (strcmp(doc["event"]["type"], "run-start") == 0)
        {
          m_binaryHandler = doc["event"]["data"]["runner_data"]["stt_binary_handler_id"];
          Serial.printf("webSocketEvent() - run-start received with binary handler id %d\n", m_binaryHandler);
          m_state = STARTED;
        }
        if (strcmp(doc["event"]["type"], "run-end") == 0)
        {
          Serial.printf("webSocketEvent() - run-end received\n");

          if (m_urlToSpeak.length() > 0) {
            Serial.printf("webSocketEvent() - Playing URL %s\n", m_urlToSpeak.c_str());

            if (!m_speaker->playAudioFromURL(m_urlToSpeak.c_str())) 
            {
              Serial.printf("webSocketEvent() - Failed to start playback.\n");              
            }
          }

          m_state = FINISHED;
        }
        if (strcmp(doc["event"]["type"], "wake_word-start") == 0)
        {
          Serial.printf("webSocketEvent() - wake_word-start received\n");
          m_state = RECORDING;
        }
        if (strcmp(doc["event"]["type"], "stt-start") == 0)
        {
          Serial.printf("webSocketEvent() - stt-start received\n");
          m_state = RECORDING;
          m_speaker->playReady();
        }
        if (strcmp(doc["event"]["type"], "vad-end") == 0)
        {
          Serial.printf("webSocketEvent() - vad-end received\n");
          m_state = STTFINISHED;

          uint8_t transferbuffer[1];
          transferbuffer[0] = m_binaryHandler;

          m_webSocket->sendBIN(transferbuffer, 1);
        }
        if (strcmp(doc["event"]["type"], "tts-end") == 0)
        {
          const char* ressource = doc["event"]["data"]["tts_output"]["url"];
          m_urlToSpeak.append("http://192.168.0.159:8080").append(&(ressource[4]));

          Serial.printf("webSocketEvent() - tts-end received. Playing URL %s\n", m_urlToSpeak.c_str());
        }
      }
    }
    break;
  }
  case WStype_BIN:
    Serial.printf("webSocketEvent() - get binary length: %u\n", length);
    //			hexdump(payload, length);

    // send data to server
    // webSocket.sendBIN(payload, length);
    break;
  case WStype_ERROR:
    break;
  case WStype_FRAGMENT_TEXT_START:
    break;
  case WStype_FRAGMENT_BIN_START:
    break;
  case WStype_FRAGMENT:
    break;
  case WStype_FRAGMENT_FIN:
    break;
  }
}

void Homeassistant::sendAuthentication()
{
    m_state = AUTHREQUESTED;
    JsonDocument auth;
    auth["type"] = "auth";
    auth["access_token"] = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiIwYTJmOTU1ZDYwNjY0YmI1YTc2NGU4ZDAyNTMwZTA1ZSIsImlhdCI6MTcxOTE0MTcxNiwiZXhwIjoyMDM0NTAxNzE2fQ.u2rLYy7Mc4VIQ9-x_25Ra2IRejvkXBsRX8lxvjBzPIM";

    char buffer[512];
    size_t buffersize = serializeJson(auth, buffer);
    m_webSocket->sendTXT(buffer);
}

Homeassistant::Homeassistant(Speaker *speaker)
{
    m_state = DISCONNECTED;
    m_speaker = speaker;
    m_commandid = 1;
    m_webSocket = new WebSocketsClient();
    m_webSocket->onEvent(std::bind(&Homeassistant::webSocketEvent, this,  std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}

void Homeassistant::start()
{
    m_webSocket->begin("192.168.0.159", 8123, "/api/websocket");
}

void Homeassistant::reset()
{
    if (m_state == FINISHED) {
      m_state = AUTHENTICATED;
    }
}

HAState Homeassistant::state()
{
    return m_state;
}

bool Homeassistant::startPipeline(bool includeWakeWordDetection)
{
    m_urlToSpeak = std::string();
    if (m_state == AUTHENTICATED) 
    {
      JsonDocument runcmd;
      runcmd["id"] = ++m_commandid;
      runcmd["type"] = "assist_pipeline/run";
      if (includeWakeWordDetection) {
        runcmd["start_stage"] = "wake_word";
      } else 
      {
        runcmd["start_stage"] = "stt";
      }
      runcmd["end_stage"] = "tts";
      runcmd["input"]["sample_rate"] = 16000;

      char buffer[512];
      size_t buffersize = serializeJson(runcmd, buffer);

      Serial.printf("startPipeline() - Debug payload %s\n", buffer);

      m_webSocket->sendTXT(buffer);

      return true;
    } else 
    {
        Serial.printf("startPipeline() - Not authenticated, cannot start pipeline\n");
        return false;
    }
}

void Homeassistant::sendAudioData(int16_t *data, int length)
{
  if (m_state == RECORDING)
  {
    int transferlength = 1 + length * 2;

    uint8_t transferbuffer[transferlength];
    transferbuffer[0] = m_binaryHandler;

    int index = 1;
    for (int i=0; i < length; i++)     {
      int16_t value = data[i];
      int16_t *target = (int16_t *) &transferbuffer[index];
      *target = value;
      index += 2;
    }

    m_webSocket->sendBIN(transferbuffer, transferlength);
  }
}

void Homeassistant::loop()
{
    m_webSocket->loop();
}

Homeassistant::~Homeassistant()
{
    m_webSocket->disconnect();
    delete m_webSocket;
}

