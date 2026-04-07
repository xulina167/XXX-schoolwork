#include <WiFi.h>
#include <WebServer.h>

// ==================== 配置项（仅改这2行）====================
const char* ssid = "REDMI K80 Pro";    // 替换为自己的2.4G WiFi名
const char* password = "a15309119633";     // 替换为WiFi密码
// ============================================================

// 硬件引脚定义
const int touchPin = T0;       // 触摸引脚（T0 = GPIO4）
const int ledPin = 2;          // ESP32板载LED引脚（可换其他引脚）

// 全局状态变量（核心）
bool isArmed = false;          // 布防状态：false=撤防，true=布防
bool isAlarming = false;       // 报警状态：false=未报警，true=报警锁定

WebServer server(80);          // Web服务器（端口80）

// ==================== 网页请求处理函数 ====================
// 处理布防请求：仅切换布防状态，LED保持熄灭
void handleArm() {
  isArmed = true;
  isAlarming = false;          // 布防时重置报警状态
  digitalWrite(ledPin, LOW);   // 强制熄灭LED
  server.send(200, "text/plain", "已布防！触碰GPIO4触发报警");
}

// 处理撤防请求：重置所有状态，LED停止闪烁
void handleDisarm() {
  isArmed = false;
  isAlarming = false;
  digitalWrite(ledPin, LOW);   // 强制熄灭LED
  server.send(200, "text/plain", "已撤防！报警已停止");
}

// 返回控制网页（带状态显示）
void handleRoot() {
  // 构建网页（自适应手机/电脑，中文无乱码）
  String html = R"HTML(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>ESP32安防报警器</title>
  <style>
    * { margin: 0; padding: 0; box-sizing: border-box; }
    body { 
      background: #f5f5f5; 
      font-family: Arial, "Microsoft Yahei"; 
      text-align: center;
      padding-top: 60px;
    }
    .container {
      background: white;
      padding: 40px;
      border-radius: 15px;
      box-shadow: 0 0 20px rgba(0,0,0,0.1);
      max-width: 400px;
      margin: 0 auto;
    }
    .status {
      font-size: 24px;
      margin: 30px 0;
      font-weight: bold;
    }
    .btn {
      width: 150px;
      height: 70px;
      font-size: 22px;
      margin: 10px;
      border: none;
      border-radius: 10px;
      color: white;
      cursor: pointer;
    }
    .arm-btn { background: #e74c3c; }  /* 布防按钮-红色 */
    .disarm-btn { background: #2ecc71; }/* 撤防按钮-绿色 */
  </style>
</head>
<body>
  <div class="container">
    <h1>ESP32安防报警器</h1>
    <div class="status" id="status">当前状态：撤防（触摸无反应）</div>
    <button class="btn arm-btn" onclick="sendRequest('/arm')">布防 (Arm)</button>
    <button class="btn disarm-btn" onclick="sendRequest('/disarm')">撤防 (Disarm)</button>
  </div>

  <script>
    // 发送请求并刷新页面更新状态
    function sendRequest(path) {
      fetch(path)
        .then(() => location.reload())
        .catch(err => alert("操作失败：" + err));
    }

    // 页面加载后更新状态显示
    window.onload = function() {
      const statusDiv = document.getElementById('status');
      // 从URL参数/服务器状态同步（简化版直接通过页面刷新更新）
      // 实际状态已在网页生成时由ESP32填充，此处仅做视觉强化
    };
  </script>
</body>
</html>
  )HTML";

  // 根据全局状态动态修改网页显示的状态文本
  if (isArmed && !isAlarming) {
    html.replace("当前状态：撤防（触摸无反应）", "当前状态：<span style='color:#e74c3c;'>布防（触碰GPIO4报警）</span>");
  } else if (isAlarming) {
    html.replace("当前状态：撤防（触摸无反应）", "当前状态：<span style='color:red;'>报警中（仅撤防可停止）</span>");
  }

  // 发送网页，指定UTF-8编码避免中文乱码
  server.send(200, "text/html; charset=utf-8", html);
}

// ==================== 初始化函数 ====================
void setup() {
  // 初始化串口（调试用）
  Serial.begin(115200);
  
  // 初始化LED引脚为输出，默认熄灭
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  // 连接WiFi（带超时处理，避免卡死）
  Serial.print("连接WiFi：");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  
  int wifiTimeout = 0;
  while (WiFi.status() != WL_CONNECTED && wifiTimeout < 40) {
    delay(500);
    Serial.print(".");
    wifiTimeout++;
  }

  // WiFi连接成功后启动Web服务器
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi连接成功！");
    Serial.print("报警器控制页面：http://");
    Serial.println(WiFi.localIP()); // 打印访问地址（关键！）
    
    // 注册网页路由
    server.on("/", handleRoot);    // 主页（控制界面）
    server.on("/arm", handleArm);  // 布防接口
    server.on("/disarm", handleDisarm); // 撤防接口
    server.begin();                // 启动服务器
    Serial.println("安防报警器已就绪！");
  } else {
    Serial.println("\nWiFi连接失败！检查2.4G网络/密码");
  }
}

// ==================== 主循环（核心逻辑）====================
void loop() {
  // 处理网页客户端请求（必须放在循环开头）
  server.handleClient();

  // 1. 布防状态下检测触摸，触发报警（一旦触发就锁定）
  if (isArmed && !isAlarming) {
    // 触摸值<30判定为有效触摸（值越小越灵敏，可微调）
    if (touchRead(touchPin) < 30) {
      isAlarming = true; // 锁定报警状态（松手也不会解除）
      Serial.println("⚠️ 检测到触碰！触发报警！");
    }
  }

  // 2. 报警状态下控制LED高频狂闪（非阻塞，不影响网页响应）
  if (isAlarming) {
    static unsigned long lastBlink = 0;
    // 50ms切换一次（高频闪烁，可改数值调整速度）
    if (millis() - lastBlink > 50) {
      digitalWrite(ledPin, !digitalRead(ledPin));
      lastBlink = millis();
    }
  } else {
    // 非报警状态强制熄灭LED（布防/撤防都灭）
    digitalWrite(ledPin, LOW);
  }
}