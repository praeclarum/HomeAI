#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#include "Heater.h"
#include "Config.h"
#include "State.h"
#include "Server.h"

#include "Secrets.h"

enum EventType {
  TEMP_EVENT        =0,
  MANUAL_TEMP_EVENT =1,
  TARGET_TEMP_EVENT =2,
  HEATER_ON_EVENT   =3,
};

static QueueHandle_t postValueQueue = 0;

// static StateChangedEvent stateChanged(HEATER_TASK_ID);

static WiFiMulti WiFiMulti;

static bool connected = false;

struct PostValueData {
  EventType eventType;
  float value;
};

static bool apiPostValue(EventType eventType, float value);



// This is GandiStandardSSLCA2.pem, the root Certificate Authority that signed 
// the server certifcate for the demo server https://jigsaw.w3.org in this
// example. This certificate is valid until Sep 11 23:59:59 2024 GMT
static const char* rootCACertificate = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIF6TCCA9GgAwIBAgIQBeTcO5Q4qzuFl8umoZhQ4zANBgkqhkiG9w0BAQwFADCB\n" \
"iDELMAkGA1UEBhMCVVMxEzARBgNVBAgTCk5ldyBKZXJzZXkxFDASBgNVBAcTC0pl\n" \
"cnNleSBDaXR5MR4wHAYDVQQKExVUaGUgVVNFUlRSVVNUIE5ldHdvcmsxLjAsBgNV\n" \
"BAMTJVVTRVJUcnVzdCBSU0EgQ2VydGlmaWNhdGlvbiBBdXRob3JpdHkwHhcNMTQw\n" \
"OTEyMDAwMDAwWhcNMjQwOTExMjM1OTU5WjBfMQswCQYDVQQGEwJGUjEOMAwGA1UE\n" \
"CBMFUGFyaXMxDjAMBgNVBAcTBVBhcmlzMQ4wDAYDVQQKEwVHYW5kaTEgMB4GA1UE\n" \
"AxMXR2FuZGkgU3RhbmRhcmQgU1NMIENBIDIwggEiMA0GCSqGSIb3DQEBAQUAA4IB\n" \
"DwAwggEKAoIBAQCUBC2meZV0/9UAPPWu2JSxKXzAjwsLibmCg5duNyj1ohrP0pIL\n" \
"m6jTh5RzhBCf3DXLwi2SrCG5yzv8QMHBgyHwv/j2nPqcghDA0I5O5Q1MsJFckLSk\n" \
"QFEW2uSEEi0FXKEfFxkkUap66uEHG4aNAXLy59SDIzme4OFMH2sio7QQZrDtgpbX\n" \
"bmq08j+1QvzdirWrui0dOnWbMdw+naxb00ENbLAb9Tr1eeohovj0M1JLJC0epJmx\n" \
"bUi8uBL+cnB89/sCdfSN3tbawKAyGlLfOGsuRTg/PwSWAP2h9KK71RfWJ3wbWFmV\n" \
"XooS/ZyrgT5SKEhRhWvzkbKGPym1bgNi7tYFAgMBAAGjggF1MIIBcTAfBgNVHSME\n" \
"GDAWgBRTeb9aqitKz1SA4dibwJ3ysgNmyzAdBgNVHQ4EFgQUs5Cn2MmvTs1hPJ98\n" \
"rV1/Qf1pMOowDgYDVR0PAQH/BAQDAgGGMBIGA1UdEwEB/wQIMAYBAf8CAQAwHQYD\n" \
"VR0lBBYwFAYIKwYBBQUHAwEGCCsGAQUFBwMCMCIGA1UdIAQbMBkwDQYLKwYBBAGy\n" \
"MQECAhowCAYGZ4EMAQIBMFAGA1UdHwRJMEcwRaBDoEGGP2h0dHA6Ly9jcmwudXNl\n" \
"cnRydXN0LmNvbS9VU0VSVHJ1c3RSU0FDZXJ0aWZpY2F0aW9uQXV0aG9yaXR5LmNy\n" \
"bDB2BggrBgEFBQcBAQRqMGgwPwYIKwYBBQUHMAKGM2h0dHA6Ly9jcnQudXNlcnRy\n" \
"dXN0LmNvbS9VU0VSVHJ1c3RSU0FBZGRUcnVzdENBLmNydDAlBggrBgEFBQcwAYYZ\n" \
"aHR0cDovL29jc3AudXNlcnRydXN0LmNvbTANBgkqhkiG9w0BAQwFAAOCAgEAWGf9\n" \
"crJq13xhlhl+2UNG0SZ9yFP6ZrBrLafTqlb3OojQO3LJUP33WbKqaPWMcwO7lWUX\n" \
"zi8c3ZgTopHJ7qFAbjyY1lzzsiI8Le4bpOHeICQW8owRc5E69vrOJAKHypPstLbI\n" \
"FhfFcvwnQPYT/pOmnVHvPCvYd1ebjGU6NSU2t7WKY28HJ5OxYI2A25bUeo8tqxyI\n" \
"yW5+1mUfr13KFj8oRtygNeX56eXVlogMT8a3d2dIhCe2H7Bo26y/d7CQuKLJHDJd\n" \
"ArolQ4FCR7vY4Y8MDEZf7kYzawMUgtN+zY+vkNaOJH1AQrRqahfGlZfh8jjNp+20\n" \
"J0CT33KpuMZmYzc4ZCIwojvxuch7yPspOqsactIGEk72gtQjbz7Dk+XYtsDe3CMW\n" \
"1hMwt6CaDixVBgBwAc/qOR2A24j3pSC4W/0xJmmPLQphgzpHphNULB7j7UTKvGof\n" \
"KA5R2d4On3XNDgOVyvnFqSot/kGkoUeuDcL5OWYzSlvhhChZbH2UF3bkRYKtcCD9\n" \
"0m9jqNf6oDP6N8v3smWe2lBvP+Sn845dWDKXcCMu5/3EFZucJ48y7RetWIExKREa\n" \
"m9T8bJUox04FB6b9HbwZ4ui3uRGKLXASUoWNjDNKD/yZkuBjcNqllEdjB+dYxzFf\n" \
"BT02Vf6Dsuimrdfp5gJ0iHRc2jTbkNJtUQoj1iM=\n" \
"-----END CERTIFICATE-----\n";

