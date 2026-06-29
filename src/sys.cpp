#include "sys.h"
#include <Preferences.h>

SystemMode currentMode = MODE_NORMAL;

struct SystemStatus sysStatus = {
    .bleConnected = false,
    .lastLedUpdate = 0
};



/*
字节 0 修饰键
    0x01: 左 Ctrl
    0x02: 左 Shift
    0x04: 左 Alt
    0x08: 左 GUI (Win/Cmd)
    0x10: 右 Ctrl
    0x20: 右 Shift
    0x40: 右 Alt
    0x80: 右 GUI

字节 1 保留

字节 2 按键 1
字节 3 按键 2
字节 4 按键 3
字节 5 按键 4
字节 6 按键 5
字节 7 按键 6
*/
KeyPreset presets[PRESET_COUNT] = {
    {
        {
            { 0x01, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00 }, // Ctrl+C
            { 0x01, 0x00, 0x19, 0x00, 0x00, 0x00, 0x00, 0x00 }, // Ctrl+V
            { 0x01, 0x00, 0x1B, 0x00, 0x00, 0x00, 0x00, 0x00 }, // Ctrl+X
            { 0x00 },                                           // 空
            { 0x00 }                                            // 空
            // { 0x01, 0x00, 0x1D, 0x00, 0x00, 0x00, 0x00, 0x00 }  // Ctrl+Z
        },
        "Ctrl CVX",
        { "Ctrl+C", "Ctrl+V", "Ctrl+X", "N/A", "N/A" }
    },
    {
        {
            { 0x00, 0x00, 0x52, 0x00, 0x00, 0x00, 0x00, 0x00 }, // 上
            { 0x00, 0x00, 0x51, 0x00, 0x00, 0x00, 0x00, 0x00 }, // 下
            { 0x00, 0x00, 0x50, 0x00, 0x00, 0x00, 0x00, 0x00 }, // 左
            { 0x00, 0x00, 0x4F, 0x00, 0x00, 0x00, 0x00, 0x00 }, // 右
            { 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0x00, 0x00 }  // 回车
        },
        "Arrow Keys",
        { "Up", "Down", "Left", "Right", "Enter" }
    },
    {
        //{ 0x05, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00 }, // Ctrl+Alt+B（注释掉的备用键）
        //{ 0x05, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00 }, // Ctrl+Alt+U（注释掉的备用键）
        {
            { 0x01, 0x00, 0x0E, 0x06, 0x00, 0x00, 0x00, 0x00 }, // Ctrl+K+C（注释）
            { 0x01, 0x00, 0x0E, 0x18, 0x00, 0x00, 0x00, 0x00 }, // Ctrl+K+U（取消注释）
            { 0x01, 0x00, 0x2F, 0x00, 0x00, 0x00, 0x00, 0x00 }, // Ctrl+[（左缩进）
            { 0x01, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00 }, // Ctrl+]（右缩进）
            { 0x00 }                                            // 空
        },
        "VSCode",
        { "(Comment)", "(Uncomment)", "(Left Align)", "(Right Align)", "N/A" }
    },
    {
        {
            
            { 0x04, 0x00, 0x1A, 0x00, 0x00, 0x00, 0x00, 0x00 }, // Alt+W（布线）
            { 0x04, 0x00, 0x19, 0x00, 0x00, 0x00, 0x00, 0x00 }, // Alt+V（过孔）
            { 0x04, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00 }, // Alt+E（铺铜）
            { 0x02, 0x00, 0x55, 0x00, 0x00, 0x00, 0x00, 0x00 }, // Shift+Num*（上层）
            { 0x00, 0x00, 0x55, 0x00, 0x00, 0x00, 0x00, 0x00 }  // Num*（下层）
        },
        "LCEDA Tools",
        { "Alt+W(Wire)", "Alt+V(Via)", "Alt+E(Copper)", "Upper Copper", "Lower Copper" }
    },
    {
        {
            { 0x00, 0x00, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00 }, // L - 直线
            { 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00 }, // C - 圆
            { 0x00, 0x00, 0x15, 0x00, 0x00, 0x00, 0x00, 0x00 }, // R - 矩形
            { 0x00, 0x00, 0x16, 0x00, 0x00, 0x00, 0x00, 0x00 }, // S - 智能尺寸
            { 0x01, 0x00, 0x25, 0x00, 0x00, 0x00, 0x00, 0x00 }  // Ctrl+8 - 正视于
        },
        "SW Drawing",
        { "Line", "Circle", "Rectangle", "Smart Dim", "Normal to" }
    },
/*************************************************************************************************************/
    {
        {
            { 0x00, 0x00, 0xE9, 0x00, 0x00, 0x00, 0x00, 0x00 }, // 音量增大
            { 0x00, 0x00, 0xEA, 0x00, 0x00, 0x00, 0x00, 0x00 }, // 音量减小
            { 0x00, 0x00, 0x6F, 0x00, 0x00, 0x00, 0x00, 0x00 }, // 亮度增大（不工作）
            { 0x00, 0x00, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00 }, // 亮度减小（不工作）
            { 0x00 }
        },
        "MediaCtrls(N/A)",
        { "Volume +", "Volume -", "Brightness +", "Brightness -", "N/A" } // 当前不工作...
    }
};

