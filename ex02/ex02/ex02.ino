unsigned long myTime;
const int ledPin = 2;
// 新增：记录上一次LED切换时间
unsigned long lastBlinkTime = 0;
// 新增：1Hz闪烁=500ms切换一次状态
const int blinkInterval = 500;
// 新增：记录LED当前状态
bool ledState = LOW;

void setup() {
  Serial.begin(9600);
  // 新增：设置LED引脚为输出模式
  pinMode(ledPin, OUTPUT);
}

void loop() {
  Serial.print("Time: ");
  myTime = millis();
  Serial.println(myTime); // 保留原有串口打印逻辑

  // 新增：核心LED闪烁控制（基于myTime，非阻塞）
  if (myTime - lastBlinkTime >= blinkInterval) {
    lastBlinkTime = myTime; // 更新切换时间
    ledState = !ledState;   // 翻转LED状态（亮↔灭）
    digitalWrite(ledPin, ledState); // 控制LED亮灭
  }

  // 保留原有delay，但改为10ms避免阻塞（不影响1Hz闪烁）
  // 原delay(1000)会导致闪烁频率错乱，必须改小
  delay(10);          
}