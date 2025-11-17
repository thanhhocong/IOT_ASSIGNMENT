#ifndef __TASK_LCD_DISPLAY_H__
#define __TASK_LCD_DISPLAY_H__

#include <Arduino.h>
#include "LiquidCrystal_I2C.h"
#include "global.h"

/**
 * @brief TASK 3: LCD Display Task with State-Based Display
 * 
 * This task manages an LCD display that shows different information
 * based on sensor readings. It demonstrates:
 * 1. Queue-based communication (receives data via xSensorDataQueue)
 * 2. Mutex semaphore for display state protection
 * 3. Three distinct display states (NORMAL, WARNING, CRITICAL)
 * 4. Elimination of global variables through FreeRTOS primitives
 */

void lcd_display_task(void *pvParameters);

#endif