// Not sure if WiFiClientSecure checks the validity date of the certificate. 
// Setting clock just to be sure...
static void setClock() {
  configTime(0, 0, "pool.ntp.org");
  
  time_t nowSecs = time(nullptr);
  while (nowSecs < 8 * 3600 * 2) {
    Serial.println(F("Waiting for NTP time sync..."));
    vTaskDelay(1000 / portTICK_RATE_MS);
    yield();
    nowSecs = time(nullptr);
  }

  struct tm timeinfo;
  gmtime_r(&nowSecs, &timeinfo);
  Serial.print(F("Current time: "));
  Serial.println(asctime(&timeinfo));
}

static void serverSetup() {

  postValueQueue = xQueueCreate(16, sizeof(PostValueData));

  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(WIFI_NAME, WIFI_PASSWORD);

  // Wait for WiFi connection
  while ((WiFiMulti.run() != WL_CONNECTED)) {
    Serial.println("Waiting for WiFi to connect...");
    vTaskDelay(1000 / portTICK_RATE_MS);
  }
  Serial.println("WiFi connected!");

  setClock();
  connected = true;
}

static bool apiGetDevices()
{
  if (!connected) return false;
  WiFiClientSecure client;//new BearSSL::WiFiClientSecure);
  client.setInsecure();
  HTTPClient https;  
  String url = "https://housebot.azurewebsites.net/api/devices";
  Serial.printf("[HTTPS] GET %s\n", url.c_str()); 
  if (https.begin(client, url)) {  // HTTPS
//    https.addHeader("accept", "application/json");
    int httpCode = https.GET();
    if (httpCode > 0) {
      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
        String resp = https.getString();
        Serial.println(resp);
        return true;
      }
    } else {
      Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
    }
    https.end();
  } else {
    Serial.printf("[HTTPS] Unable to connect\n");
  }
  return false;
}

static bool apiGetTargetTemperature(float *targetCelsius)
{
  if (!connected) return false;
  DynamicJsonDocument doc(1024);
  *targetCelsius = 0;
  bool result = false;
  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient https;  
  String url = "https://housebot.azurewebsites.net/api/thermostatesetpoint/" + String(DEVICE_ID);
  Serial.printf("[HTTPS] GET %s\n", url.c_str()); 
  if (https.begin(client, url)) {  // HTTPS
//    https.addHeader("accept", "application/json");
    int httpCode = https.GET();
    if (httpCode > 0) {
      if (httpCode == HTTP_CODE_OK) {
        String json = https.getString();
        Serial.println(json);
        
        DeserializationError error = deserializeJson(doc, json);
        if (!error) {
          *targetCelsius = doc["setpoint"];
          Serial.print("SETPOINT: ");
          Serial.println(*targetCelsius);
          result = *targetCelsius > 0;
        }
        else {
          Serial.print(F("apiGetTargetTemperature deserializeJson() failed: "));
          Serial.println(error.c_str());
        }
      }
    } else {
      Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
    }
    https.end();
  } else {
    Serial.printf("[HTTPS] Unable to connect\n");
  }
  return result;
}

