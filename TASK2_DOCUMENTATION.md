# Task 2: NeoPixel LED Control Based on Humidity

## Overview
This implementation creates a humidity-responsive NeoPixel RGB LED system using FreeRTOS on ESP32. The NeoPixel displays different colors with breathing effects based on real-time humidity readings from a DHT20 sensor, with task synchronization managed through binary semaphores.

## System Architecture

### Tasks Involved
1. **`temp_humi_monitor`** (Producer Task)
   - Reads temperature and humidity from DHT20 sensor
   - Updates global humidity variable
   - Signals NeoPixel task via semaphore when new data is available
   - Runs every 5 seconds

2. **`neo_blinky`** (Consumer Task)
   - Waits for humidity update notifications via semaphore
   - Adjusts RGB color based on humidity range
   - Applies breathing/pulsing visual effects
   - Controls NeoPixel on GPIO pin 45

## Humidity Ranges and Color Mapping

The system implements **5 distinct humidity-responsive color behaviors** (exceeds requirement of 3):

| Humidity Range | Condition | Color | RGB Values | Visual Effect | Meaning |
|----------------|-----------|-------|------------|---------------|---------|
| **< 30%** | DRY | ORANGE | (255, 165, 0) | Medium breathing | Air is too dry, possible discomfort |
| **30% - 40%** | LOW | YELLOW | (255, 255, 0) | Slow pulse | Slightly dry, acceptable |
| **40% - 60%** | COMFORTABLE | GREEN | (0, 255, 0) | Very slow pulse | Optimal humidity range |
| **60% - 70%** | HIGH | CYAN | (0, 255, 255) | Medium pulse | Higher humidity |
| **> 70%** | VERY HIGH | BLUE | (0, 0, 255) | Fast pulse (warning) | Too humid, potential mold risk |

### Visual Effects Details

Each humidity range has a unique breathing/pulsing pattern:

- **Breathing Effect**: Smooth brightness modulation from 50 to 255
- **Pulse Speed**: Varies by humidity level (faster = more critical)
- **Color Transitions**: Smooth and visually appealing

| Mode | Breath Direction | Breath Delay | Effect Speed |
|------|------------------|--------------|--------------|
| DRY | ±5 | 30ms | Medium |
| LOW | ±3 | 40ms | Slow |
| COMFORTABLE | ±2 | 50ms | Very Slow |
| HIGH | ±4 | 35ms | Medium |
| VERY HIGH | ±7 | 20ms | Fast (Warning) |

## Semaphore Synchronization

### Semaphore: `xHumidityUpdateSemaphore`
Type: **Binary Semaphore**

### Producer-Consumer Pattern

```
┌─────────────────────────┐         Semaphore            ┌─────────────────────────┐
│   temp_humi_monitor     │   xHumidityUpdateSemaphore   │      neo_blinky         │
│      (Producer)         │                               │      (Consumer)         │
├─────────────────────────┤                               ├─────────────────────────┤
│                         │                               │                         │
│ 1. Read DHT20 sensor    │                               │ 1. Wait for semaphore   │
│ 2. Update glob_humidity │──────► Give Semaphore ───────►│    (with timeout)       │
│ 3. Give semaphore       │                               │ 2. Take semaphore       │
│ 4. Wait 5 seconds       │                               │ 3. Read humidity        │
│ 5. Repeat               │                               │ 4. Update RGB color     │
│                         │                               │ 5. Apply breathing FX   │
│                         │                               │ 6. Repeat               │
└─────────────────────────┘                               └─────────────────────────┘
```

### Synchronization Flow

1. **Initialization**
   ```cpp
   SemaphoreHandle_t xHumidityUpdateSemaphore = xSemaphoreCreateBinary();
   ```
   - Binary semaphore created at startup
   - Initially in "not available" state

2. **Temperature Monitor Task (Producer)**
   ```cpp
   // After successful sensor read
   glob_humidity = humidity;
   xSemaphoreGive(xHumidityUpdateSemaphore);  // Signal NeoPixel task
   ```
   - Reads sensor every 5 seconds
   - Updates global humidity variable
   - Gives semaphore to notify NeoPixel task

3. **NeoPixel Task (Consumer)**
   ```cpp
   // Wait for humidity update with 100ms timeout
   if (xSemaphoreTake(xHumidityUpdateSemaphore, pdMS_TO_TICKS(100)) == pdTRUE) {
       currentHumidity = glob_humidity;
       // Update color based on humidity range
   }
   ```
   - Waits for semaphore with timeout
   - Takes semaphore when available
   - Reads humidity and adjusts RGB color
   - Continues breathing effect regardless

## Implementation Details

### Hardware Configuration
- **Board**: YoloUNO (ESP32)
- **NeoPixel GPIO**: Pin 45
- **LED Count**: 1 NeoPixel RGB LED
- **Sensor**: DHT20 (I2C)
- **I2C Pins**: SDA=11, SCL=12

