#if defined(ESP8266)
#include <ESP8266WiFi.h>
#define THINGSBOARD_ENABLE_PROGMEM 0
#elif defined(ESP32) || defined(RASPBERRYPI_PICO) || defined(RASPBERRYPI_PICO_W)
#include <WiFi.h>
#endif

#include <Arduino_MQTT_Client.h>
#include <Server_Side_RPC.h>
#include <Attribute_Request.h>
#include <Shared_Attribute_Update.h>
#include <ThingsBoard.h>
#include <ArduinoJson.h>

constexpr char WIFI_SSID[] = "WIFI_NAME";
constexpr char WIFI_PASSWORD[] = "WIFI_PASSWORD";

// See https://thingsboard.io/docs/getting-started-guides/helloworld/
// to understand how to obtain an access token
constexpr char TOKEN[] = "ACCESS_TOKEN";

// Thingsboard we want to establish a connection too
constexpr char THINGSBOARD_SERVER[] = "SERVER_WEBSITE";
// MQTT port used to communicate with the server, 1883 is the default unencrypted MQTT port.
constexpr uint16_t THINGSBOARD_PORT = 1883U;

// Maximum size packets will ever be sent or received by the underlying MQTT client,
// if the size is to small messages might not be sent or received messages will be discarded
constexpr uint32_t MAX_MESSAGE_SIZE = 1024U;

// Baud rate for the debugging serial connection.
// If the Serial output is mangled, ensure to change the monitor speed accordingly to this variable
constexpr uint32_t SERIAL_DEBUG_BAUD = 115200U;

// Maximum amount of attributs we can request or subscribe, has to be set both in the ThingsBoard template list and Attribute_Request_Callback template list
// and should be the same as the amount of variables in the passed array. If it is less not all variables will be requested or subscribed
constexpr size_t MAX_ATTRIBUTES = 3U;

constexpr uint64_t REQUEST_TIMEOUT_MICROSECONDS = 5000U * 1000U;

// Attribute names for attribute request and attribute updates functionality
constexpr const char SAMPLING_INTERVAL_ATTR[] = "samplingInterval";
constexpr const char PUBLISH_INTERVAL_ATTR[] = "publishInterval";
constexpr const char BPM_CHANGE_THRESHOLD_ATTR[] = "bpmChangeThreshold";

// Initialize underlying client, used to establish a connection
WiFiClient wifiClient;

// Initalize the Mqtt client instance
Arduino_MQTT_Client mqttClient(wifiClient);

// Initialize used apis
Server_Side_RPC<3U, 5U> rpc;
Attribute_Request<2U, MAX_ATTRIBUTES> attr_request;
Shared_Attribute_Update<3U, MAX_ATTRIBUTES> shared_update;

const std::array<IAPI_Implementation*, 3U> apis = {
    &rpc,
    &attr_request,
    &shared_update
};

// Initialize ThingsBoard instance with the maximum needed buffer size, stack size and the apis we want to use
ThingsBoard tb(mqttClient, MAX_MESSAGE_SIZE, Default_Max_Stack_Size, apis);

// Pulse sensor config
const int PULSE_SENSOR_PIN = 36;
const int SAMPLE_SIZE = 10;
int readings[SAMPLE_SIZE];
int readIndex = 0;
int Signal = 0;

// Pulse detection variables
unsigned long lastBeatTime = 0;
int BPM = 0;
int threshold = 0;
int peakValue = 0;
int valleyValue = 1023;

// Settings for pulse monitoring and publishing
volatile int SAMPLE_INTERVAL = 10;
volatile int PUBLISH_INTERVAL = 2000;
volatile int BPM_CHANGE_THRESHOLD = 5;
unsigned long lastPublishTime = 0;
int lastPublishedBPM = 0;
volatile bool attributesChanged = false;

// For telemetry
uint32_t previousDataSend;

// List of shared attributes for subscribing to their updates
constexpr std::array<const char *, 3U> SHARED_ATTRIBUTES_LIST = {
  SAMPLING_INTERVAL_ATTR,
  PUBLISH_INTERVAL_ATTR,
  BPM_CHANGE_THRESHOLD_ATTR
};

