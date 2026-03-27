#define TOUCH_PIN 4
#define LED_PIN 2
#define THRESHOLD 50 // 需要根据实际测试修改此阈值

bool ledState = false;
unsigned long lastTouchTime = 0;
const unsigned long debounceDelay = 200;
bool lastTouchDetected = false; // 新增：记录上一次触摸状态（边缘检测用）
// 中断服务函数 (ISR)
void gotTouch() {
  // 1. 原有防抖逻辑保留
  if (millis() - lastTouchTime > debounceDelay) {
    // 2. 新增：读取当前触摸状态（触摸值 < 阈值 = 触摸按下）
    bool currentTouchDetected = (touchRead(TOUCH_PIN) < THRESHOLD);
    
    // 3. 新增：边缘检测核心（仅「上一次未触摸→当前触摸」才翻转）
    if (currentTouchDetected && !lastTouchDetected) {
      ledState = !ledState;          // 翻转LED状态
      digitalWrite(LED_PIN, ledState); // 更新LED
      lastTouchTime = millis();      // 重置防抖时间戳
    }
    
    // 4. 新增：更新上一次触摸状态（关键！否则边缘检测失效）
    lastTouchDetected = currentTouchDetected;
  }
}
void setup() {
  Serial.begin(115200);
  delay(1000);
  pinMode(LED_PIN, OUTPUT);
  
  // 绑定中断函数
  touchAttachInterrupt(TOUCH_PIN, gotTouch, THRESHOLD);
}

void loop() {
  // 主循环可以处理其他任务，或者置空
  // 注意：在某些ESP32核心版本中，loop中可能需要保留简单的代码以保持看门狗复位
  int touchValue = touchRead(TOUCH_PIN);  // 读取当前触摸值
  Serial.print("Touch Value：");
  Serial.println(touchValue);
  delay(100);
  
}