# ESP32-C3 BLE Keybrick - 二次开发版

基于 [WilliTourt/ESP32-Keyboard-Customized](https://github.com/WilliTourt/ESP32-Keyboard-Customized) 的二次开发项目。

> **原作者：** WilliTourt | **原项目协议：** GPL-3.0

---

## 项目简介

一款基于 ESP32-C3 的蓝牙迷你键盘，通过 BLE HID 协议连接电脑/手机，支持可编程按键、OLED 显示、倒计时器、节拍器等功能。

## 功能特性

| 功能 | 状态 |
|------|------|
| 5 个可编程按键（支持组合键） | 可用 |
| 7 个内置快捷键预设 | 可用 |
| 预设持久化存储（NVS） | 可用 |
| 128x32 OLED 显示屏 | 可用 |
| 倒计时器 + 蜂鸣器提醒 | 可用 |
| 节拍器（40-240 BPM） | 可用 |
| 锂电池电量监测 | 可用 |
| 屏幕超时自动熄屏 | 可用 |
| BLE 自动重连 | 待实现 |
| 媒体键支持 | 待调试 |

## 内置预设

| 编号 | 名称 | 用途 |
|------|------|------|
| 0 | Ctrl CVX | Ctrl+C / Ctrl+V / Ctrl+X |
| 1 | Win Combos | Alt+Tab / Win+D / Ctrl+Alt+Del |
| 2 | VSCode | 注释/取消注释 / 缩进/反缩进 |
| 3 | LCEDA Tools | 立创 EDA 布线/过孔/铺铜快捷键 |
| 4 | SW Drawing | SolidWorks 绘图工具快捷键 |
| 5 | Tab.b.b.b.b. | 全部为 Tab 键 |
| 6 | MediaCtrls | 音量/亮度控制（待调试） |

## 硬件配置

### 引脚定义

| GPIO | 功能 | 说明 |
|------|------|------|
| 0 | 电池电压检测 | ADC，分压比 0.6357 |
| 1 | 蜂鸣器 | PWM 输出 |
| 2 | 按键 1 | 低电平有效 |
| 3 | 按键 2 | 低电平有效 |
| 4 | 按键 3 | 低电平有效 |
| 5 | 按键 4 | 低电平有效，支持长按 |
| 6 | OLED SDA | 软件 I2C |
| 7 | OLED SCL | 软件 I2C |
| 8 | 按键 5 | 低电平有效，支持长按 |
| 10 | 状态 LED | 高电平=已连接，闪烁=未连接 |

### 外设

- **MCU：** ESP32-C3-MINI-1 模组
- **显示屏：** 0.91 寸 SSD1306 OLED（128x32，I2C 地址 0x78）
- **电池：** 3.7V 锂电池（652772 型，厚度 ≤6.5mm）
- **充电：** TP4056 充电模块
- **蜂鸣器：** 无源蜂鸣器

## 操作说明

### 基本操作
- **按键 1-3：** 发送当前预设的快捷键组合
- **默认映射：** BTN1=Ctrl+C，BTN2=Ctrl+V，BTN3=Ctrl+X

### 模式切换（长按触发）
- **长按按键 4（2秒）：** 进入节拍器模式
- **长按按键 5（2秒）：** 进入倒计时器模式
- **同时长按按键 4+5：** 进入预设切换模式

### 预设切换
1. 同时长按按键 4+5 进入配置模式
2. 按键 1/2 浏览预设
3. 按键 3 确认并保存
4. 长按按键 5 退出

## 开发环境

- **构建系统：** [PlatformIO](https://platformio.org/)
- **框架：** Arduino (ESP-IDF)
- **开发板：** esp32-c3-devkitm-1

### 编译烧录

```bash
# 编译
pio run

# 烧录
pio run --target upload

# 串口监视器
pio device monitor
```

## 项目结构

```
├── platformio.ini              # PlatformIO 配置
├── include/                    # 头文件
│   ├── def.h                   # 引脚定义
│   ├── key.h                   # 按键状态定义
│   ├── sys.h                   # 系统模式和预设定义
│   ├── battery.h               # 电池监测定义
│   ├── oled.h                  # OLED 驱动头文件
│   ├── iic.h                   # 软件 I2C 头文件
│   ├── char.h                  # 字体和图标数据
│   ├── music.h                 # 音符和旋律定义
│   └── timerMetronome.h        # 定时器和节拍器定义
├── lib/
│   └── hid2ble/                # 自定义 BLE HID 键盘库
│       ├── Hid2Ble.cpp/.h      # BLE HID 核心实现
│       ├── BleConnectionStatus  # 连接状态管理
│       └── KeyboardOutputCallbacks
├── src/
│   ├── main.cpp                # 主程序入口
│   ├── sys.cpp                 # 系统逻辑（预设、模式切换）
│   ├── key.c                   # 按键扫描（消抖、长按检测）
│   ├── oled.c                  # SSD1306 OLED 驱动
│   ├── iic.c                   # 软件 I2C 实现
│   ├── battery.c               # 电池电压检测
│   ├── char.c                  # 字体和图标位图数据
│   └── timerMetronome.cpp      # 倒计时器和节拍器逻辑
└── test/
```

## 二次开发记录

在此处记录相对于原项目的改动：

- （待补充）

## 致谢

- [WilliTourt](https://github.com/WilliTourt) — 原项目作者
- [BearLaboratory](https://github.com/nicoverduin/BearLaboratory) — Hid2Ble 库
- @WoodBreeze — Bug 修复贡献

## 许可证

本项目基于 GPL-3.0 协议开源。详见原项目 [LICENSE](https://github.com/WilliTourt/ESP32-Keyboard-Customized)。