// List of client attributes for requesting them (empty for now)
constexpr std::array<const char *, 0U> CLIENT_ATTRIBUTES_LIST = {};

/// @brief Initalizes WiFi connection,
// will endlessly delay until a connection has been successfully established
void InitWiFi() {
  Serial.println("Connecting to AP ...");
  // Attempting to establish a connection to the given WiFi network
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    // Delay 500ms until a connection has been succesfully established
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to AP");
}

/// @brief Reconnects the WiFi uses InitWiFi if the connection has been removed
/// @return Returns true as soon as a connection has been established again
const bool reconnect() {
  // Check to ensure we aren't connected yet
  const wl_status_t status = WiFi.status();
  if (status == WL_CONNECTED) {
    return true;
  }

  // If we aren't establish a new connection to the given WiFi network
  InitWiFi();
  return true;
}

/// @brief Processes function for RPC call "setSamplingInterval"
void processSamplingInterval(const JsonVariantConst &data, JsonDocument &response) {
  Serial.println("Received the set sampling interval RPC method");

  // Process data
  int new_interval = data;

  Serial.print("New sampling interval: ");
  Serial.println(new_interval);
  StaticJsonDocument<1> response_doc;

  if (new_interval < 5 || new_interval > 100) {
    response_doc["error"] = "Invalid interval! Must be between 5-100ms";
    response.set(response_doc);
    return;
  }

  SAMPLE_INTERVAL = new_interval;
  attributesChanged = true;

  // Returning current value
  response_doc["newInterval"] = SAMPLE_INTERVAL;
  response.set(response_doc);
}

// RPC callbacks
const std::array<RPC_Callback, 1U> callbacks = {
  RPC_Callback{ "setSamplingInterval", processSamplingInterval }
};

/// @brief Update callback that will be called as soon as one of the provided shared attributes changes value
void processSharedAttributes(const JsonObjectConst &data) {
  for (auto it = data.begin(); it != data.end(); ++it) {
    if (strcmp(it->key().c_str(), SAMPLING_INTERVAL_ATTR) == 0) {
      const int16_t new_interval = it->value().as<int16_t>();
      if (new_interval >= 5 && new_interval <= 100) {
        SAMPLE_INTERVAL = new_interval;
        Serial.print("Sampling interval is set to: ");
        Serial.println(new_interval);
      }
    } else if (strcmp(it->key().c_str(), PUBLISH_INTERVAL_ATTR) == 0) {
      const int16_t new_interval = it->value().as<int16_t>();
      if (new_interval >= 500 && new_interval <= 10000) {
        PUBLISH_INTERVAL = new_interval;
        Serial.print("Publish interval is set to: ");
        Serial.println(new_interval);
      }
    } else if (strcmp(it->key().c_str(), BPM_CHANGE_THRESHOLD_ATTR) == 0) {
      const int16_t new_threshold = it->value().as<int16_t>();
      if (new_threshold >= 1 && new_threshold <= 20) {
        BPM_CHANGE_THRESHOLD = new_threshold;
        Serial.print("BPM change threshold is set to: ");
        Serial.println(new_threshold);
      }
    }
  }
  attributesChanged = true;
}

void processClientAttributes(const JsonObjectConst &data) {
  // No client attributes to process for now
}

// Attribute request did not receive a response in the expected amount of microseconds 
void requestTimedOut() {
  Serial.printf("Attribute request timed out did not receive a response in (%llu) microseconds. Ensure client is connected to the MQTT broker and that the keys actually exist on the target device\n", REQUEST_TIMEOUT_MICROSECONDS);
}

