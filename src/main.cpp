
/*******************************************************
 * ESP32-C3 BLE Keybrick
 * Copyright (c) 2025 WilliTourt 2944925833@qq.com
*******************************************************/

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "Hid2Ble.h"

extern "C" {

#include "key.h"
#include "battery.h"

}

#include "sys.h"
#include "def.h"
#include "char.h"
#include "timerMetronome.h"

#define SCREEN_ALMOST_TIMEOUT 20000
#define SCREEN_TIMEOUT 30000
#define OLED_ADDR 0x3C

Adafruit_SSD1306 display(128, 32, &Wire, -1);

void OLED_Update();
void OLED_ChkTimeout();
void KEY_Send();

Hid2Ble keybrick("ESP32C3 BLE Keybrick", "WilliTourt", 100);

/**
  * @brief  Determine whether should send key events to BLE HID device, and count down for timer. 10ms timer interrupt.
  * @param  None
  * @retval None
  */
void IRAM_ATTR KEY_Detect() {

    // Loop through each key to check its state and decide if a key event should be sent
    for (int i = 0; i < 5; i++) {
        if (keyState[i].isReleased) {
            // If the key was released, ensure it is not pressed and set the flag to send a release event
            if (!keyState[i].isPressed) {
                sendRelease = true;
                keyState[i].isReleased = false;
            }
        } else {
            // If the key is pressed, mark it as shouldSend to send the press event later
            if (keyState[i].isPressed) {
                keyState[i].shouldSend = true;
            }
        }
    }

    // Variable for timer count down (each second)
    static uint8_t timerCnt = 0;
    if (timer.enabled) {
        timerCnt++;
        if (timerCnt >= 100) {
            uint32_t now = millis();
            if (now >= (timer.targetSec - 1)) {
                timerTriggered = true;
                timer.enabled = false;
            }
            timerCnt = 0;
        }
    }

}

/**
  * @brief  Update battery level to BLE HID device. 1min timer interrupt.
  * @param  None
  * @retval None
  */
void IRAM_ATTR BLE_UpdateBAT() {
    if (keybrick.isConnected()) {
        keybrick.setBatteryLevel(BAT_GetPercentage());
    }
}

void setup() {

    // Initialize keys, load and apply system preset
    KEY_Init();
    SYS_LoadPreset();
    SYS_ApplyPreset(currentPreset);

    Wire.begin(6, 7);  // SDA=GPIO6, SCL=GPIO7
    display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.display();

    pinMode(STATUS_LED, OUTPUT);
    // pinMode(BUZZER_PIN, OUTPUT);  // 调试时暂时禁用蜂鸣器

    hw_timer_t* timer = timerBegin(0, 80, true);
    timerAttachInterrupt(timer, &KEY_Detect, true);
    timerAlarmWrite(timer, 10000, true);            // 10 ms
    timerAlarmEnable(timer);

    hw_timer_t* timer1 = timerBegin(1, 800, true);
    timerAttachInterrupt(timer1, &BLE_UpdateBAT, true);
    timerAlarmWrite(timer1, 6000000, true);         // 1 min
    timerAlarmEnable(timer1);

    keybrick.begin();
}

void loop() {
    // Update key states and handle mode switching
    active = KEY_Update();

    if (BAT_IS_LOW) {
        display.dim(true);
    }

    SYS_ModeSwitch();

    // Check BLE connection status and send key events if connected
    if (keybrick.isConnected()) {
        sysStatus.bleConnected = true;

        if (enableKey) {
            KEY_Send();
        }
    } else {
        sysStatus.bleConnected = false;
        // How to reconnect automatically?
    }

    // Handle different modes
    switch (currentMode) {  // Clear OLED display and send a release event if mode changed
        case MODE_NORMAL:
            if (!enableKey) { display.clearDisplay(); keybrick.send2Ble(release); scrollPos = 0; }
            enableKey = true;
            TIMER_Display();
            break;
        case MODE_TIMER_SET:
            if (enableKey) { display.clearDisplay(); keybrick.send2Ble(release); }
            enableKey = false;
            TIMER_Set();
            break;
        case MODE_METRONOME:
            if (enableKey) { display.clearDisplay(); keybrick.send2Ble(release); }
            enableKey = false;
            METRONOME_Set();
            break;
        case MODE_KEY_CONFIG:
            if (enableKey) { display.clearDisplay(); keybrick.send2Ble(release); scrollPos = 0; }
            enableKey = false;
            SYS_KeyConfig();
            break;
    }

    TIMER_Handle();
    METRONOME_Handle();
    BAT_Read();
    SYS_StatusLEDCtrl();
    OLED_ChkTimeout();
    OLED_Update();
}

