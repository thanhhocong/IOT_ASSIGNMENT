# Tasks 2 & 3: Quick Start Guide

## What Was Implemented

### Task 2: NeoPixel Humidity Monitor ✅
- **5 humidity-responsive colors** (Orange, Yellow, Green, Cyan, Blue)
- **Binary semaphore synchronization** between sensor and NeoPixel
- **Breathing/pulsing effects** for visual appeal
- **Clear humidity-color mapping** documented

### Task 3: LCD Display with Queue Communication ✅
- **3 display states** (NORMAL, WARNING, CRITICAL)
- **Eliminated ALL global variables** using FreeRTOS queues
- **Mutex semaphore** for state protection
- **Queue-based data transfer** replaces global variable access

---

## Build and Upload

```bash
# Navigate to project directory
cd YoloUNO_PlatformIO_RTOS_Project

# Build project
pio run

# Upload to board
pio run --target upload

# Open serial monitor
pio device monitor
# Or press Ctrl+Shift+M in VS Code
```

---

## What to Expect

### NeoPixel Colors (Task 2)

Watch the NeoPixel RGB LED change colors:

| Humidity | Color | Visual |
|----------|-------|--------|
| Dry (< 30%) | Orange | Medium pulsing |
| Low (30-40%) | Yellow | Slow pulsing |
| **Comfortable (40-60%)** | **Green** | Very slow (optimal) |
| High (60-70%) | Cyan | Medium pulsing |
| Very High (> 70%) | Blue | Fast pulsing (warning!) |

**Test it:**
- Normal room: Green
- Breathe on sensor: Cyan → Blue
- Dry environment: Orange

### LCD Display (Task 3)

Watch the LCD show different states:

**NORMAL (18-28°C, 40-60% humidity):**
```
OK 23.5C 45%
Status: NORMAL
```

**WARNING (approaching limits):**
```
!W 29.0C 35%
**  WARNING  **
```
*Alternating flash*

**CRITICAL (outside safe range):**
```
!!35.0C 75%!!
*** CRITICAL ***
```
*Fast flashing*

**Test it:**
- Room temperature: NORMAL
- Heat sensor with hand: WARNING
- Very hot/cold or humid: CRITICAL

### Serial Monitor Output

You should see:
```
================================================================================
         IoT BTLIOT Project - FreeRTOS Tasks with Semaphore Sync
================================================================================
TASK 1: Temperature-Responsive LED (4 behaviors, binary semaphore)
TASK 2: Humidity-Responsive NeoPixel (5 colors, binary semaphore)
TASK 3: LCD Display (3 states, queue + mutex, NO GLOBALS)
================================================================================

[Task initialization messages...]

----------------------------------------
TEMP Task: Humidity: 45.2%  Temperature: 23.5°C
TEMP Task: Temperature semaphore given - LED task notified
TEMP Task: Humidity semaphore given - NeoPixel task notified
TEMP Task: Sensor data sent to queue
----------------------------------------

NEO Task: Humidity update received: 45.2%
NEO Mode: COMFORTABLE - Green (steady)

>>> LCD Task: Data received from queue <<<
    Temperature: 23.5°C, Humidity: 45.2%
LCD Display: NORMAL mode - All values optimal
```

---

## Quick Testing Checklist

### Task 2: NeoPixel
- [ ] NeoPixel shows green color at startup
- [ ] Color changes when you breathe on sensor (humidity increases)
- [ ] Serial shows "NEO Mode: COMFORTABLE" etc.
- [ ] Breathing/pulsing effect visible
- [ ] Serial shows "Humidity semaphore given"

### Task 3: LCD Display
- [ ] LCD shows "LCD Task Ready" at startup
- [ ] LCD updates with temperature and humidity values
- [ ] Display changes to WARNING when values approach limits
- [ ] Display shows CRITICAL when values extreme
- [ ] Serial shows "Data received from queue" (NOT direct global access!)
- [ ] Serial shows "STATE CHANGE" messages

### Task 3: Global Elimination Verification
- [ ] Serial shows "Sensor data sent to queue"
- [ ] Serial shows "Data received from queue"
- [ ] No direct "glob_temperature" access in LCD task logs
- [ ] System runs stable for 10+ minutes

---

## Troubleshooting

### NeoPixel Issues

**Problem:** NeoPixel not lit
- Check GPIO 45 connection
- Verify power to NeoPixel
- Check serial for "TASK 2: NeoPixel Humidity Monitor Started"

**Problem:** Color not changing
- Verify sensor is reading humidity correctly
- Check serial for "Humidity semaphore given"
- Check for "NEO Task: Humidity update received"

### LCD Issues

**Problem:** LCD blank
- Check I2C address (default 0x27)
- Verify I2C connections (SDA/SCL)
- Try different I2C address in code if needed

**Problem:** LCD shows "Waiting data..."
- Sensor not reading properly
- Check serial for queue messages
- Verify queue creation succeeded

