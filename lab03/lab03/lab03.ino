// 定义LED引脚
const int ledPin = 2;

// 设置PWM属性
const int freq = 5000;      // 频率 5000Hz
const int resolution = 8;   // 分辨率 8位 (0-255)
const int channel = 0;      // 直接用数字0作为PWM通道

void setup() {
  Serial.begin(115200);
  // 初始化串口通信，设置波特率为115200
  
  // 第一步：先配置通道的频率和分辨率
  ledcSetup(channel, freq, resolution);
  
  // 第二步：把引脚绑定到这个通道
  ledcAttachPin(ledPin, channel);
}

void loop() {
  // 开始循环，控制LED呼吸灯效果
  // 逐渐变亮
  for(int dutyCycle = 0; dutyCycle <= 255; dutyCycle++){
    ledcWrite(channel, dutyCycle); // 必须写通道号，不是引脚号
    delay(10);
  }
  
  // 逐渐变暗
  for(int dutyCycle = 255; dutyCycle >= 0; dutyCycle--){
    ledcWrite(channel, dutyCycle);
    delay(10);
  }
  
  Serial.println("Breathing cycle completed");
  // Indicate the end of a cycle
}