/**
  * @brief  Send key events to BLE HID device
  * @param  None
  * @retval None
  */
void KEY_Send() {
    for (int i = 0; i < 5; i++) {
        if (keyState[i].shouldSend && !keyState[i].isReleased) {
            // if (currentPreset == 4) { // MediaCtrl
            //     switch (i) {
            //         case 0: keybrick.sendMedia2Ble(media[0]); break;
            //         case 1: keybrick.sendMedia2Ble(media[1]); break;
            //         case 2: keybrick.sendMedia2Ble(media[2]); break;
            //         case 3: keybrick.sendMedia2Ble(media[3]); break;
            //         case 4: keybrick.sendMedia2Ble(media[4]); break;
            //     }
            // } else {
                switch (i) {
                    case 0: keybrick.send2Ble(k1Buf); break;
                    case 1: keybrick.send2Ble(k2Buf); break;
                    case 2: keybrick.send2Ble(k3Buf); break;
                    case 3: keybrick.send2Ble(k4Buf); break;
                    case 4: keybrick.send2Ble(k5Buf); break;
                }
            // }
            keyState[i].isReleased = true;
            keyState[i].shouldSend = false;
        }
    }
    if (sendRelease) {
        keybrick.send2Ble(release);
        sendRelease = false;
    }
}

// Check if screen timeout has been reached and turn off OLED if necessary
void OLED_ChkTimeout() {
    static uint32_t lastCheck = 0;
    if (active) {
        if (!BAT_IS_LOW) { display.dim(false); }
        lastActivityTime = millis();
        if (!screenOn) {
            display.ssd1306_command(SSD1306_DISPLAYON);
            screenOn = true;
        }
    }
    if (millis() - lastCheck > 1000) {
        if (screenOn && (millis() - lastActivityTime > SCREEN_ALMOST_TIMEOUT)) {
            display.dim(true);
        }
        if (screenOn && (millis() - lastActivityTime > SCREEN_TIMEOUT)) {
            display.ssd1306_command(SSD1306_DISPLAYOFF);
            screenOn = false;
        }
        lastCheck = millis();
    }
}

