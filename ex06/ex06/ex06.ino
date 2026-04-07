// 定义LED引脚 → 已改为 D4 和 D16
const int ledPin = 4;   // 灯A = D4
const int ledPinB = 16; // 灯B = D16

// PWM配置
const int freq = 5000;
const int resolution = 8;

// 两个独立PWM通道
const int channelA = 0;
const int channelB = 1;

// 渐变效果（柔和不刺眼）
const int step = 2;
const int fadeDelay = 15;

void setup() {
  Serial.begin(115200);

  // 通道A初始化
  ledcSetup(channelA, freq, resolution);
  ledcAttachPin(ledPin, channelA);

  // 通道B初始化
  ledcSetup(channelB, freq, resolution);
  ledcAttachPin(ledPinB, channelB);
}

void loop() {
  // 灯A 亮 → 灯B 灭
  for(int dutyCycle = 0; dutyCycle <= 255; dutyCycle += step){
    ledcWrite(channelA, dutyCycle);          // A变亮
    ledcWrite(channelB, 255 - dutyCycle);    // B变暗
    delay(fadeDelay);
  }

  // 灯A 灭 → 灯B 亮
  for(int dutyCycle = 255; dutyCycle >= 0; dutyCycle -= step){
    ledcWrite(channelA, dutyCycle);          // A变暗
    ledcWrite(channelB, 255 - dutyCycle);    // B变亮
    delay(fadeDelay);
  }
}