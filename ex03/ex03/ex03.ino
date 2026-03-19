const int ledPin = 2;
const int channel = 0;
const int resolution = 8;
const int freq = 5000;
unsigned long previousMillis = 0;
const long interval = 1000; 
int flashState = 0;
int flashIndex = 0;
const int shortFlash = 1;
const int longFlash = 3;
const int shortDuration = 100; 
const int longDuration = 500; 
const int pauseDuration = 1000; 
const int sosPattern[] = {shortFlash, shortFlash, shortFlash, longFlash, longFlash, longFlash, shortFlash, shortFlash, shortFlash};

void setup() {
  Serial.begin(115200);
  ledcSetup(channel, freq, resolution);
  ledcAttachPin(ledPin, channel);
}

void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    if (flashIndex < sizeof(sosPattern) / sizeof(sosPattern[0])) {
      if (flashState == 0) {
        if (sosPattern[flashIndex] == shortFlash) {
          ledcWrite(channel, 255);
          previousMillis += shortDuration;
        } else {
          ledcWrite(channel, 255);
          previousMillis += longDuration;
        }
        flashState = 1;
      } else {
        ledcWrite(channel, 0);
        if (flashIndex == sizeof(sosPattern) / sizeof(sosPattern[0]) - 1) {
          previousMillis += pauseDuration;
        } else {
          previousMillis += interval;
        }
        flashIndex++;
        flashState = 0;
      }
    } else {
      flashIndex = 0;
    }
  }
}