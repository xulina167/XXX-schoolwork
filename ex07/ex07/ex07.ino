#include <WiFi.h>
#include <WebServer.h>

// WiFi配置
const char* ssid = "REDMI K80 Pro";
const char* password = "a15309119633";

// Web服务器对象，监听80端口
WebServer server(80);

// PWM配置（ESP32的PWM通道/频率/分辨率）
const int ledPin = 2;       // LED连接GPIO2（板载LED）
const int pwmChannel = 0;   // PWM通道0
const int freq = 5000;      // PWM频率5kHz
const int resolution = 8;   // 8位分辨率（0-255）

// 处理根路径请求，返回网页内容
void handleRoot() {
  // 网页HTML+JS代码
  String html = R"HTML(
    <!DOCTYPE html>
    <html>
    <head>
      <meta charset="UTF-8">
      <meta name="viewport" content="width=device-width, initial-scale=1.0">
      <title>ESP32 PWM调光器</title>
      <style>
        body { text-align: center; margin-top: 50px; font-family: Arial; }
        input[type="range"] { width: 80%; max-width: 400px; }
        h2 { color: #333; }
        .value { font-size: 20px; margin-top: 20px; color: #666; }
      </style>
    </head>
    <body>
      <h2>ESP32 LED无极调光</h2>
      <!-- 滑动条：0-255对应PWM 8位分辨率 -->
      <input type="range" id="brightness" min="0" max="255" value="0">
      <div class="value">亮度值: <span id="value">0</span></div>

      <script>
        // 获取滑动条和数值显示元素
        const slider = document.getElementById('brightness');
        const valueDisplay = document.getElementById('value');

        // 监听滑动条变化事件
        slider.addEventListener('input', function() {
          const brightness = this.value;
          valueDisplay.textContent = brightness; // 更新页面显示值
          
          // 发送GET请求到ESP32，携带亮度数值
          fetch(`/setPWM?value=${brightness}`)
            .catch(err => console.error('请求失败:', err));
        });
      </script>
    </body>
    </html>
  )HTML";
  
  // 返回网页，设置Content-Type为HTML
  server.send(200, "text/html", html);
}

// 处理PWM数值设置请求
void handleSetPWM() {
  // 检查请求中是否有"value"参数
  if (server.hasArg("value")) {
    // 解析参数为整数（0-255）
    int pwmValue = server.arg("value").toInt();
    // 限制数值范围（防止非法值）
    pwmValue = constrain(pwmValue, 0, 255);
    // 设置PWM输出
    ledcWrite(pwmChannel, pwmValue);
    // 返回成功响应
    server.send(200, "text/plain", "OK: " + String(pwmValue));
  } else {
    // 无参数时返回错误
    server.send(400, "text/plain", "Error: 缺少value参数");
  }
}

void setup() {
  // 初始化串口（调试用）
  Serial.begin(115200);
  
  // 配置PWM
  ledcSetup(pwmChannel, freq, resolution);  // 初始化PWM通道
  ledcAttachPin(ledPin, pwmChannel);        // 将引脚绑定到PWM通道
  
  // 连接WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi已连接，IP地址: " + WiFi.localIP().toString());

  // 注册路由处理函数
  server.on("/", handleRoot);          // 根路径返回网页
  server.on("/setPWM", handleSetPWM);  // /setPWM路径处理调光请求
  
  // 启动Web服务器
  server.begin();
  Serial.println("Web服务器已启动");
}

void loop() {
  // 处理客户端请求
  server.handleClient();
}