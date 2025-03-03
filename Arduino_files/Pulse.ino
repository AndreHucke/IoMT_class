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

void setup() {
  Serial.begin(115200);
  memset(readings, 0, sizeof(readings));
}

void loop() {
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
  
  // Dynamic threshold calculation
  threshold = valleyValue + (peakValue - valleyValue) * 0.6;

  // Detect beats
  if (Signal > threshold) {
    unsigned long now = millis();
    int beatDuration = now - lastBeatTime;
    
    if (beatDuration > 300 && beatDuration < 1500) {
      BPM = 60000 / beatDuration;
      if (BPM >= 40 && BPM <= 200) {
        Serial.print("Signal:");
        Serial.print(Signal);
        Serial.print(",BPM:");
        Serial.println(BPM);
      }
    }
    
    lastBeatTime = now;
    // Reset peak/valley for next beat
    peakValue = Signal;
    valleyValue = Signal;
  }

  // Check for lost signal
  if (millis() - lastBeatTime > 2500) {
    Serial.println("No pulse detected");
    BPM = 0;
    peakValue = 0;
    valleyValue = 1023;
  }

  delay(SAMPLE_INTERVAL);
}
