#include <WiFi.h>
#include <WebServer.h>

// ==================== 配置项 ====================
const char* ssid = "REDMI K80 Pro";    // 替换为你的WiFi名
const char* password = "a15309119633";     // 替换为你的WiFi密码
const int touchPin = T0;                   // T0=GPIO4（固定）
WebServer server(80);

// 触摸灵敏度配置（核心：放大靠近的感应效果）
int touchBaseline = 0;    // 无遮挡时的基准值
const int SMOOTH_FACTOR = 5; // 平滑系数（值越大，数值越稳定）
const int AMPLIFY_FACTOR = 3; // 放大系数（值越大，靠近越灵敏）
const int UPDATE_INTERVAL = 50; // 网页刷新间隔（50ms）

// 平滑滤波用数组
int touchValueBuffer[SMOOTH_FACTOR];
int bufferIndex = 0;

// ==================== 函数声明 ====================
void handleRoot();                // 返回仪表盘网页
void handleGetTouchValue();       // 返回校准后的触摸值
void calibrateTouchSensor();      // 校准基准值
int readSmoothTouchValue();       // 平滑读取触摸值
int amplifyTouchChange(int val);  // 放大靠近的变化量

// ==================== 初始化 ====================
void setup() {
  Serial.begin(115200);
  
  // 初始化缓冲区
  for (int i = 0; i < SMOOTH_FACTOR; i++) {
    touchValueBuffer[i] = 0;
  }
  
  // 校准触摸基准值（开机时读取无遮挡的初始值）
  calibrateTouchSensor();
  Serial.print("触摸基准值校准完成：");
  Serial.println(touchBaseline);

  // 连接WiFi
  Serial.print("连接WiFi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  
  int wifiTimeout = 0;
  while (WiFi.status() != WL_CONNECTED && wifiTimeout < 40) {
    delay(500);
    Serial.print(".");
    wifiTimeout++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi已连接！IP: " + WiFi.localIP().toString());
    // 注册Web路由
    server.on("/", handleRoot);
    server.on("/get_touch_value", handleGetTouchValue);
    server.begin();
    Serial.println("非接触触摸仪表盘已启动！靠近GPIO4导线即可感应");
  } else {
    Serial.println("\nWiFi连接失败！检查2.4G网络/密码");
  }
}

// ==================== 主循环 ====================
void loop() {
  server.handleClient();
  delay(10);
}

// ==================== 核心函数实现 ====================
// 1. 校准触摸基准值（多次读取取平均）
void calibrateTouchSensor() {
  int sum = 0;
  // 读取20次取平均，减少环境干扰
  for (int i = 0; i < 20; i++) {
    sum += touchRead(touchPin);
    delay(10);
  }
  touchBaseline = sum / 20;
}

// 2. 平滑读取触摸值（减少抖动，替代滤波函数）
int readSmoothTouchValue() {
  // 读取原始值并加入缓冲区
  int rawValue = touchRead(touchPin);
  touchValueBuffer[bufferIndex] = rawValue;
  bufferIndex = (bufferIndex + 1) % SMOOTH_FACTOR;
  
  // 计算缓冲区平均值
  int sum = 0;
  for (int i = 0; i < SMOOTH_FACTOR; i++) {
    sum += touchValueBuffer[i];
  }
  return sum / SMOOTH_FACTOR;
}

// 3. 放大靠近的变化量（核心：实现非接触感应）
int amplifyTouchChange(int val) {
  // 计算与基准值的差值（靠近时差值变大）
  int diff = touchBaseline - val;
  // 放大差值，让微弱变化更明显
  int amplifiedDiff = diff * AMPLIFY_FACTOR;
  // 计算最终显示值（值越小=越靠近）
  int displayValue = 1000 - amplifiedDiff;
  // 限制范围（0~1000），避免负数/异常值
  displayValue = constrain(displayValue, 0, 1000);
  return displayValue;
}

// 4. 返回仪表盘网页（AJAX高频刷新）
void handleRoot() {
  String html = R"HTML(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>ESP32非接触触摸仪表盘</title>
  <style>
    * { margin: 0; padding: 0; box-sizing: border-box; }
    body { 
      background: #0f172a; 
      font-family: Arial, "Microsoft Yahei"; 
      display: flex;
      align-items: center;
      justify-content: center;
      height: 100vh;
    }
    .dashboard {
      background: #1e293b;
      padding: 50px 80px;
      border-radius: 20px;
      box-shadow: 0 0 40px rgba(56, 189, 248, 0.4);
      text-align: center;
    }
    .title {
      font-size: 32px;
      color: #38bdf8;
      margin-bottom: 40px;
    }
    .value-display {
      font-size: 100px;
      font-weight: bold;
      color: #ffffff;
      margin: 20px 0;
      min-width: 350px;
      height: 140px;
      line-height: 140px;
      background: #0284c7;
      border-radius: 15px;
      transition: all 0.05s ease;
    }
    .tip {
      font-size: 18px;
      color: #94a3b8;
      margin-top: 30px;
    }
  </style>
</head>
<body>
  <div class="dashboard">
    <div class="title">非接触触摸传感器仪表盘</div>
    <div class="value-display" id="touchValue">0</div>
    <div class="tip">手指靠近GPIO4导线，数值实时减小</div>
  </div>

  <script>
    const valueDisplay = document.getElementById('touchValue');
    // 50ms刷新一次，实时感应
    setInterval(() => {
      fetch('/get_touch_value')
        .then(res => res.text())
        .then(val => {
          valueDisplay.textContent = val;
          // 视觉反馈：数值越小（越靠近），背景越红
          const num = parseInt(val);
          const red = Math.min(255, 255 - num * 0.25);
          const blue = Math.min(255, 50 + num * 0.2);
          valueDisplay.style.backgroundColor = `rgb(${red}, 0, ${blue})`;
        })
        .catch(err => console.error("获取数据失败：", err));
    }, 50);
  </script>
</body>
</html>
  )HTML";
  
  server.send(200, "text/html; charset=utf-8", html);
}

// 5. 返回高灵敏度触摸值（纯数字）
void handleGetTouchValue() {
  int smoothValue = readSmoothTouchValue();
  int displayValue = amplifyTouchChange(smoothValue);
  server.send(200, "text/plain", String(displayValue));
}