const int ledPin = 2;
const int channel = 0;
const int resolution = 8;
const int freq = 5000;

// 语义化常量，提升可读性
const int DOT = 1;          // 短闪（点）
const int DASH = 3;         // 长闪（划）
const int shortDuration = 200;     // 点的持续时间
const int longDuration = 600;      // 划的持续时间
const int pauseBetweenFlashes = 200;  // 闪炼间隔
const int pauseAfterPattern = 1500;   // 图案后暂停

// SOS摩尔斯电码：···---···
int sosPattern[] = {DOT, DOT, DOT, DASH, DASH, DASH, DOT, DOT, DOT};
int patternIndex = 0;       // 当前图案索引
unsigned long previousMillis = 0;  // 计时基准
// 状态标记：true=亮灯阶段，false=间隔阶段
bool isLightOn = false;

void setup() {
  Serial.begin(115200);
  ledcSetup(channel, freq, resolution);
  ledcAttachPin(ledPin, channel);
}

void loop() {
  unsigned long currentMillis = millis();
  int patternLength = sizeof(sosPattern) / sizeof(sosPattern[0]);

  if (patternIndex < patternLength) {
    int currentSymbol = sosPattern[patternIndex];
    int targetDuration = (currentSymbol == DOT) ? shortDuration : longDuration;

    if (isLightOn) {
      // 亮灯阶段：时间到则关灯，切换到间隔阶段
      if (currentMillis - previousMillis >= targetDuration) {
        ledcWrite(channel, 0);          // 关灯
        previousMillis = currentMillis; // 重置计时
        isLightOn = false;              // 切换状态
      }
    } else {
      // 间隔阶段：时间到则切换到下一个图案，打开LED
      if (currentMillis - previousMillis >= pauseBetweenFlashes) {
        patternIndex++;                // 下一个图案位
        if (patternIndex < patternLength) {
          ledcWrite(channel, 255);     // 开灯
          previousMillis = currentMillis;
          isLightOn = true;            // 切换到亮灯状态
        }
      }
    }
    // 首次进入该图案位，初始化亮灯
    if (patternIndex < patternLength && previousMillis == 0) {
      ledcWrite(channel, 255);  // 打开LED
      previousMillis = currentMillis;
      isLightOn = true;
    }
  } else {
    // 图案执行完，暂停后重置
    if (currentMillis - previousMillis >= pauseAfterPattern) {
      patternIndex = 0;
      previousMillis = 0;  // 重置计时，让下次重新初始化亮灯
      isLightOn = false;
    }
  }
}