**Problem:** Display not changing states
- Values might be in NORMAL range
- Try heating/cooling sensor
- Check state change thresholds in code

### Queue Issues

**Problem:** "Queue full" warnings
- LCD task not consuming fast enough
- Increase queue size (currently 5)
- Check LCD task is running

**Problem:** No "Data received from queue"
- Queue not created properly
- Check xQueueCreate() success
- Verify sender and receiver using same queue handle

---

## Files You Need to Know

### Source Code
- `src/neo_blinky.cpp` - Task 2 implementation
- `src/task_lcd_display.cpp` - Task 3 implementation
- `src/temp_humi_monitor.cpp` - Sensor + Queue sender
- `include/global.h` - Queue and semaphore declarations
- `src/global.cpp` - Queue and semaphore creation
- `src/main.cpp` - Task creation

### Documentation
- `TASK2_DOCUMENTATION.md` - Complete Task 2 explanation
- `TASK3_DOCUMENTATION.md` - Complete Task 3 explanation
- `TASKS_2_3_SUMMARY.md` - Combined summary and overview
- `TASKS_2_3_QUICK_START.md` - This guide

---

## For Your Report

### Task 2 Summary

**Implemented:**
- 5 humidity-responsive NeoPixel colors (exceeds 3 minimum)
- Binary semaphore `xHumidityUpdateSemaphore` for synchronization
- Producer-consumer pattern between sensor and NeoPixel tasks

**Humidity-Color Mapping:**
1. DRY (< 30%): Orange
2. LOW (30-40%): Yellow
3. COMFORTABLE (40-60%): Green ✅
4. HIGH (60-70%): Cyan
5. VERY HIGH (> 70%): Blue ⚠️

**Semaphore Usage:**
- Sensor task gives semaphore after reading humidity
- NeoPixel task takes semaphore and updates color
- Ensures immediate response to humidity changes

### Task 3 Summary

**Implemented:**
- 3 LCD display states: NORMAL, WARNING, CRITICAL
- Eliminated ALL global variables using FreeRTOS queue
- Mutex semaphore protects display state transitions

**Global Variable Elimination:**
- **Before:** `glob_temperature` and `glob_humidity` accessed directly
- **After:** Data sent via `xSensorDataQueue`
- **Result:** Thread-safe, no race conditions

**Semaphore Conditions:**

*Creating/Giving Semaphores:*
1. When sensor reads valid data → Give binary semaphores
2. When queue receives new data → Signal consumers
3. When display state changes → Give mutex (after taking)

*Taking Semaphores:*
1. Consumer tasks take binary semaphores to wait for data
2. LCD task takes mutex before changing display state
3. All with timeouts for fault tolerance

**Display States:**
1. **NORMAL:** Temp 18-28°C AND Humidity 40-60%
2. **WARNING:** Approaching limits
3. **CRITICAL:** Outside safe ranges

---

## Key Differences from Tasks 1-2

| Feature | Tasks 1 & 2 | Tasks 2 & 3 |
|---------|-------------|-------------|
| **Data Transfer** | Direct global access | Queue-based transfer |
| **Synchronization** | Binary semaphore only | Queue + Mutex + Binary |
| **Safety** | Basic | Thread-safe atomic ops |
| **Best Practice** | Good | Production-ready |

---

## Success Criteria

✅ Task 2:
- [ ] NeoPixel shows at least 3 different colors for humidity levels
- [ ] Semaphore synchronization working
- [ ] Serial shows semaphore give/take messages
- [ ] Code is well-commented

✅ Task 3:
- [ ] LCD shows 3 distinct display states
- [ ] Queue used for data transfer (verify in serial)
- [ ] Global variables NOT accessed directly by LCD task
- [ ] Mutex protects state changes
- [ ] System stable and responsive

---

## Next Steps

1. **Test thoroughly** - Try all humidity and temperature ranges
2. **Capture screenshots** - Serial output and LCD display
3. **Document observations** - Note state transitions
4. **Review code** - Ensure you understand all concepts
5. **Prepare demo** - Show working system to instructor

---

## Important Notes

⚠️ **LCD I2C Address:** If LCD doesn't work, try address 0x3F instead of 0x27
⚠️ **Sensor Warm-up:** DHT20 may need 1-2 minutes to stabilize
⚠️ **Serial Baud Rate:** Must be 115200
⚠️ **Power:** Ensure stable USB power supply

---

## Quick Reference Commands

```bash
# Build
pio run

# Upload
pio run -t upload

# Clean
pio run -t clean

# Monitor
pio device monitor --baud 115200
```

---

**Everything is ready for testing and demonstration!** 🚀

For detailed technical explanations, see:
- `TASK2_DOCUMENTATION.md`
- `TASK3_DOCUMENTATION.md`
- `TASKS_2_3_SUMMARY.md`

