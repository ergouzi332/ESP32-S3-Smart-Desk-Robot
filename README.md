# ESP32-S3-smart-desk-terminal# ESP32-S3 桌面智能终端

[![PlatformIO](https://img.shields.io/badge/PlatformIO-IDE-blue)](https://platformio.org/)
[![ESP32-S3](https://img.shields.io/badge/MCU-ESP32--S3--N16R8-green)](https://www.espressif.com/)

> 基于 ESP32-S3-N16R8 的桌面智能交互机器人，支持语音控制、集成 OLED 显示、心率监测、温湿度检测、天气查询、时间查询、电量查询等功能。
> 已完成PCB设计打板并焊接验证，调试功能完善。

---

## 功能介绍

| 功能 | 说明 |
|------|------|
| 🎤 **语音控制** | 通过 SU-03T 语音模块，识别语音通过 UART 传指令给 ESP32 即可切换功能 |
| 🖥️ **OLED 显示** | 0.96寸 128×64 OLED 屏幕，显示时间、天气、心率、温湿度等，以及天气、WiFI、表情等图标 |
| 🫀 **心率监测** | MAX30102 传感器，10秒测量，显示平均心率（BPM） |
| 🌡️ **温湿度检测** | DHT11 传感器，可读取环境温湿度 |
| 🌤️ **天气查询** | 连接 WiFi 后可通过HTTP获取实时天气，显示天气图标和温度 |
| ⏰ **网络授时** | 连接 WiFi 后可通过HTT获取实时时间戳，再换算成北京时间（东八区） |
| 🔋 **电量检测** | ADC 采集电池电压，实时显示百分比 |
| 🔊 **音频输出** | SU-03T 语音模块指令反馈语音播报 |

---

## 硬件设计

### 引脚分配

| 外设 | 接口 | 引脚 |
|------|------|------|
| OLED SSD1306 | I2C0 | SDA=21, SCL=20 |
| SU-03T 语音模块 | UART2 | RX=42, TX=41 |
| MAX30102 心率 | I2C1 | SDA=16, SCL=15 |
| DHT11 温湿度 | GPIO | DATA=18 |
| 电池检测 | ADC | GPIO=4 |

> 原理图与 PCB 使用 **嘉立创 EDA** 设计并打板，已焊接调试完成。

---

## 软件架构

### 项目结构
```text
SmartDeskRobot/
├── src/                          # 源代码目录
│   ├── main.cpp                  # 主程序入口 + 状态机核心逻辑
│   ├── graphic.cpp               # OLED 绘图模块（图标、文字、表情动画）
│   ├── mywifi.cpp                # WiFi 连接与网络状态管理
│   ├── mytime.cpp                # 网络授时模块（苏宁 API）
│   ├── weather.cpp               # 天气数据获取模块（心知天气 API）
│   ├── max30102.cpp              # MAX30102 心率/血氧传感器驱动
│   ├── dth11.cpp                 # DHT11 温湿度传感器驱动
│   └── battery.cpp               # 电池电压采集与电量检测
├── include/                      # 公共头文件目录
└── platformio.ini                # PlatformIO 项目配置文件
```

### 技术要点

- **非阻塞设计**：所有定时使用 `millis()`，不使用 `delay()`
- **心率算法**：自适应阈值检测 + 4次滑动平均滤波
- **WiFi 管理**：硬件自动重连 + 软件状态轮询
- **中文显示**：U8g2 库加载中文字库，支持 UTF-8 编码
- **自建图标库**：自定义 OLED 图标、表情、图案字库
- **网络解析**：HTTP 协议请求 + JSON 数据解析处理
- **UI 界面**：基于 U8g2 绘制完整界面与动画效果
- **语音通信**：串口指令通信，语音模块触发状态切换
- **状态机**：多界面统一调度，指令切换

---

## 开发环境

| 项目 | 内容 |
|------|------|
| 主控芯片 | ESP32-S3-N16R8 |
| 开发框架 | Arduino (PlatformIO) |
| 主要依赖 | U8g2、ArduinoJson、DHT sensor library |
| PCB 设计 | 嘉立创 EDA |
