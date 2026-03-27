// 触摸引脚定义
#define TOUCH_PIN 4
#define LED_PIN 2
#define THRESHOLD 50 // 按实际硬件微调

// PWM基础配置
const int freq = 5000;          
const int resolution = 8;       
const int ledcChannel = 0;     

// 触摸防抖+边缘检测
unsigned long lastTouchTime = 0;
const unsigned long debounceDelay = 200;
bool lastTouchDetected = false;

// 【强化三档区分】拉大档位延时差，确保效果明显
int speedLevel = 1;                      // 初始档位1（最慢）
// 3档延时：50ms(极慢)、20ms(中等)、5ms(极快)，差异足够大
int delayTime[3] = {50, 20, 5};          
// 可选：也可通过步长调速（效果更细腻）
int stepSize[3] = {1, 3, 5};             // 占空比递增步长（1=慢，3=中，5=快）

// ESP32中断函数必需加IRAM_ATTR
void IRAM_ATTR gotTouch() { 
  unsigned long currentTime = millis();
  if (currentTime - lastTouchTime > debounceDelay) {
    bool currentTouchDetected = (touchRead(TOUCH_PIN) < THRESHOLD);
    // 边缘检测：仅触摸按下瞬间切换档位
    if (currentTouchDetected && !lastTouchDetected) {
      speedLevel = (speedLevel % 3) + 1; // 严格循环：1→2→3→1
      lastTouchTime = currentTime;
    }
    lastTouchDetected = currentTouchDetected;
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  // ESP32 PWM初始化（标准流程）
  ledcSetup(ledcChannel, freq, resolution);
  ledcAttachPin(LED_PIN, ledcChannel);
  
  // 绑定触摸中断
  touchAttachInterrupt(TOUCH_PIN, gotTouch, THRESHOLD);
  
  // 上电提示：打印初始档位
  Serial.print("初始档位：");
  Serial.println(speedLevel);
}

void loop() {
  // 获取当前档位对应的延时和步长
  int currentDelay = delayTime[speedLevel - 1];
  int currentStep = stepSize[speedLevel - 1];

  // 呼吸灯-逐渐变亮（按步长递增，强化档位差异）
  for(int dutyCycle = 0; dutyCycle <= 255; dutyCycle += currentStep){   
    ledcWrite(ledcChannel, dutyCycle);   
    delay(currentDelay);
  }

  // 呼吸灯-逐渐变暗（按步长递减）
  for(int dutyCycle = 255; dutyCycle >= 0; dutyCycle -= currentStep){
    ledcWrite(ledcChannel, dutyCycle);   
    delay(currentDelay);
  }
  
  // 串口打印：明确当前档位+参数，方便调试
  Serial.print("当前档位：");
  Serial.print(speedLevel);
  Serial.print(" | 延时：");
  Serial.print(currentDelay);
  Serial.print("ms | 步长：");
  Serial.println(currentStep);
  
  // 保留触摸值打印，方便校准阈值
  Serial.print("Touch Value：");
  Serial.println(touchRead(TOUCH_PIN));
  
  delay(100);
}