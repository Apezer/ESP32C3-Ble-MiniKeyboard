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
| 6 个内置快捷键预设 | 可用 |
| 预设持久化存储（NVS） | 可用 |
| 128x32 OLED 显示屏 | 可用 |
| 倒计时器 + 蜂鸣器提醒 | 可用 |
| 节拍器（40-240 BPM） | 可用 |
| 锂电池电量监测 | 可用 |
| 屏幕超时自动熄屏 | 可用 |
| BLE 自动重连 | 待实现 |
| 媒体键支持 | 待调试 |

## 内置预设

| 编号 | 名称 | K1 | K2 | K3 | K4 | K5 |
|------|------|-----|-----|-----|-----|-----|
| 1 | Ctrl CVX | Ctrl+C | Ctrl+V | Ctrl+X | - | - |
| 2 | Arrow Keys | Up | Down | Left | Right | Enter |
| 3 | VSCode | 注释 | 取消注释 | 左缩进 | 右缩进 | - |
| 4 | LCEDA Tools | 布线 | 过孔 | 铺铜 | 切换上层 | 切换下层 |
| 5 | SW Drawing | 直线 | 圆 | 矩形 | 智能尺寸 | 正视于 |
| 6 | MediaCtrls | 音量+ | 音量- | 亮度+ | 亮度- | - |

> 媒体键预设（#6）当前不工作，待调试。

---

## 使用说明

### 首次配对

1. 给设备上电，LED 指示灯闪烁表示等待连接
2. 在电脑/手机的蓝牙设置中搜索 `ESP32C3 BLE Keybrick`
3. 点击配对，连接成功后 LED 变为常亮
4. OLED 屏幕显示连接状态、电量和当前预设的按键说明

### 基本操作

- **按键 1-5：** 发送当前预设对应的按键/组合键
- 默认预设为 Ctrl CVX（Ctrl+C / Ctrl+V / Ctrl+X）
- OLED 屏幕会轮播显示当前预设各按键的功能说明

### 模式切换

设备有 4 种工作模式，通过长按按键 4/5 切换：

| 操作 | 进入模式 |
|------|----------|
| 长按按键 4（2 秒） | 节拍器模式 |
| 长按按键 5（2 秒） | 倒计时器模式 |
| 同时长按按键 4+5 | 预设配置模式 |
| 长按按键 5（在特殊模式中） | 返回普通模式 |

### 倒计时器

**进入方式：** 长按按键 5

| 按键 | 功能 |
|------|------|
| K1 | 小时 +1 |
| K2 | 分钟 +1 |
| K3 | 启动倒计时 |
| K4 | 重置 |

- 倒计时期间返回普通模式，OLED 显示剩余时间
- 倒计时结束后播放蜂鸣器提醒，按任意键停止

### 节拍器

**进入方式：** 长按按键 4

| 按键 | 功能 |
|------|------|
| K1 | BPM -1 |
| K2 | BPM +1 |
| K3 | 切换拍号（1/4 ~ 8/4） |
| K4 | 开始/停止 |

- BPM 范围：40-240，默认 120
- 默认拍号：4/4
- 运行时 LED 在每小节第一拍闪烁

### 预设切换

**进入方式：** 同时长按按键 4+5

| 按键 | 功能 |
|------|------|
| K1 | 上一个预设 |
| K2 | 下一个预设 |
| K3 | 确认并保存 |
| 长按 K5 | 退出配置模式 |

- 预设通过 NVS 持久化存储，断电后不丢失
- 确认后自动应用新预设，蜂鸣器提示

### 屏幕省电

- 无操作 20 秒：屏幕自动降低亮度
- 无操作 30 秒：屏幕自动关闭
- 任意按键操作唤醒屏幕

---

## 按键处理架构

按键从硬件到 BLE 发送共经过 4 层处理：

```
┌──────────────────────────────────────────────────────────────┐
│  Layer 1: 硬件引脚读取 (key.c)                                │
│  digitalRead() 读取 GPIO 2/3/4/5/8，低电平 = 按下              │
├──────────────────────────────────────────────────────────────┤
│  Layer 2: 消抖扫描 KEY_Update() (key.c, 主循环)                │
│  5ms 消抖确认，记录 isPressed 状态                             │
│  K4/K5 额外检测 2 秒长按                                       │
├──────────────────────────────────────────────────────────────┤
│  Layer 3: 状态机 KEY_Detect() (main.cpp, 10ms 硬件定时器 ISR)   │
│  检测 isPressed → 设置 shouldSend = true                      │
│  检测松开 → 设置 sendRelease = true                            │
├──────────────────────────────────────────────────────────────┤
│  Layer 4: BLE 发送 KEY_Send() (main.cpp, 主循环)               │
│  shouldSend=true 时发送对应键值到 HID 设备                      │
│  sendRelease=true 时发送释放事件                                │
└──────────────────────────────────────────────────────────────┘
```

**按键数据流：**