const Shared_Attribute_Callback<MAX_ATTRIBUTES> attributes_callback(&processSharedAttributes, SHARED_ATTRIBUTES_LIST.cbegin(), SHARED_ATTRIBUTES_LIST.cend());
const Attribute_Request_Callback<MAX_ATTRIBUTES> attribute_shared_request_callback(&processSharedAttributes, REQUEST_TIMEOUT_MICROSECONDS, &requestTimedOut, SHARED_ATTRIBUTES_LIST);
const Attribute_Request_Callback<MAX_ATTRIBUTES> attribute_client_request_callback(&processClientAttributes, REQUEST_TIMEOUT_MICROSECONDS, &requestTimedOut, CLIENT_ATTRIBUTES_LIST);

void setup() {
  // Initialize serial connection for debugging
  Serial.begin(SERIAL_DEBUG_BAUD);
  delay(1000);
  InitWiFi();
  
  // Initialize pulse sensor
  memset(readings, 0, sizeof(readings));
}

void loop() {
  delay(SAMPLE_INTERVAL); // Use our configurable sampling interval

  if (!reconnect()) {
    return;
  }

  if (!tb.connected()) {
    // Connect to the ThingsBoard
    Serial.print("Connecting to: ");
    Serial.print(THINGSBOARD_SERVER);
    Serial.print(" with token ");
    Serial.println(TOKEN);
    if (!tb.connect(THINGSBOARD_SERVER, TOKEN, THINGSBOARD_PORT)) {
      Serial.println("Failed to connect");
      return;
    }
    
    Serial.println("Connected to ThingsBoard");
    
  if (!rpc.RPC_Subscribe(callbacks.cbegin(), callbacks.cend())) {
    Serial.println("Failed to subscribe for RPC");
    return;
  }

    if (!shared_update.Shared_Attributes_Subscribe(attributes_callback)) {
      Serial.println("Failed to subscribe for shared attribute updates");
      return;
    }
    
    // Create request for shared attributes
    // Use the pre-defined arrays directly without add_attribute
    if (!attr_request.Shared_Attributes_Request(attribute_shared_request_callback)) {
      Serial.println("Failed to request shared attributes");
      return;
    }
  }

  // Process any pending messages
  tb.loop();

  // Get current pulse reading with moving average
  readings[readIndex] = analogRead(PULSE_SENSOR_PIN);
  Signal = 0;
  for (int i = 0; i < SAMPLE_SIZE; i++) {
    Signal += readings[i];
    // Send Signal to ThingsBoard
    tb.sendTelemetryData("signal", readings[i]);
    Serial.print("Published Signal: ");
    Serial.println(Raw data);
  }
  Signal /= SAMPLE_SIZE;
  readIndex = (readIndex + 1) % SAMPLE_SIZE;

  // Update peak and valley values
  peakValue = max(peakValue, Signal);
  valleyValue = min(valleyValue, Signal);
  
  threshold = valleyValue + (peakValue - valleyValue) * 0.6;

  if (Signal > threshold) {
    unsigned long now = millis();
    int beatDuration = now - lastBeatTime;
    
    if (beatDuration > 300 && beatDuration < 1500) {
      BPM = 60000 / beatDuration;
      if (BPM >= 40 && BPM <= 200) {
        // Only publish if enough time has passed AND there's significant BPM change
        if ((millis() - lastPublishTime >= PUBLISH_INTERVAL) && 
            (abs(BPM - lastPublishedBPM) >= BPM_CHANGE_THRESHOLD)) {
          
          // Send BPM to ThingsBoard
          tb.sendTelemetryData("bpm", BPM);
          Serial.print("Published BPM: ");
          Serial.println(BPM);
          
          lastPublishTime = millis();
          lastPublishedBPM = BPM;
        }
      }
    }
    
    lastBeatTime = now;
    peakValue = Signal;
    valleyValue = Signal;
  }

  if (millis() - lastBeatTime > 2500) {
    // Only publish no-pulse status if we haven't published recently
    if (millis() - lastPublishTime >= PUBLISH_INTERVAL) {
      tb.sendTelemetryData("status", "No pulse");
      lastPublishTime = millis();
      lastPublishedBPM = 0;
    }
    
    BPM = 0;
    peakValue = 0;
    valleyValue = 1023;
  }
}