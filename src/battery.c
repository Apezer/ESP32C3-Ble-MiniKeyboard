#include "battery.h"

bool BAT_IS_LOW = false;

float BAT_Voltage = 0.0;

/**
  * @brief  读取电池电压并检测是否低电量。
  * @note   数据存储在：float BAT_Voltage
  * @param  无
  * @retval 无
  */
void BAT_Read() {
    static uint32_t batReadStartTime = 0;

    if (batReadStartTime == 0) {
        batReadStartTime = millis();
    } else if (millis() - batReadStartTime > BAT_READ_TIME_GAP) {

        uint16_t adcValue = analogRead(ADC_PIN);
        float adcVoltage = (float)adcValue * ADC_VREF / 4096.0;
        BAT_Voltage = adcVoltage / BAT_VOLT_DIVIDER;
        (BAT_Voltage < BAT_LOW) ? (BAT_IS_LOW = true) : (BAT_IS_LOW = false);

        batReadStartTime = 0;
    }

    // return BAT_Voltage;（已注释，不需要返回值）
}

/**
  * @brief  获取电池电量百分比。
  * @param  无
  * @retval uint8_t 电量百分比
  */
uint8_t BAT_GetPercentage() {
    uint8_t batPercentage = (BAT_Voltage - BAT_EMPTY) / (BAT_FULL - BAT_EMPTY) * 100;
    batPercentage = (batPercentage > 100) ? 100 : batPercentage;
    return batPercentage;
}