// OLED information display & prompts
void OLED_Update() {

    if (!screenOn) { return; }

    switch (currentMode) {
        case MODE_NORMAL:
            display.drawBitmap(0, 0, Title, 128, 8, SSD1306_WHITE);
            display.drawBitmap(2, 8, BT, 8, 8, SSD1306_WHITE);
            display.setTextSize(1);
            display.setCursor(10, 8);
            if (sysStatus.bleConnected) {
                display.print("Connected  ");
            } else {
                display.print("Unconnected");
            }
            display.drawBitmap(90, 8, Bat, 8, 8, SSD1306_WHITE);
            display.setCursor(100, 8);
            char batStr[4];
            sprintf(batStr, "%3d", BAT_GetPercentage());
            display.print(batStr);
            display.setCursor(118, 8);
            display.print("%");

            static uint32_t lastScroll = 0;
            if (scrollPos < 5) {
                display.fillRect(0, 16, 128, 8, SSD1306_BLACK);
                display.setCursor(0, 16);
                char keydesc[24];
                sprintf(keydesc, "Key%d: %s", scrollPos + 1, presets[currentPreset].keyDescription[scrollPos]);
                display.print(keydesc);
            }
            if (millis() - lastScroll > 2000) {
                scrollPos = (scrollPos + 1) % 5;
                lastScroll = millis();
                display.fillRect(0, 16, 128, 16, SSD1306_BLACK);
            }
            display.display();
            break;
        case MODE_TIMER_SET:
            display.setCursor(0, 0);
            display.setTextSize(1);
            display.print("> Timer Settings");
            display.setCursor(0, 8);
            display.setTextSize(2);
            char timeStr[16];
            sprintf(timeStr, " <%02d:%02d>", timer.hours, timer.minutes);
            display.print(timeStr);
            if (timer.enabled) {
                display.setTextSize(1);
                display.setCursor(72, 8);
                char timeEn[10];
                uint32_t remainingSec = (timer.targetSec - millis()) / 1000;
                sprintf(timeEn, "%02d:%02d[ON]", remainingSec / 3600, (remainingSec % 3600) / 60);
                display.print(timeEn);
            } else {
                display.fillRect(72, 8, 56, 8, SSD1306_BLACK);
            }
            display.setTextSize(1);
            display.setCursor(72, 16);
            display.print("Cnt Down");
            display.setCursor(0, 24);
            display.print("1|HH 2|MM 3|En 4|Rst");
            display.display();
            break;
        case MODE_METRONOME:
            display.setCursor(0, 0);
            display.setTextSize(1);
            display.print("> Metronome");
            display.setCursor(0, 8);
            display.setTextSize(2);
            char infoStr[32];
            sprintf(infoStr, "BPM:%03d SIG:%d/4", metro.bpm, metro.timeSig);
            display.print(infoStr);
            display.setTextSize(1);
            display.setCursor(0, 24);
            display.print("1|- 2|+ 3|Sig 4|");
            display.setCursor(96, 24);
            display.print(metro.isRunning ? "[RUN]" : "[OFF]");
            display.display();
            break;
        case MODE_KEY_CONFIG:

            static uint32_t lastScroll_cfg = 0;

            display.setTextSize(1);
            display.setCursor(0, 0);
            display.print("> Config Mode");
            if (changeName) {
                display.fillRect(30, 8, 98, 8, SSD1306_BLACK);
                changeName = false;
            }
            display.setCursor(0, 8);
            display.print(" Tag:");
            display.setCursor(36, 8);
            display.print((const char*)presets[currentPreset].name);
            for (int i = 0; i < 2; i++) {
                if (scrollPos + i < 5) {
                    display.fillRect(0, 16 + i * 8, 128, 8, SSD1306_BLACK);
                    display.setCursor(0, 16 + i * 8);
                    char line[24];
                    sprintf(line, "- Key%d: %s", (scrollPos + i + 1), presets[currentPreset].keyDescription[scrollPos + i]);
                    display.print(line);
                }
            }
            if (millis() - lastScroll_cfg > 2000) {
                scrollPos = (scrollPos + 1) % 4;
                lastScroll_cfg = millis();
                display.fillRect(0, 16, 128, 16, SSD1306_BLACK);
            }

            char presetInfo[8];
            sprintf(presetInfo, "[%d/%d]", currentPreset + 1, PRESET_COUNT);
            display.setCursor(96, 0);
            display.print(presetInfo);
            display.display();
            break;
    }

}

// If timer is enabled, display remaining time on OLED
void TIMER_Display() {
    display.setTextSize(1);
    if(timer.enabled) {
        char timeStr[32];
        uint32_t remainingSec = (timer.targetSec - millis()) / 1000;
        sprintf(timeStr, "TIM remaining: %02d:%02d", remainingSec / 3600, (remainingSec % 3600) / 60);
        display.setCursor(0, 24);
        display.print(timeStr);
    } else {
        display.fillRect(0, 24, 128, 8, SSD1306_BLACK);
    }
}