static bool apiPostValue(EventType eventType, float value)
{
  if (!connected) return false;
  WiFiClientSecure client;//new BearSSL::WiFiClientSecure);
  client.setInsecure();
  HTTPClient https;  
  String url = "https://housebot.azurewebsites.net/api/events";
  Serial.printf("POST EVENT #%d VALUE %g\n", eventType, value); 
  if (https.begin(client, url)) {  // HTTPS
    https.addHeader("accept", "application/json");
    https.addHeader("content-type", "application/json");
    struct tm timeinfo;
    time_t nowSecs = time(nullptr);
    gmtime_r(&nowSecs, &timeinfo);
    // Get an RFC formatted time string
    char buffer[256];
    strftime(buffer, 256, "%FT%TZ", &timeinfo);
    String timestamp = String(buffer);

    String req =
      "{\"deviceId\":\"" + String(DEVICE_ID) + "\","
      "\"key\":\"" + String(HUB_WRITE_KEY) + "\","
      "\"timestamp\":\"" + timestamp + "\","
      "\"eventType\":"+String(eventType)+","
      "\"value\":\"" + String(value, 4) + "\""
      "}";
    // Serial.println(req);
    https.setTimeout(30000);
    int httpCode = https.POST(req);
    // Serial.println("STATUS CODE: " + String(httpCode));
    if (httpCode > 0) {
      if (httpCode == HTTP_CODE_OK) {
        String resp = https.getString();
        // Serial.println(resp);
        return true;
      }
    } else {
      Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
    }
    https.end();
  } else {
    Serial.printf("[HTTPS] Unable to connect\n");
  }
  return false;
}

static bool apiPostValueAsync(EventType eventType, float value)
{
  Serial.printf("QUEUE EVENT #%d VALUE %g\n", eventType, value); 
  PostValueData data;
  data.eventType = eventType;
  data.value = value;
  return xQueueSend(postValueQueue, &data, 100 / portTICK_RATE_MS) == pdTRUE;
}

bool serverPostTemperatureAsync(float celsius)
{
  return apiPostValueAsync(TEMP_EVENT, celsius);
}

bool serverPostManualTemperatureAsync(float celsius)
{
  return apiPostValueAsync(MANUAL_TEMP_EVENT, celsius);
}

bool serverPostTargetTemperatureAsync(float celsius)
{
  return apiPostValueAsync(TARGET_TEMP_EVENT, celsius);
}

bool serverPostHeaterOnAsync(bool on)
{
  return apiPostValueAsync(HEATER_ON_EVENT, on ? 1.0f : 0.0f);
}

static unsigned long lastReadMillis = 0;
static float lastCelsius = 0;

static void serverLoop()
{
  const auto nowMillis = millis();

  const bool shouldRead = lastReadMillis == 0 || ((nowMillis - lastReadMillis) > 5 * 60 * 1000);

  const State state = readState();
  lastCelsius = state.thermometerCelsius;

  if (lastCelsius > 1 && shouldRead) {
    Serial.print("READ ");
    Serial.print(lastCelsius);
    Serial.print("C ");
    Serial.print(lastCelsius * 9.0f / 5.0f + 32.0f);
    Serial.println("F");

    bool readSucceeded = false;
    if (apiPostValue(TEMP_EVENT, lastCelsius)) {
      vTaskDelay(1000 / portTICK_RATE_MS);
      float targetCelsius = 0.0;
      if (apiGetTargetTemperature(&targetCelsius)) {
        updateState(SERVER_TASK_ID, [targetCelsius](State &x) {
          x.targetCelsius = targetCelsius;
        });
        Serial.print("TARGET ");
        Serial.print(targetCelsius);
        Serial.println("C ");
        vTaskDelay(1000 / portTICK_RATE_MS);
        if (apiPostValue(TARGET_TEMP_EVENT, targetCelsius)) {
          readSucceeded = true;
        }
      }
    }

    lastReadMillis = millis();

    if (!readSucceeded) {
      updateState(SERVER_TASK_ID, [](State &x) {
        x.networkError = true;
      });
      Serial.println("NETWORK ERROR, RESTARTING AFTER A MINUTE...");
      digitalWrite(HEATER_PIN, LOW);
      // displayError();
      vTaskDelay(60000 / portTICK_PERIOD_MS);
      ESP.restart();
    }
  }

  PostValueData postValueData;
  while (xQueueReceive(postValueQueue, &postValueData, 2000 / portTICK_RATE_MS)) {
    apiPostValue(postValueData.eventType, postValueData.value);    
  }

  // if (stateChanged.wait(7000)) {
  //   //Serial.println("SERVER SAW STATE CHANGE");
  // }
}

static void serverTask(void *arg) {
  serverSetup();
  for (;;) {
    serverLoop();
  }  
}

void serverStart() {
  // subscribeToStateChanges(&stateChanged);
  xTaskCreatePinnedToCore(
    serverTask
    ,  "Server"
    ,  16*1024  // Stack size
    ,  nullptr // Arg
    ,  TASK_PRIORITY  // Priority
    ,  NULL 
    ,  ARDUINO_RUNNING_CORE);
}
