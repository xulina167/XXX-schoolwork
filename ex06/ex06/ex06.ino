// 定义LED引脚
const int ledPin = 2;  // （灯A）
const int ledPinB = 4; // 新增：灯B引脚

// 设置PWM属性
const int freq = 5000;          // 频率 5000Hz
const int resolution = 8;       // 分辨率 8位 (0-255)

// 新增：定义两个独立PWM通道
const int channelA = 0;
const int channelB = 1;

// 新增：强化交闪的参数
const int step = 5;       // 渐变步长（替代原固定+1，让亮度变化更明显）
const int fadeDelay = 5;  // 渐变延时
const int peakDelay = 100;// 峰值停留（新增：亮/灭顶点停留，强化交替）

void setup() {
  Serial.begin(115200);

  // 标准双通道初始化（核心适配，不改变逻辑）
  // ledcAttach(ledPin, freq, resolution);
  ledcSetup(channelA, freq, resolution);  // 初始化灯A通道
  ledcAttachPin(ledPin, channelA);        // 绑定灯A引脚
  ledcSetup(channelB, freq, resolution);  // 新增：初始化灯B通道
  ledcAttachPin(ledPinB, channelB);       // 新增：绑定灯B引脚
}

void loop() {
  //单灯渐变 → 改为双灯反相渐变（核心改动）
  // 逐渐变亮（灯A）+ 逐渐变暗（灯B）
  for(int dutyCycle = 0; dutyCycle <= 255; dutyCycle += step){ // 改动：步长从1→step
    // ledcWrite(ledPin, dutyCycle);
    ledcWrite(channelA, dutyCycle);        // 保留：灯A渐变亮
    ledcWrite(channelB, 255 - dutyCycle);  // 新增：灯B反相渐变暗
    delay(fadeDelay);                      // 改动：延时从10→fadeDelay
  }
  delay(peakDelay); // 新增：灯A最亮、灯B全灭时停留

  // 逐渐变暗（灯A）+ 逐渐变亮（灯B）
  for(int dutyCycle = 255; dutyCycle >= 0; dutyCycle -= step){ // 改动：步长从1→step
    // ledcWrite(ledPin, dutyCycle);
    ledcWrite(channelA, dutyCycle);        // 保留：灯A渐变暗
    ledcWrite(channelB, 255 - dutyCycle);  // 新增：灯B反相渐变亮
    delay(fadeDelay);                      // 改动：延时从10→fadeDelay
  }
  delay(peakDelay); // 新增：灯A全灭、灯B最亮时停留
  
  Serial.println("Breathing cycle completed (enhanced police flash)");
}