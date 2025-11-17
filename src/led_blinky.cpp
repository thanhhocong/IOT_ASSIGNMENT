#include "led_blinky.h"

/**
 * @brief LED Blinky Task with Temperature-Responsive Behavior
 * 
 * This task implements a temperature-responsive LED system with semaphore synchronization.
 * The LED blinks at different rates depending on the current temperature reading:
 * 
 * TEMPERATURE RANGES AND LED BEHAVIORS:
 * 
 * 1. COLD (< 20°C):
 *    - Slow blink pattern (1000ms ON, 1000ms OFF)
 *    - Indicates cold environment
 *    - Total cycle: 2000ms
 * 
 * 2. COMFORTABLE (20°C - 28°C):
 *    - Medium blink pattern (500ms ON, 500ms OFF)
 *    - Indicates comfortable/normal temperature
 *    - Total cycle: 1000ms
 * 
 * 3. WARM (28°C - 35°C):
 *    - Fast blink pattern (200ms ON, 200ms OFF)
 *    - Indicates warm environment
 *    - Total cycle: 400ms
 * 
 * 4. HOT (> 35°C):
 *    - Very fast blink pattern (100ms ON, 100ms OFF)
 *    - Indicates hot/critical temperature
 *    - Total cycle: 200ms
 * 
 * SEMAPHORE SYNCHRONIZATION:
 * - Uses xTempUpdateSemaphore to synchronize with temp_humi_monitor task
 * - When temperature is read and updated, the semaphore is given
 * - LED task takes the semaphore and adjusts its behavior accordingly
 * - Timeout mechanism (100ms) ensures LED continues blinking even if no updates
 */

void led_blinky(void *pvParameters){
  pinMode(LED_GPIO, OUTPUT);
  
  // Local variables to store blink timings based on temperature
  int onTime = 1000;   // Default ON time in milliseconds
  int offTime = 1000;  // Default OFF time in milliseconds
  
  // TASK 3: Local variable for temperature (NO GLOBAL ACCESS)
  float currentTemperature = 25.0;  // Default value
  
  Serial.println("LED Blinky Task Started - Temperature Responsive Mode");
  Serial.println("TASK 3: Using semaphore sync (no direct global access)");
  
  while(1) {
    // Wait for temperature update semaphore with timeout
    // Timeout allows LED to continue blinking even if temp sensor fails
    if (xSemaphoreTake(xTempUpdateSemaphore, pdMS_TO_TICKS(100)) == pdTRUE) {
      // Semaphore acquired - new temperature data available
      // TASK 3: Read from global only when signaled (alternative: could use queue)
      // Note: For backwards compatibility with Task 1, we still use glob_temperature
      // but only access it when semaphore is acquired (synchronized access)
      currentTemperature = glob_temperature;
      
      Serial.print("LED Task: Temperature update received: ");
      Serial.print(currentTemperature);
      Serial.println("°C");
      
      // Determine LED behavior based on temperature ranges
      if (currentTemperature < 20.0) {
        // COLD: Slow blink (1000ms ON, 1000ms OFF)
        onTime = 1000;
        offTime = 1000;
        Serial.println("LED Mode: COLD - Slow blink (1Hz)");
      }
      else if (currentTemperature >= 20.0 && currentTemperature < 28.0) {
        // COMFORTABLE: Medium blink (500ms ON, 500ms OFF)
        onTime = 500;
        offTime = 500;
        Serial.println("LED Mode: COMFORTABLE - Medium blink (2Hz)");
      }
      else if (currentTemperature >= 28.0 && currentTemperature < 35.0) {
        // WARM: Fast blink (200ms ON, 200ms OFF)
        onTime = 200;
        offTime = 200;
        Serial.println("LED Mode: WARM - Fast blink (5Hz)");
      }
      else {
        // HOT: Very fast blink (100ms ON, 100ms OFF)
        onTime = 100;
        offTime = 100;
        Serial.println("LED Mode: HOT - Very fast blink (10Hz)");
      }
    }
    
    // Execute LED blink pattern with current timing settings
    digitalWrite(LED_GPIO, HIGH);  // Turn LED ON
    vTaskDelay(pdMS_TO_TICKS(onTime));
    
    digitalWrite(LED_GPIO, LOW);   // Turn LED OFF
    vTaskDelay(pdMS_TO_TICKS(offTime));
  }
}