1. **按下瞬间** — `KEY_Update()` 消抖确认 `isPressed=true`
2. **下一个 10ms 周期** — `KEY_Detect()` 检测到 `isPressed`，设置 `shouldSend=true`
3. **主循环中** — `KEY_Send()` 发送 8 字节 HID 报文（修饰键 + 键码），标记 `isReleased=true`
4. **松开后** — `KEY_Detect()` 确认松开，设置 `sendRelease=true`，主循环发送释放报文

**HID 报文格式（8 字节）：**

| 字节 | 内容 | 说明 |
|------|------|------|
| Byte 0 | 修饰键 | Ctrl/Shift/Alt/GUI 的位掩码 |
| Byte 1 | 保留 | 固定 0x00 |
| Byte 2 | 键码 1 | 主要按键的 HID Usage Code |
| Byte 3-5 | 键码 2-4 | 支持同时按下最多 6 个键 |
| Byte 6-7 | 预留 | 固定 0x00 |

**模式与按键映射：**

| 模式 | K1 | K2 | K3 | K4 | K5 |
|------|-----|-----|-----|-----|-----|
| 普通模式 | 预设键值 | 预设键值 | 预设键值 | 预设键值 | 预设键值 |
| 倒计时设置 | 小时+1 | 分钟+1 | 启动 | 重置 | 返回 |
| 节拍器 | BPM-1 | BPM+1 | 切换拍号 | 启停 | 返回 |
| 预设配置 | 上一个 | 下一个 | 确认保存 | - | 退出 |

---

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

### 外设清单

- **MCU：** ESP32-C3-MINI-1 模组
- **显示屏：** 0.91 寸 SSD1306 OLED（128x32，I2C 地址 0x78）
- **电池：** 3.7V 锂电池（652772 型，厚度 ≤6.5mm）
- **充电：** TP4056 充电模块
- **蜂鸣器：** 无源蜂鸣器

---

## 开发指南

### 开发环境

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

### 添加自定义预设

1. 打开 `include/sys.h`，将 `PRESET_COUNT` 加 1
2. 打开 `src/sys.cpp`，在 `presets[]` 数组末尾添加新预设：

```cpp
{
    {
        { modifier, 0x00, keycode, 0x00, 0x00, 0x00, 0x00, 0x00 }, // K1
        { modifier, 0x00, keycode, 0x00, 0x00, 0x00, 0x00, 0x00 }, // K2
        { modifier, 0x00, keycode, 0x00, 0x00, 0x00, 0x00, 0x00 }, // K3
        { modifier, 0x00, keycode, 0x00, 0x00, 0x00, 0x00, 0x00 }, // K4
        { modifier, 0x00, keycode, 0x00, 0x00, 0x00, 0x00, 0x00 }  // K5
    },
    "预设名称",
    { "K1说明", "K2说明", "K3说明", "K4说明", "K5说明" }
}
```

**修饰键（Byte 0）：**
| 值 | 修饰键 |
|----|--------|
| 0x01 | Left Ctrl |
| 0x02 | Left Shift |
| 0x04 | Left Alt |
| 0x08 | Left GUI (Win) |
| 0x00 | 无修饰键 |

常用 HID 键码参考：[USB HID Usage Tables](https://usb.org/sites/default/files/hut1_3_0.pdf)

---

## 项目结构

```
├── platformio.ini              # PlatformIO 配置
├── include/                    # 头文件
│   ├── def.h                   # 引脚定义
│   ├── key.h                   # 按键状态定义
│   ├── sys.h                   # 系统模式和预设定义
│   ├── battery.h               # 电池监测定义
│   ├── char.h                  # 图标位图数据声明
│   ├── music.h                 # 音符和旋律定义
│   └── timerMetronome.h        # 定时器和节拍器定义
├── lib/
│   └── hid2ble/                # 自定义 BLE HID 键盘库
│       ├── Hid2Ble.cpp/.h      # BLE HID 核心实现
│       ├── BleConnectionStatus  # 连接状态管理
│       └── KeyboardOutputCallbacks
├── src/
│   ├── main.cpp                # 主程序入口（OLED 显示、按键发送）
│   ├── sys.cpp                 # 系统逻辑（预设、模式切换）
│   ├── key.c                   # 按键扫描（消抖、长按检测）
│   ├── battery.c               # 电池电压检测
│   ├── char.c                  # 图标位图数据
│   └── timerMetronome.cpp      # 倒计时器和节拍器逻辑
└── test/
```

## 二次开发记录

相对于原项目的改动：

- OLED 驱动从自写软件 I2C 迁移至 Adafruit SSD1306 + GFX 库，删除 `oled.c`、`iic.c` 等文件
- 删除"Tab.b.b.b.b."预设（全部为 Tab 键，无实际用途），预设数量 7→6
- 预设 1 从"Win Combos"改为"Arrow Keys"（方向键 + Enter）
- 蜂鸣器代码暂时注释（调试用）
- 项目注释全部翻译为中文

## 致谢

- [WilliTourt](https://github.com/WilliTourt) — 原项目作者
- [BearLaboratory](https://github.com/nicoverduin/BearLaboratory) — Hid2Ble 库
- @WoodBreeze — Bug 修复贡献

## 许可证

本项目基于 GPL-3.0 协议开源。详见原项目 [LICENSE](https://github.com/WilliTourt/ESP32-Keyboard-Customized)。
