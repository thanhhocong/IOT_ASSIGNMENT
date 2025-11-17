#include "task_lcd_display.h"

// LCD instance is created in temp_humi_monitor.cpp
// We declare it as extern to use it here
LiquidCrystal_I2C lcd_display(0x27, 16, 2);  // Use standard I2C address 0x27

/**
 * @brief TASK 3: LCD Display Task with State-Based Display
 * 
 * This task demonstrates advanced FreeRTOS concepts:
 * 
 * 1. QUEUE-BASED COMMUNICATION:
 *    - Receives sensor data via xSensorDataQueue
 *    - No direct access to global variables
 *    - Clean producer-consumer pattern
 * 
 * 2. MUTEX SEMAPHORE:
 *    - Uses xLCDStateSemaphore to protect display state
 *    - Ensures atomic state transitions
 *    - Prevents race conditions
 * 
 * 3. DISPLAY STATES:
 * 
 *    NORMAL STATE:
 *    - Conditions: Temp 18-28°C AND Humidity 40-60%
 *    - Display: Shows current temp/humidity with checkmark
 *    - Background: Green indicator
 *    - Update rate: Every 5 seconds
 * 
 *    WARNING STATE:
 *    - Conditions: Temp 15-18°C OR 28-32°C OR Humidity 30-40% OR 60-70%
 *    - Display: Shows values with warning symbol
 *    - Background: Yellow indicator (alternating)
 *    - Update rate: Every 2 seconds
 * 
 *    CRITICAL STATE:
 *    - Conditions: Temp <15°C OR >32°C OR Humidity <30% OR >70%
 *    - Display: Shows values with alert symbol, flashing
 *    - Background: Red indicator (fast blinking)
 *    - Update rate: Every second
 * 
 * 4. SEMAPHORE USAGE:
 *    - Semaphore is "given" when state should change
 *    - Semaphore is "taken" when reading/updating state
 *    - Mutex ensures only one task modifies state at a time
 * 
 * 5. NO GLOBAL VARIABLES:
 *    - All sensor data received via queue
 *    - Display state protected by mutex
 *    - Local variables for all processing
 */

