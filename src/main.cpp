#include "global.h"

#include "led_blinky.h"
#include "neo_blinky.h"
#include "temp_humi_monitor.h"
// #include "mainserver.h"
// #include "tinyml.h"
#include "coreiot.h"

// include task
#include "task_check_info.h"
#include "task_toogle_boot.h"
#include "task_wifi.h"
#include "task_webserver.h"
#include "task_core_iot.h"
#include "task_lcd_display.h"  // TASK 3: LCD Display with state management

void setup()
{
  Serial.begin(115200);
  
  Serial.println("\n\n");
  Serial.println("================================================================================");
  Serial.println("         IoT BTLIOT Project - FreeRTOS Tasks with Semaphore Sync");
  Serial.println("================================================================================");
  Serial.println("TASK 1: Temperature-Responsive LED (4 behaviors, binary semaphore)");
  Serial.println("TASK 2: Humidity-Responsive NeoPixel (5 colors, binary semaphore)");
  Serial.println("TASK 3: LCD Display (3 states, queue + mutex, NO GLOBALS)");
  Serial.println("================================================================================");
  Serial.println();
  
  check_info_File(0);

  // TASK 1: Temperature-responsive LED blink
  xTaskCreate(led_blinky, "Task LED Blink", 2048, NULL, 2, NULL);
  
  // TASK 2: Humidity-responsive NeoPixel colors
  xTaskCreate(neo_blinky, "Task NEO Blink", 2048, NULL, 2, NULL);
  
  // Sensor monitoring task (provides data to all consumer tasks)
  xTaskCreate(temp_humi_monitor, "Task TEMP HUMI Monitor", 2048, NULL, 2, NULL);
  
  // TASK 3: LCD Display with state management
  xTaskCreate(lcd_display_task, "Task LCD Display", 3072, NULL, 2, NULL);
  
  // Other tasks
  // xTaskCreate(main_server_task, "Task Main Server" ,8192  ,NULL  ,2 , NULL);
  // xTaskCreate( tiny_ml_task, "Tiny ML Task" ,2048  ,NULL  ,2 , NULL);
  xTaskCreate(coreiot_task, "CoreIOT Task" ,4096  ,NULL  ,2 , NULL);
  // xTaskCreate(Task_Toogle_BOOT, "Task_Toogle_BOOT", 4096, NULL, 2, NULL);
  
  Serial.println("All tasks created successfully!");
  Serial.println("System starting...\n");
}

void loop()
{
  if (check_info_File(1))
  {
    if (!Wifi_reconnect())
    {
      Webserver_stop();
    }
    else
    {
      //CORE_IOT_reconnect();
    }
  }
  Webserver_reconnect();
}