char media[5][2] { // 不可用 :(
    { 0x02, 0x00 }, // 音量增大
    { 0x03, 0x00 }, // 音量减小
    { 0x00, 0x10 }, // 
    { 0x00, 0x80 }, // 
    { 0x00, 0x00 }  // 
};

uint8_t currentPreset = 0;
uint8_t scrollPos = 0;
bool changeName = false;

uint32_t lastActivityTime = 0;
bool screenOn = true;
bool active = false;

/**
  * @brief  根据按键状态切换系统模式。
  * @param  无
  * @retval 无
  */
void SYS_ModeSwitch() {
    if (k5LongPressed && !keyState[3].isPressed) {
        if (currentMode == MODE_NORMAL) {
            currentMode = MODE_TIMER_SET;
        } else if ( (currentMode == MODE_TIMER_SET) ||
                    (currentMode == MODE_METRONOME) ||
                    (currentMode == MODE_KEY_CONFIG) ) {
            currentMode = MODE_NORMAL;
        }
        k5LongPressed = false;
        k5PressStartTime = 0;
    } else if (k4LongPressed && !keyState[4].isPressed) {
        if (currentMode == MODE_NORMAL) {
            currentMode = MODE_METRONOME;
        };
        k4LongPressed = false;
        k4PressStartTime = 0;
    }
    if (k4LongPressed && k5LongPressed) {
        currentMode = MODE_KEY_CONFIG;
        k4LongPressed = false;
        k5LongPressed = false;
        k4PressStartTime = 0;
        k5PressStartTime = 0;
    }
}

/**
  * @brief  切换按键配置与按键映射。
  * @param  无
  * @retval 无
  */
 void SYS_KeyConfig() {
    // static uint8_t selectedOption = 0; // 0: 加载, 1: 新建

    if ( keyState[0].isPressed ) {
        currentPreset = (currentPreset + PRESET_COUNT - 1) % PRESET_COUNT;
        scrollPos = 0;
        changeName = true;
        delay(100);
    }
    
    if ( keyState[1].isPressed ) {
        currentPreset = (currentPreset + 1) % PRESET_COUNT;
        changeName = true;
        scrollPos = 0;
        delay(100);
    }
    
    if( keyState[2].isPressed ) {
        SYS_ConfirmPreset(currentPreset);
        SYS_ApplyPreset(currentPreset);
        delay(500);
        currentMode = MODE_NORMAL;
    }
}


void SYS_SavePreset() {

}

void SYS_LoadPreset() {
    Preferences prefs;
    prefs.begin("KEY_CONFIG", true);
    currentPreset = prefs.getUChar("preset", 0);
    prefs.end();
}

void SYS_ConfirmPreset(uint8_t preset) {
    Preferences prefs;
    prefs.begin("KEY_CONFIG", false);
    prefs.putUChar("preset", currentPreset);
    prefs.end();
}

void SYS_ApplyPreset(uint8_t presetIndex) {
    if (presetIndex >= PRESET_COUNT) { return; }

    memcpy(k1Buf, presets[presetIndex].keymap[0], 8);
    memcpy(k2Buf, presets[presetIndex].keymap[1], 8);
    memcpy(k3Buf, presets[presetIndex].keymap[2], 8);
    memcpy(k4Buf, presets[presetIndex].keymap[3], 8);
    memcpy(k5Buf, presets[presetIndex].keymap[4], 8);

    // tone(BUZZER_PIN, 1000, 100);  // 调试时暂时禁用蜂鸣器
    // delay(100);
}

/**
  * @brief  根据当前系统状态更新状态 LED。
  * @param  无
  * @retval 无
  */
void SYS_StatusLEDCtrl() {

    static bool ledState = LOW;

    // if(!screenOn) {
    //     digitalWrite(STATUS_LED, LOW);
    //     return;
    // }

    if (currentMode == MODE_METRONOME) {
        return;
    }

    if (sysStatus.bleConnected) {                           // 已连接：常亮
        digitalWrite(STATUS_LED, HIGH);
    } else {                                                // 未连接：1Hz 闪烁
        if (millis() - sysStatus.lastLedUpdate > 500) {
            ledState = !ledState;
            digitalWrite(STATUS_LED, ledState);
            sysStatus.lastLedUpdate = millis();
        }
    }

}

