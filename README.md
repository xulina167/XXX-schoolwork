# XXX-schoolwork
学习以及提交作业

## ex01 Homework
### Task 1: Basic HelloWorld
A simple C program that prints "Hello World".

### Task 2: Keyboard Input
Added input function to read user's name and print a personalized greeting.
2026年 3月25日 早上八点打卡
### Task lab02: 
通过定义引脚常量控制两路 LED（引脚 2、17）同步闪烁，实现串口打印状态信息，掌握引脚输出模式配置、数字电平写入与延时函数的基础用法。
### Task lab03: 
通过配置 PWM 通道（频率 5000Hz、8 位分辨率）驱动引脚 2 的 LED 实现渐变呼吸效果，掌握 ledcSetup/ledcAttachPin/ledcWrite 的 PWM 核心用法，实现 LED 亮度从 0 到 255 平滑渐变。
### Task ex02:  
ex02 实验使用 millis () 函数代替 delay ()，实现 LED 以 1 赫兹的频率稳定闪烁，避免阻塞程序运行
### Task ex03:  
ex03 实验使用 millis () 函数实现 LED 灯的 SOS 求救信号闪烁，通过精准控制亮灭时长，让 LED 按照三短、三长、三短的摩尔斯电码规律闪烁，同时避免程序阻塞
### Task ex04:  
实现触摸传感器控制 LED 自锁效果，通过边缘检测 + 软件防抖，实现 “摸一下亮、再摸一下灭”，避免触摸误触发。
### Task ex05:  
结合 PWM 呼吸灯与触摸逻辑，实现 3 档循环调速呼吸灯，触摸引脚切换档位，不同档位呼吸节奏差异显著。
### Task ex06:  
基于双通道 PWM 实现警车双闪灯效，两 LED 亮度反相渐变，通过步长 / 峰值优化，交闪效果明显且过渡柔和。
