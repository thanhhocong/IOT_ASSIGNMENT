# Task 1: Single LED Blink with Temperature Conditions

## Overview
This implementation creates a temperature-responsive LED system using FreeRTOS on ESP32. The LED blinks at different rates based on real-time temperature readings from a DHT20 sensor, with task synchronization managed through semaphores.

## System Architecture

### Tasks Involved
1. **`temp_humi_monitor`** (Producer Task)
   - Reads temperature and humidity from DHT20 sensor
   - Updates global temperature variable
   - Signals LED task via semaphore when new data is available
   - Runs every 5 seconds

2. **`led_blinky`** (Consumer Task)
   - Waits for temperature update notifications via semaphore
   - Adjusts LED blink pattern based on temperature range
   - Controls LED GPIO pin 48

## Temperature Ranges and LED Behaviors

The system implements **4 distinct temperature-responsive behaviors**:

| Temperature Range | Condition | LED Behavior | ON Time | OFF Time | Frequency |
|------------------|-----------|--------------|---------|----------|-----------|
| **< 20°C** | COLD | Slow blink | 1000ms | 1000ms | 0.5 Hz (1 blink every 2s) |
| **20°C - 28°C** | COMFORTABLE | Medium blink | 500ms | 500ms | 1 Hz (1 blink per second) |
| **28°C - 35°C** | WARM | Fast blink | 200ms | 200ms | 2.5 Hz (2.5 blinks per second) |
| **> 35°C** | HOT | Very fast blink | 100ms | 100ms | 5 Hz (5 blinks per second) |

### Behavior Rationale
- **COLD**: Slow blinking indicates a stable, cool environment
- **COMFORTABLE**: Normal blinking for optimal temperature range
- **WARM**: Faster blinking alerts to rising temperature
- **HOT**: Rapid blinking warns of critical high temperature

## Semaphore Synchronization

### Semaphore: `xTempUpdateSemaphore`
Type: **Binary Semaphore**

### Producer-Consumer Pattern

```
┌─────────────────────────┐         Semaphore          ┌─────────────────────────┐
│   temp_humi_monitor     │    xTempUpdateSemaphore    │      led_blinky         │
│      (Producer)         │                             │      (Consumer)         │
├─────────────────────────┤                             ├─────────────────────────┤
│                         │                             │                         │
│ 1. Read DHT20 sensor    │                             │ 1. Wait for semaphore   │
│ 2. Update glob_temp     │──────► Give Semaphore ─────►│    (with timeout)       │
│ 3. Give semaphore       │                             │ 2. Take semaphore       │
│ 4. Wait 5 seconds       │                             │ 3. Read glob_temp       │
│ 5. Repeat               │                             │ 4. Update LED pattern   │
│                         │                             │ 5. Blink LED            │
│                         │                             │ 6. Repeat               │
└─────────────────────────┘                             └─────────────────────────┘
```

### Synchronization Flow

1. **Initialization**
   ```cpp
   SemaphoreHandle_t xTempUpdateSemaphore = xSemaphoreCreateBinary();
   ```
   - Binary semaphore created at startup
   - Initially in "not available" state

2. **Temperature Task (Producer)**
   ```cpp
   // After successful sensor read
   glob_temperature = temperature;
   xSemaphoreGive(xTempUpdateSemaphore);  // Signal LED task
   ```
   - Reads sensor every 5 seconds
   - Updates global temperature variable
   - Gives semaphore to notify LED task

3. **LED Task (Consumer)**
   ```cpp
   // Wait for temperature update with 100ms timeout
   if (xSemaphoreTake(xTempUpdateSemaphore, pdMS_TO_TICKS(100)) == pdTRUE) {
       // Update LED behavior based on glob_temperature
   }
   ```
   - Waits for semaphore with timeout
   - Takes semaphore when available
   - Reads temperature and adjusts LED pattern

### Benefits of Semaphore Usage

1. **Event-Driven Communication**
   - LED task doesn't continuously poll temperature
   - Responds immediately to temperature updates
   - Reduces CPU usage

2. **Synchronization**
   - Ensures LED reads temperature only after it's updated
   - Prevents race conditions on shared variable
   - Clear producer-consumer relationship

3. **Timeout Protection**
   - 100ms timeout ensures LED continues blinking
   - System remains responsive even if sensor fails
   - Graceful degradation