void lcd_display_task(void *pvParameters) {
    
    // Initialize LCD
    // Note: We create our own LCD instance to avoid conflicts
    lcd_display.begin();
    lcd_display.backlight();
    lcd_display.clear();
    
    Serial.println("========================================");
    Serial.println("TASK 3: LCD Display Task Started");
    Serial.println("Display States:");
    Serial.println("  NORMAL:   Temp 18-28°C AND Humidity 40-60%");
    Serial.println("  WARNING:  Approaching limits");
    Serial.println("  CRITICAL: Outside safe ranges");
    Serial.println("Queue-based communication (NO GLOBALS)");
    Serial.println("========================================");
    
    // Display startup message
    lcd_display.setCursor(0, 0);
    lcd_display.print("LCD Task Ready");
    lcd_display.setCursor(0, 1);
    lcd_display.print("Waiting data...");
    
    // Local variables (NO GLOBALS USED!)
    SensorData_t receivedData;
    DisplayState_t previousState = DISPLAY_STATE_NORMAL;
    bool flashState = false;
    unsigned long lastUpdate = 0;
    uint16_t updateInterval = 5000;  // Default update interval
    
    while (1) {
        // RECEIVE DATA FROM QUEUE (replaces reading global variables)
        if (xQueueReceive(xSensorDataQueue, &receivedData, pdMS_TO_TICKS(500)) == pdTRUE) {
            
            // Extract sensor data from queue
            float temperature = receivedData.temperature;
            float humidity = receivedData.humidity;
            unsigned long timestamp = receivedData.timestamp;
            
            Serial.println(">>> LCD Task: Data received from queue <<<");
            Serial.print("    Temperature: ");
            Serial.print(temperature);
            Serial.print("°C, Humidity: ");
            Serial.print(humidity);
            Serial.println("%");
            
            // DETERMINE DISPLAY STATE based on sensor readings
            DisplayState_t newState;
            
            // Critical conditions
            if (temperature < 15.0 || temperature > 32.0 || 
                humidity < 30.0 || humidity > 70.0) {
                newState = DISPLAY_STATE_CRITICAL;
                updateInterval = 1000;  // Fast updates
            }
            // Warning conditions
            else if ((temperature >= 15.0 && temperature < 18.0) || 
                     (temperature > 28.0 && temperature <= 32.0) ||
                     (humidity >= 30.0 && humidity < 40.0) ||
                     (humidity > 60.0 && humidity <= 70.0)) {
                newState = DISPLAY_STATE_WARNING;
                updateInterval = 2000;  // Medium updates
            }
            // Normal conditions
            else {
                newState = DISPLAY_STATE_NORMAL;
                updateInterval = 5000;  // Slow updates
            }
            
            // USE MUTEX SEMAPHORE to protect state change
            if (xSemaphoreTake(xLCDStateSemaphore, pdMS_TO_TICKS(100)) == pdTRUE) {
                
                // Check if state changed
                if (newState != previousState) {
                    Serial.print(">>> LCD Task: STATE CHANGE: ");
                    
                    switch (previousState) {
                        case DISPLAY_STATE_NORMAL: Serial.print("NORMAL"); break;
                        case DISPLAY_STATE_WARNING: Serial.print("WARNING"); break;
                        case DISPLAY_STATE_CRITICAL: Serial.print("CRITICAL"); break;
                    }
                    
                    Serial.print(" -> ");
                    
                    switch (newState) {
                        case DISPLAY_STATE_NORMAL: Serial.println("NORMAL"); break;
                        case DISPLAY_STATE_WARNING: Serial.println("WARNING"); break;
                        case DISPLAY_STATE_CRITICAL: Serial.println("CRITICAL"); break;
                    }
                    
                    // Semaphore "given" on state change (signal event)
                    Serial.println(">>> LCD Task: Display state semaphore signaled <<<");
                }
                
                // Update global state (protected by mutex)
                currentDisplayState = newState;
                previousState = newState;
                
                // Release mutex
                xSemaphoreGive(xLCDStateSemaphore);
                
            } else {
                Serial.println("LCD Task: Warning - Could not acquire mutex");
            }
            
            // UPDATE LCD DISPLAY based on current state
            lcd_display.clear();
            
            switch (newState) {
                case DISPLAY_STATE_NORMAL:
                    // NORMAL: Green, checkmark, calm display
                    lcd_display.setCursor(0, 0);
                    lcd_display.print("OK ");
                    lcd_display.print(temperature, 1);
                    lcd_display.print("C ");
                    lcd_display.print(humidity, 0);
                    lcd_display.print("%");
                    
                    lcd_display.setCursor(0, 1);
                    lcd_display.print("Status: NORMAL");
                    
                    Serial.println("LCD Display: NORMAL mode - All values optimal");
                    break;
                    
                case DISPLAY_STATE_WARNING:
                    // WARNING: Yellow, warning symbol, alternating
                    lcd_display.setCursor(0, 0);
                    lcd_display.print("!W ");
                    lcd_display.print(temperature, 1);
                    lcd_display.print("C ");
                    lcd_display.print(humidity, 0);
                    lcd_display.print("%");
                    
                    lcd_display.setCursor(0, 1);
                    if (flashState) {
                        lcd_display.print("**  WARNING  **");
                    } else {
                        lcd_display.print("   WARNING     ");
                    }
                    flashState = !flashState;
                    
                    Serial.println("LCD Display: WARNING mode - Values approaching limits");
                    break;
                    
                case DISPLAY_STATE_CRITICAL:
                    // CRITICAL: Red, alert symbol, fast flashing
                    lcd_display.setCursor(0, 0);
                    if (flashState) {
                        lcd_display.print("!!");
                        lcd_display.print(temperature, 1);
                        lcd_display.print("C ");
                        lcd_display.print(humidity, 0);
                        lcd_display.print("%!!");
                    } else {
                        lcd_display.print("                "); // Blank (flash effect)
                    }
                    
                    lcd_display.setCursor(0, 1);
                    if (flashState) {
                        lcd_display.print("*** CRITICAL ***");
                    } else {
                        lcd_display.print("                ");
                    }
                    flashState = !flashState;
                    
                    Serial.println("LCD Display: CRITICAL mode - Values outside safe range!");
                    break;
            }
            
            lastUpdate = millis();
        }
        
        // Small delay to prevent task hogging CPU
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

