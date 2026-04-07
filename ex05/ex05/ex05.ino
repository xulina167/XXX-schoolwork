// 触摸三档呼吸灯：触摸即时切换档位，无需等待呼吸周期
const int touchPin = T0;    // 触摸引脚（T0 = GPIO4）
const int ledPin = 2;       // ESP32板载LED（PWM引脚，可替换为其他PWM引脚）

// 核心状态变量
int breathGear = 1;         // 呼吸档位：1=慢，2=中，3=快
bool lastTouchState = false;// 上一次触摸状态（防抖+边缘检测）
bool currentTouchState = false;

// 防抖配置
const unsigned long debounceDelay = 300; // 防抖延时（300ms，避免误触）
unsigned long lastTouchTime = 0;         // 最后一次触摸触发时间

// 呼吸参数（三档配置，可微调）
const int slowPeriod = 3000;    // 慢呼吸周期（ms）
const int midPeriod = 1500;     // 中呼吸周期（ms）
const int fastPeriod = 500;     // 快呼吸周期（ms）
int currentPeriod = slowPeriod; // 当前呼吸周期（默认慢）

// 触摸阈值（灵敏度）
const int touchThreshold = 30;

// PWM相关（ESP32 PWM范围0-255）
const int pwmChannel = 0;       // PWM通道
const int pwmFreq = 5000;       // PWM频率
const int pwmResolution = 8;    // PWM分辨率（8位=0-255）

void setup() {
  Serial.begin(115200);
  
  // 初始化PWM（呼吸灯核心）
  ledcSetup(pwmChannel, pwmFreq, pwmResolution);
  ledcAttachPin(ledPin, pwmChannel);
  
  Serial.println("触摸三档呼吸灯就绪！");
  Serial.println("触摸GPIO4：1次=慢呼吸 | 2次=中呼吸 | 3次=快呼吸 | 4次=复位");
}

void loop() {
  // 1. 触摸检测+防抖+档位切换（核心：即时切换）
  checkTouchAndSwitchGear();
  
  // 2. 呼吸灯驱动（根据当前档位即时调整速度）
  runBreathLight();
}

// ==================== 触摸检测+档位切换函数（即时生效）====================
void checkTouchAndSwitchGear() {
  // 读取当前触摸状态
  int touchValue = touchRead(touchPin);
  currentTouchState = (touchValue < touchThreshold);
  
  // 防抖：仅间隔超过防抖延时才处理
  if (millis() - lastTouchTime > debounceDelay) {
    // 边缘检测：仅“上一次未触摸→当前触摸”的瞬间触发
    if (!lastTouchState && currentTouchState) {
      // 切换档位（循环：1→2→3→1）
      breathGear++;
      if (breathGear > 3) {
        breathGear = 1;
      }
      
      // 即时更新呼吸周期（无需等待当前呼吸）
      switch (breathGear) {
        case 1:
          currentPeriod = slowPeriod;
          Serial.println("切换为【慢呼吸】（3秒/周期）");
          break;
        case 2:
          currentPeriod = midPeriod;
          Serial.println("切换为【中呼吸】（1.5秒/周期）");
          break;
        case 3:
          currentPeriod = fastPeriod;
          Serial.println("切换为【快呼吸】（0.5秒/周期）");
          break;
      }
      
      // 更新最后触摸时间（防抖）
      lastTouchTime = millis();
    }
    
    // 更新上一次触摸状态
    lastTouchState = currentTouchState;
  }
}

// ==================== 呼吸灯驱动函数（即时响应档位变化）====================
void runBreathLight() {
  // 计算单次亮度变化的步长时间（周期/255/2：亮+灭各255级）
  unsigned long stepTime = currentPeriod / 510; 
  static unsigned long lastStepTime = 0;
  static int brightness = 0;    // 当前亮度（0-255）
  static int fadeDirection = 1; // 亮度变化方向：1=递增（亮），-1=递减（灭）
  
  // 按当前档位的步长时间更新亮度（即时响应档位变化）
  if (millis() - lastStepTime > stepTime) {
    // 更新亮度
    brightness += fadeDirection;
    
    // 到达亮度极值时反转方向
    if (brightness <= 0) {
      brightness = 0;
      fadeDirection = 1;
    } else if (brightness >= 255) {
      brightness = 255;
      fadeDirection = -1;
    }
    
    // 输出PWM亮度
    ledcWrite(pwmChannel, brightness);
    
    // 更新步长时间戳
    lastStepTime = millis();
  }
}