4. **Task Coordination**
   - Explicit signaling between tasks
   - Clear data flow and dependencies
   - Easier to debug and maintain

## Implementation Details

### Global Variables
```cpp
extern float glob_temperature;           // Shared temperature data
extern SemaphoreHandle_t xTempUpdateSemaphore;  // Synchronization semaphore
```

### LED GPIO Configuration
- **GPIO Pin**: 48
- **Mode**: OUTPUT
- **Hardware**: Built-in LED on YoloUNO board

### Timing Configuration
- **Temperature Read Interval**: 5000ms (5 seconds)
- **Semaphore Timeout**: 100ms
- **LED Patterns**: Variable based on temperature (100ms - 1000ms)

## Code Structure

### Files Modified
1. **`include/global.h`**
   - Added `xTempUpdateSemaphore` declaration

2. **`src/global.cpp`**
   - Created and initialized semaphore

3. **`src/led_blinky.cpp`**
   - Complete rewrite with temperature-responsive logic
   - Semaphore-based synchronization
   - Four distinct blink patterns

4. **`src/temp_humi_monitor.cpp`**
   - Added semaphore signaling
   - Enhanced logging
   - Producer role in synchronization

## Testing and Verification

### Expected Behavior
1. **System Startup**
   ```
   LED Blinky Task Started - Temperature Responsive Mode
   Temperature/Humidity Monitor Task Started
   Sensor: DHT20
   Update interval: 5 seconds
   ```

2. **Temperature Update Cycle** (every 5 seconds)
   ```
   ----------------------------------------
   TEMP Task: Humidity: 45.2%  Temperature: 25.3°C
   TEMP Task: Semaphore given - LED task notified
   ----------------------------------------
   LED Task: Temperature update received: 25.3°C
   LED Mode: COMFORTABLE - Medium blink (2Hz)
   ```

3. **LED Blink Pattern Changes** (based on temperature)
   - Observe LED blink rate changing as temperature varies
   - Serial monitor shows mode transitions

### Serial Monitor Output
The implementation provides detailed logging:
- Temperature readings every 5 seconds
- Semaphore give/take operations
- LED mode transitions
- Current blink patterns

## Hardware Requirements
- **Board**: YoloUNO (ESP32-based)
- **Sensor**: DHT20 (I2C temperature/humidity sensor)
- **LED**: Built-in LED on GPIO 48
- **I2C Pins**: SDA=11, SCL=12

## Software Dependencies
- **Framework**: Arduino + FreeRTOS
- **Platform**: ESP32 (PlatformIO)
- **Libraries**:
  - DHT20 sensor library
  - FreeRTOS (included in ESP32 core)

## Key FreeRTOS Concepts Demonstrated

1. **Binary Semaphores**
   - Task synchronization primitive
   - Event notification mechanism

2. **Task Communication**
   - Producer-consumer pattern
   - Shared memory with synchronization

3. **Task Delays**
   - `vTaskDelay()` for non-blocking delays
   - Allows task scheduling

4. **Timeout Mechanisms**
   - `pdMS_TO_TICKS()` for time conversion
   - Graceful handling of missing signals

## Advantages of This Implementation

1. **Responsive**: LED reacts immediately to temperature changes
2. **Efficient**: Event-driven, not polling-based
3. **Robust**: Timeout ensures operation even if sensor fails
4. **Clear**: Well-documented code with extensive comments
5. **Scalable**: Easy to add more temperature ranges or behaviors
6. **Thread-Safe**: Proper synchronization prevents race conditions

## Potential Enhancements

1. Add mutex for temperature variable access
2. Implement moving average for temperature smoothing
3. Add more granular temperature ranges
4. Include RGB LED with color-coded temperature indication
5. Add LCD display showing current mode
6. Implement hysteresis to prevent rapid mode switching
7. Add configurable temperature thresholds via web interface

## Conclusion

This implementation successfully demonstrates:
- ✅ Multiple temperature-responsive LED behaviors (4 distinct patterns)
- ✅ Semaphore-based task synchronization
- ✅ Producer-consumer pattern in FreeRTOS
- ✅ Clear documentation and code comments
- ✅ Robust error handling and timeout mechanisms
- ✅ Real-time responsive system

The system provides an excellent foundation for understanding FreeRTOS task synchronization and real-world embedded systems programming.
