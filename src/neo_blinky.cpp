#include "neo_blinky.h"

/**
 * @brief TASK 2: NeoPixel LED Control Based on Humidity Levels
 * 
 * This task implements a humidity-responsive RGB LED system using NeoPixel.
 * The LED displays different colors based on current humidity readings.
 * 
 * HUMIDITY RANGES AND COLOR MAPPING:
 * 
 * 1. DRY (< 30%):
 *    - Color: ORANGE (255, 165, 0)
 *    - Indicates very dry air
 *    - Breathing effect (pulsing brightness)
 * 
 * 2. LOW (30% - 40%):
 *    - Color: YELLOW (255, 255, 0)
 *    - Indicates low humidity, slightly dry
 *    - Steady glow with slight pulse
 * 
 * 3. COMFORTABLE (40% - 60%):
 *    - Color: GREEN (0, 255, 0)
 *    - Indicates optimal humidity range
 *    - Steady, calm glow
 * 
 * 4. HIGH (60% - 70%):
 *    - Color: CYAN (0, 255, 255)
 *    - Indicates high humidity
 *    - Steady with medium pulse
 * 
 * 5. VERY HIGH (> 70%):
 *    - Color: BLUE (0, 0, 255)
 *    - Indicates very high humidity (potential mold risk)
 *    - Fast pulsing to indicate warning
 * 
 * SEMAPHORE SYNCHRONIZATION:
 * - Uses xHumidityUpdateSemaphore to synchronize with temp_humi_monitor task
 * - When humidity is read and updated, the semaphore is given
 * - NeoPixel task takes the semaphore and updates color accordingly
 * - Timeout mechanism ensures LED continues displaying even if no updates
 * 
 * VISUAL EFFECTS:
 * - Breathing/pulsing effects for different humidity levels
 * - Color transitions are smooth and visually appealing
 * - Brightness modulation indicates urgency/comfort level
 */

void neo_blinky(void *pvParameters){

    // Initialize NeoPixel strip
    Adafruit_NeoPixel strip(LED_COUNT, NEO_PIN, NEO_GRB + NEO_KHZ800);
    strip.begin();
    strip.clear();
    strip.show();
    
    Serial.println("========================================");
    Serial.println("TASK 2: NeoPixel Humidity Monitor Started");
    Serial.println("Humidity-Color Mapping:");
    Serial.println("  DRY      (< 30%):   ORANGE (255,165,0)");
    Serial.println("  LOW      (30-40%):  YELLOW (255,255,0)");
    Serial.println("  COMFORT  (40-60%):  GREEN  (0,255,0)");
    Serial.println("  HIGH     (60-70%):  CYAN   (0,255,255)");
    Serial.println("  VERY HIGH (> 70%):  BLUE   (0,0,255)");
    Serial.println("========================================");

    // Current color values
    uint8_t red = 0, green = 255, blue = 0;  // Default: Green
    
    // Breathing effect variables
    uint8_t brightness = 255;
    int8_t breathDirection = -5;  // Breathing speed and direction
    uint16_t breathDelay = 30;    // Delay between brightness steps
    
    // TASK 3: Local variable for humidity (NO GLOBAL ACCESS except when signaled)
    float currentHumidity = 50.0;  // Default value
    
    while(1) {
        // Wait for humidity update semaphore with timeout
        if (xSemaphoreTake(xHumidityUpdateSemaphore, pdMS_TO_TICKS(100)) == pdTRUE) {
            // Semaphore acquired - new humidity data available
            // TASK 3: Read from global only when signaled (synchronized access)
            currentHumidity = glob_humidity;
            float humidity = currentHumidity;
            
            Serial.println("----------------------------------------");
            Serial.print("NEO Task: Humidity update received: ");
            Serial.print(humidity);
            Serial.println("%");
            
            // Determine color and breathing effect based on humidity range
            if (humidity < 30.0) {
                // DRY: Orange with breathing effect
                red = 255;
                green = 165;
                blue = 0;
                breathDirection = -5;
                breathDelay = 30;
                Serial.println("NEO Mode: DRY - Orange (breathing)");
            }
            else if (humidity >= 30.0 && humidity < 40.0) {
                // LOW: Yellow with slight pulse
                red = 255;
                green = 255;
                blue = 0;
                breathDirection = -3;
                breathDelay = 40;
                Serial.println("NEO Mode: LOW - Yellow (slow pulse)");
            }
            else if (humidity >= 40.0 && humidity < 60.0) {
                // COMFORTABLE: Green steady glow
                red = 0;
                green = 255;
                blue = 0;
                breathDirection = -2;
                breathDelay = 50;
                Serial.println("NEO Mode: COMFORTABLE - Green (steady)");
            }
            else if (humidity >= 60.0 && humidity < 70.0) {
                // HIGH: Cyan with medium pulse
                red = 0;
                green = 255;
                blue = 255;
                breathDirection = -4;
                breathDelay = 35;
                Serial.println("NEO Mode: HIGH - Cyan (medium pulse)");
            }
            else {
                // VERY HIGH: Blue with fast pulse (warning)
                red = 0;
                green = 0;
                blue = 255;
                breathDirection = -7;
                breathDelay = 20;
                Serial.println("NEO Mode: VERY HIGH - Blue (fast pulse WARNING)");
            }
            
            Serial.print("NEO Task: RGB Color = (");
            Serial.print(red);
            Serial.print(", ");
            Serial.print(green);
            Serial.print(", ");
            Serial.print(blue);
            Serial.println(")");
            Serial.println("----------------------------------------");
        }
        
        // Apply breathing effect to current color
        brightness += breathDirection;
        
        // Reverse direction at brightness limits
        if (brightness <= 50) {
            breathDirection = abs(breathDirection);  // Start increasing
        } else if (brightness >= 255) {
            breathDirection = -abs(breathDirection); // Start decreasing
        }
        
        // Calculate color with brightness adjustment
        uint8_t displayRed = (red * brightness) / 255;
        uint8_t displayGreen = (green * brightness) / 255;
        uint8_t displayBlue = (blue * brightness) / 255;
        
        // Update NeoPixel display
        strip.setPixelColor(0, strip.Color(displayRed, displayGreen, displayBlue));
        strip.show();
        
        // Wait before next brightness update
        vTaskDelay(pdMS_TO_TICKS(breathDelay));
    }
}