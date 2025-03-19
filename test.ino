#include "bsp.h"
#include "siliqs_esp32.h"
#include "esp_sleep.h"
void setup()
{
  siliqs_esp32_setup(SQ_INFO);
  // Enter deep sleep for 300 seconds (5 minutes) to verify RTC slow clock functionality
  esp_sleep_enable_timer_wakeup(300 * 1000000); // Wake up after 5 minutes
  Serial.println("Entering deep sleep...");
  esp_deep_sleep_start();
}

void loop()
{
  // Not used
}