### Color Calculation

The NeoPixel task uses a breathing effect algorithm:

```cpp
// Breathing effect calculation
brightness += breathDirection;

// Reverse at limits
if (brightness <= 50) breathDirection = abs(breathDirection);
else if (brightness >= 255) breathDirection = -abs(breathDirection);

// Apply brightness to base color
displayRed = (red * brightness) / 255;
displayGreen = (green * brightness) / 255;
displayBlue = (blue * brightness) / 255;
```

### Update Rates
- **Sensor Reading**: Every 5000ms (5 seconds)
- **Semaphore Timeout**: 100ms
- **Breathing Update**: 20-50ms (depending on mode)
- **Color Transition**: Immediate on humidity change

## Code Structure

### Files Modified/Created

| File | Purpose | Changes |
|------|---------|---------|
| `include/global.h` | Global declarations | Added `xHumidityUpdateSemaphore` |
| `src/global.cpp` | Global definitions | Created and initialized semaphore |
| `include/neo_blinky.h` | NeoPixel header | Added global.h include |
| `src/neo_blinky.cpp` | NeoPixel control | Complete rewrite with 5 humidity colors |
| `src/temp_humi_monitor.cpp` | Temperature sensing | Added humidity semaphore signaling |

## Benefits of This Design

1. **Event-Driven**: NeoPixel responds immediately to humidity changes
2. **Visually Informative**: 5 distinct colors provide clear environmental feedback
3. **Smooth Animation**: Breathing effects create professional appearance
4. **Efficient**: Event-based, not continuous polling
5. **Fault Tolerant**: Continues operating with default color if sensor fails
6. **Clear Feedback**: Different pulse speeds indicate urgency

## Testing and Verification

### Expected Serial Output

```
========================================
TASK 2: NeoPixel Humidity Monitor Started
Humidity-Color Mapping:
  DRY      (< 30%):   ORANGE (255,165,0)
  LOW      (30-40%):  YELLOW (255,255,0)
  COMFORT  (40-60%):  GREEN  (0,255,0)
  HIGH     (60-70%):  CYAN   (0,255,255)
  VERY HIGH (> 70%):  BLUE   (0,0,255)
========================================
----------------------------------------
NEO Task: Humidity update received: 45.5%
NEO Mode: COMFORTABLE - Green (steady)
NEO Task: RGB Color = (0, 255, 0)
----------------------------------------
```

### Visual Verification

1. **DRY (< 30%)**: Orange breathing - medium speed
2. **LOW (30-40%)**: Yellow slow pulse
3. **COMFORTABLE (40-60%)**: Green very slow pulse
4. **HIGH (60-70%)**: Cyan medium pulse
5. **VERY HIGH (> 70%)**: Blue fast pulse (warning)

### Testing Steps

1. Start system and observe default green color
2. Place sensor in dry environment (desiccant) → Orange
3. Normal room humidity → Green
4. Breathe on sensor or use humidifier → Cyan or Blue
5. Monitor serial output for synchronization messages

## Key FreeRTOS Concepts Demonstrated

1. **Binary Semaphores**: Event notification between tasks
2. **Producer-Consumer Pattern**: Clear data flow
3. **Task Synchronization**: Coordinated behavior
4. **Timeout Mechanisms**: Graceful degradation
5. **Periodic Tasks**: Regular sensor readings

## Color Psychology

The chosen colors follow intuitive environmental indicators:

- **Orange**: Warm/dry, alerts to dry conditions
- **Yellow**: Caution, slightly dry
- **Green**: Optimal, all is well
- **Cyan**: Cool/moist, higher humidity
- **Blue**: Water/wet, too much humidity

## Advantages

✅ **Visual Clarity**: Anyone can understand humidity at a glance  
✅ **Exceeds Requirements**: 5 levels instead of minimum 3  
✅ **Professional Effects**: Breathing animation adds polish  
✅ **Immediate Response**: Event-driven updates  
✅ **Low CPU Usage**: Efficient semaphore-based communication  
✅ **Robust**: Continues operation even if sensor fails  

## Potential Enhancements

1. Add RGB LED ring for more dramatic effects
2. Implement color transition animations
3. Add configurable humidity thresholds via web interface
4. Include sound alerts at critical levels
5. Log humidity history with color indicators
6. Add multiple NeoPixels showing humidity trend
7. Implement rainbow effects for system boot

## Conclusion

Task 2 successfully demonstrates:
- ✅ Multiple humidity-responsive colors (5 levels, exceeds 3)
- ✅ Semaphore-based synchronization
- ✅ Producer-consumer pattern
- ✅ Professional visual effects
- ✅ Clear documentation and logging

The NeoPixel provides an intuitive, visually appealing way to monitor humidity levels at a glance, making environmental monitoring both functional and aesthetic.

