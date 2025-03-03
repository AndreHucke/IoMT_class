#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// WiFi credentials
const char* ssid = "WIFI_NAME";
const char* password = "WIFI_PASSWORD";

// MQTT Broker settings
const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;  // Non-SSL port
const char* mqtt_client_id = "CLIENTID";  // Make this unique
const char* mqtt_topic = "TOPIC";

// Pin and sampling configuration
const int PULSE_SENSOR_PIN = 36;
const int SAMPLE_INTERVAL = 10;

// Signal processing variables
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

// Add these variables after other global declarations
const unsigned long PUBLISH_INTERVAL = 2000;  // Publish every 2 seconds
const int BPM_CHANGE_THRESHOLD = 5;  // Minimum BPM change to trigger publication
unsigned long lastPublishTime = 0;
int lastPublishedBPM = 0;

// MQTT objects
WiFiClient espClient;
PubSubClient client(espClient);
StaticJsonDocument<200> doc;

void setup_wifi() {
  delay(10);
  Serial.println("Connecting to WiFi");
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
}

void reconnect() {
  while (!client.connected()) {
    Serial.println("Attempting MQTT connection...");
    if (client.connect(mqtt_client_id)) {
      Serial.println("Connected to MQTT broker");
    } else {
      Serial.print("Failed to connect. MQTT state: ");
      Serial.println(client.state());
      if (client.state() == -2) {
        Serial.println("Error: Connection timed out. Broker may be down or unreachable.");
      } else if (client.state() == -1) {
        Serial.println("Error: MQTT connection failed (Bad Credentials?).");
      } else {
        Serial.print("Error Code: ");
        Serial.println(client.state());
      }
      Serial.println("Retrying in 5 seconds...");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  memset(readings, 0, sizeof(readings));
  
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
}

void loop() {
  // Ensure the WiFi connection is still active
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi disconnected! Reconnecting...");
    setup_wifi();
  }

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Get current reading with moving average
  readings[readIndex] = analogRead(PULSE_SENSOR_PIN);
  Signal = 0;
  for (int i = 0; i < SAMPLE_SIZE; i++) {
    Signal += readings[i];
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
          
          doc.clear();
          doc["bpm"] = BPM;  // Remove signal value to reduce payload size
          
          char jsonBuffer[64];  // Reduced buffer size
          serializeJson(doc, jsonBuffer);
          
          client.publish(mqtt_topic, jsonBuffer);
          Serial.print("Published: ");
          Serial.println(jsonBuffer);
          
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
      doc.clear();
      doc["status"] = "No pulse";  // Shortened status message
      
      char jsonBuffer[64];  // Reduced buffer size
      serializeJson(doc, jsonBuffer);
      client.publish(mqtt_topic, jsonBuffer);
      
      lastPublishTime = millis();
      lastPublishedBPM = 0;
    }
    
    BPM = 0;
    peakValue = 0;
    valleyValue = 1023;
  }

  delay(SAMPLE_INTERVAL);
}
