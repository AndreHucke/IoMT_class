#include <BluetoothSerial.h>

BluetoothSerial SerialBT;

#define BT_DISCOVER_TIME 10000

static bool btScanAsync = true;
static bool btScanSync = true;

const int PULSE_SENSOR_PIN = 36;
const int SAMPLE_INTERVAL = 10;

// Signal processing variables
const int SAMPLE_SIZE = 100;
int readings[SAMPLE_SIZE];
int readIndex = 0;
int Signal = 0;

// Pulse detection variables
unsigned long lastBeatTime = 0;
int BPM = 0;
int threshold = 0;
int peakValue = 0;
int valleyValue = 0;

void btAdvertisedDeviceFound(BTAdvertisedDevice *pDevice) {
  Serial.printf("Found a device asynchronously: %s\n", pDevice->toString().c_str());
}

void setup() {
  Serial.begin(115200);
  SerialBT.begin("Pulse_device");  //Bluetooth device name
  Serial.println("The device started, now you can pair it with bluetooth!");
  SerialBT.println("The device started, now you can pair it with bluetooth!");

  if (btScanAsync) {
    Serial.print("Starting asynchronous discovery... ");
    if (SerialBT.discoverAsync(btAdvertisedDeviceFound)) {
      Serial.println("Findings will be reported in \"btAdvertisedDeviceFound\"");
      delay(10000);
      Serial.print("Stopping discoverAsync... ");
      SerialBT.discoverAsyncStop();
      Serial.println("stopped");
    } else {
      Serial.println("Error on discoverAsync f.e. not working after a \"connect\"");
    }

  memset(readings, 0, sizeof(readings));
  }

  if (btScanSync) {
    Serial.println("Starting synchronous discovery... ");
    BTScanResults *pResults = SerialBT.discover(BT_DISCOVER_TIME);
    if (pResults) {
      pResults->dump(&Serial);
    } else {
      Serial.println("Error on BT Scan, no result!");
    }
  }
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

        SerialBT.print("Signal:");
        SerialBT.print(Signal);
        SerialBT.print(",BPM:");
        SerialBT.println(BPM);
      }
    }
    
    lastBeatTime = now;
    // Reset peak/valley for next beat
    peakValue = Signal;
    valleyValue = Signal;
  }

  // Check for lost signal
  if (millis() - lastBeatTime > 2500) {
    Serial.println("Adquiring signal...");
    SerialBT.println("Adquiring signal...");
    BPM = 0;
    peakValue = 0;
    valleyValue = 1023;
  }

  delay(SAMPLE_INTERVAL);
}
