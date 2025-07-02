#include "joystick.h"
#include "main.h"
#include "stm32f4xx_hal.h"

extern ADC_HandleTypeDef hadc1;
extern DMA_HandleTypeDef hdma_adc1;

// Thresholds for direction detection
#define JOY_THRESHOLD_LOW    1000
#define JOY_THRESHOLD_HIGH   3000

// ADC buffer for 4 channels: [CH0, CH1, CH4, CH8]
// CH0 = PA0 (JS1_VRX), CH1 = PA1 (JS1_VRY)
// CH4 = PA4 (JS2_VRX), CH8 = PB0 (JS2_VRY)
volatile uint32_t adc_buffer[4] = {0};
volatile uint8_t adc_conversion_complete = 0;

void Joystick_Init(void) {
    // Nothing needed here unless ADC calibration or startup logic is required
}

// Called by HAL when ADC DMA conversion is complete
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) {
    if (hadc->Instance == hadc1.Instance) {
        adc_conversion_complete = 1;
    }
}

// Start one-time DMA ADC conversion for all channels
void Joystick_StartConversion(void) {
    adc_conversion_complete = 0;
    HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_buffer, 4);
}

// Wait until conversion is done (max ~10ms)
void Joystick_WaitAndStopConversion(void) {
    uint32_t timeout = HAL_GetTick() + 10;
    while (!adc_conversion_complete) {
        if (HAL_GetTick() > timeout) {
            break; // Timeout fallback
        }
    }
    HAL_ADC_Stop_DMA(&hadc1);
}

// Get raw X/Y values for one joystick
void Joystick_GetRawValues(JoystickId id, uint16_t* x_val, uint16_t* y_val) {
    Joystick_StartConversion();
    Joystick_WaitAndStopConversion();

    if (id == JOYSTICK_1) {
        *x_val = adc_buffer[0]; // CH0
        *y_val = adc_buffer[1]; // CH1
    } else {
        *x_val = adc_buffer[2]; // CH4
        *y_val = adc_buffer[3]; // CH8
    }
}

// Read direction of single joystick (8 directions including diagonals)
int Joystick_ReadDirection(JoystickId id) {
    uint16_t x_val = 0, y_val = 0;
    Joystick_GetRawValues(id, &x_val, &y_val);

    // Check for diagonal directions first
    if (x_val < JOY_THRESHOLD_LOW && y_val < JOY_THRESHOLD_LOW) {
        return JOY_DOWN_LEFT;
    }
    if (x_val > JOY_THRESHOLD_HIGH && y_val < JOY_THRESHOLD_LOW) {
        return JOY_UP_LEFT;
    }
    if (x_val < JOY_THRESHOLD_LOW && y_val > JOY_THRESHOLD_HIGH) {
        return JOY_DOWN_RIGHT;
    }
    if (x_val > JOY_THRESHOLD_HIGH && y_val > JOY_THRESHOLD_HIGH) {
        return JOY_UP_RIGHT;
    }
    
    // Check for cardinal directions
    if (x_val < JOY_THRESHOLD_LOW) return JOY_DOWN;
    if (x_val > JOY_THRESHOLD_HIGH) return JOY_UP;
    if (y_val < JOY_THRESHOLD_LOW) return JOY_LEFT;
    if (y_val > JOY_THRESHOLD_HIGH) return JOY_RIGHT;

    return JOY_NONE;
}

// Read buttons (digital)
bool Joystick_ReadButton(JoystickId id) {
    if (id == JOYSTICK_1)
        return HAL_GPIO_ReadPin(JS1_SW_GPIO_Port, JS1_SW_Pin) == GPIO_PIN_RESET;
    else
        return HAL_GPIO_ReadPin(JS2_SW_GPIO_Port, JS2_SW_Pin) == GPIO_PIN_RESET;
}

// Optimized function: read both joystick directions using one ADC read (8 directions)
void Joystick_ReadBothDirections(int* joy1_direction, int* joy2_direction) {
    Joystick_StartConversion();
    Joystick_WaitAndStopConversion();

    uint16_t joy1_x = adc_buffer[0];
    uint16_t joy1_y = adc_buffer[1];
    uint16_t joy2_x = adc_buffer[2];
    uint16_t joy2_y = adc_buffer[3];

    // Joystick 1 - Check diagonals first
    if (joy1_x < JOY_THRESHOLD_LOW && joy1_y < JOY_THRESHOLD_LOW) *joy1_direction = JOY_DOWN_LEFT;
    else if (joy1_x > JOY_THRESHOLD_HIGH && joy1_y < JOY_THRESHOLD_LOW) *joy1_direction = JOY_UP_LEFT;
    else if (joy1_x < JOY_THRESHOLD_LOW && joy1_y > JOY_THRESHOLD_HIGH) *joy1_direction = JOY_DOWN_RIGHT;
    else if (joy1_x > JOY_THRESHOLD_HIGH && joy1_y > JOY_THRESHOLD_HIGH) *joy1_direction = JOY_UP_RIGHT;
    else if (joy1_x < JOY_THRESHOLD_LOW) *joy1_direction = JOY_DOWN;
    else if (joy1_x > JOY_THRESHOLD_HIGH) *joy1_direction = JOY_UP;
    else if (joy1_y < JOY_THRESHOLD_LOW) *joy1_direction = JOY_LEFT;
    else if (joy1_y > JOY_THRESHOLD_HIGH) *joy1_direction = JOY_RIGHT;
    else *joy1_direction = JOY_NONE;

    // Joystick 2 - Check diagonals first
    if (joy2_x < JOY_THRESHOLD_LOW && joy2_y < JOY_THRESHOLD_LOW) *joy2_direction = JOY_DOWN_LEFT;
    else if (joy2_x > JOY_THRESHOLD_HIGH && joy2_y < JOY_THRESHOLD_LOW) *joy2_direction = JOY_UP_LEFT;
    else if (joy2_x < JOY_THRESHOLD_LOW && joy2_y > JOY_THRESHOLD_HIGH) *joy2_direction = JOY_DOWN_RIGHT;
    else if (joy2_x > JOY_THRESHOLD_HIGH && joy2_y > JOY_THRESHOLD_HIGH) *joy2_direction = JOY_UP_RIGHT;
    else if (joy2_x < JOY_THRESHOLD_LOW) *joy2_direction = JOY_DOWN;
    else if (joy2_x > JOY_THRESHOLD_HIGH) *joy2_direction = JOY_UP;
    else if (joy2_y < JOY_THRESHOLD_LOW) *joy2_direction = JOY_LEFT;
    else if (joy2_y > JOY_THRESHOLD_HIGH) *joy2_direction = JOY_RIGHT;
    else *joy2_direction = JOY_NONE;
}

// Optional: Read all raw values at once
void Joystick_ReadAllValues(uint16_t* joy1_x, uint16_t* joy1_y, uint16_t* joy2_x, uint16_t* joy2_y) {
    Joystick_StartConversion();
    Joystick_WaitAndStopConversion();

    *joy1_x = adc_buffer[0];
    *joy1_y = adc_buffer[1];
    *joy2_x = adc_buffer[2];
    *joy2_y = adc_buffer[3];
}
