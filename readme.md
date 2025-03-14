**Product Development Plan: Remote Sensor with Communication Function**

## **1. Introduction**

The product is a remote sensor equipped with multiple communication functions. It is designed for security and monitoring applications, particularly in controlled zones where BLE beacons are used to trigger security alerts. The system communicates through BLE and LoRaWAN and integrates a reed switch, a buzzer, and a 3-color LED for alarm indications.

## **2. Features and Functionality**

### **2.1 Communication Capabilities**

- **BLE (Bluetooth Low Energy):** Used to receive beacon signals and detect other BLE sensors within range.
- **LoRaWAN:** Used for long-range wireless communication with a central server for event reporting and status updates.

### **2.2 Sensor Components**

- **Reed Switch:** Detects state changes; must implement debounce logic to prevent false triggers.
- **Buzzer:** Emits an alarm sound when triggered.
- **3-Color LED:** Provides visual status indication (Red for alerts, other colors for system states).

## **3. System Behavior**

### **3.1 Control Zone Security Process**

- When the device enters a control zone and detects a BLE beacon, it initiates a security process.
- The **buzzer turns on**, and the **LED turns Red**.
- An **emergency message** is immediately sent to the LoRaWAN network.

### **3.2 Reed Switch Activation**

- The device can receive alert messages from another BLE sensor in the area only when the system is in security mode (dangerous zone).
- If the reed switch state changes to HIGH (activated), the device must implement a toggle control to filter out false signals.
- If confirmed, the device follows the same security alert process:
  - **Buzzer turns on**
  - **LED turns Red**
  - **Emergency message is sent to LoRaWAN**

### **3.3 BLE Sensor Alarm Reception**

- The device can receive alert messages from another BLE sensor in the area **only when the system is in security mode (dangerous zone).**
- If another BLE sensor sends an alarm message:
  - **Buzzer turns on**
  - **LED turns Red**
  - **Emergency message is sent to LoRaWAN**

### **3.4 Periodic System Reporting**

- The system must periodically report its status to the LoRaWAN server.
- The default report interval is **5 minutes**, with a minimum configurable interval of **1 minute** and a maximum of **60 minutes**.

## **4. Technical Requirements**

- **HTCT62 microcontroller with BLE & LoRaWAN support, developed on the Arduino platform**
- **Low-power design for extended battery life, with a sleep mode current consumption under 100µA and optimized to meet the target battery life and capacity requirements.**
- **Firmware capable of handling event-driven tasks efficiently, utilizing FreeRTOS for multitasking and real-time control on ESP32. BLE scan cycle and sleep time intervals need careful tuning for optimal performance.**
- **LoRaWAN supports encapsulated data communication. We will design a communication format for both normal and emergency messages to be sent to the server securely.**
- **Configurable parameters for report intervals and sensor thresholds, using a Web-BLE app for setup.**

## **5. System Control Flow**

### **5.1 Device Initialization**

- Power on and initialize BLE, LoRaWAN, and peripherals.
- Enter low-power sleep mode while waiting for events.

### **5.2 BLE Beacon Detection**

- Scan periodically for control zone BLE beacons.
- If detected, transition to security mode.
- Activate buzzer and red LED, send emergency message via LoRaWAN if the reed switch turns HIGH (with debounce logic) or if an alert message is received from another BLE sensor.

### **5.3 Reed Switch Monitoring**

- Continuously monitor reed switch state **only when the system is in security mode (dangerous zone).**
- Implement debounce mechanism to avoid false triggers.
- If confirmed HIGH, activate buzzer and LED, send emergency message.

### **5.4 BLE Sensor Alarm Reception**

- Continuously monitor BLE Sensor Alarm **only when the system is in security mode (dangerous zone).**
- Listen for alert messages from nearby BLE sensors.
- If an alert is received, trigger security mode immediately.

### **5.5 Periodic and Emergency Status Reporting**

- Report system status to LoRaWAN server at configurable intervals (1-60 min).
- Include battery status, nearby readable beacon (biggest RSSI) information, and sensor status.
- Data format for uplink
   - **Data format: Total 40 bytes per packet (report data)**
     ```c
         struct battery_data // 2 bytes
        {
          uint8_t power_mode;                               // 0: normal , 1: power saving , 2: charging(under test if it could be detect or not)  ,1 byte 
          uint8_t battery_level;                            // 0-255 ,1 byte 
        }
        
        struct peripheral_hook_sensor_data // 12 bytes (aligned)
        {
            uint8_t hook_id[6];
            uint8_t mode;
            uint8_t status;
            uint16_t reserved;
            struct battery_data hook_battery;
        };
        
        struct reed_switch_data // 2 bytes
        {
          uint8_t mode;                                     // 0: normal 1: un-know ( if device is not in dangerous zone, we may by detect reed switch ,it should be un-know)
          uint8_t status;                                   // 0: normal 1: alarm
        }
        
        struct peripheral_sensor_status // 26 bytes
        {
            struct peripheral_hook_sensor_data hooks[2]; // 24 bytes
            struct reed_switch_data reed_switch; // 2 bytes
        };

        struct peripheral_readable_beacon_data // 16 bytes
        {
          uint8_t uuid[8]; // 8 bytes
          uint16_t rssi; // 2 bytes
          uint16_t major; // 2 bytes
          uint16_t minor; // 2 bytes
          uint16_t reserved; // fill 0, 2 bytes
        }
       
        struct report_data // 46 bytes
        {
            struct battery_data battery_status; // 2 bytes
            struct peripheral_readable_beacon_data biggest_rssi_readable_in_past_one_minutes_beacon_data; // 16 bytes
            struct peripheral_sensor_status sensor_status; // 26 bytes
        };
     ```


### **5.6 Low-Power Optimization**

   - Enter sleep mode when no active event is detected.
   - BLE scan interval is set to 1 minute, with a scan duration ranging from 5 to 30 seconds.
   - LoRaWAN status update occurs once every 5 minutes by default, with a configurable interval ranging from 1 to 60 minutes.
   - Trigger emergency notifications when an event occurs, such as a hook alarm or a reed switch activation.

## **6. Hardware Specifications**

- **Size:** Minimum dimensions of 40mm x 26mm x 5mm (excluding battery)
- **External Power Connector:** MX 1.25
- **Buzzer:** 85 dB without case cover
- **Power Consumption:** Deep sleep current under 100µA
- **LED:** 3-color LED (optional)
- **Reed Switch:** 5mm distance detection without magnetic guide

## **7. Firmware Structure**

- **Develop firmware with event-driven logic for BLE and LoRaWAN communication.**
- **Use Siliqs library:** [GitHub - livinghuang/siliqs_esp32](https://github.com/livinghuang/siliqs_esp32) to implement the system.
- **Main loop behavior:**
  - Wakes up every 60 seconds to check for required operations.
  - If no periodic job is needed, it returns to sleep mode to optimize power consumption.
- **Reed switch behavior:**
  - Calls an interrupt to send emergency events.
  - Implements a robust algorithm for filtering false reed switch triggers.
- **Entering setting mode:**
  - On power-on, opens a **10-second window** for the BLE Web App to configure parameters.
- **Security process testing:**
  - Validate BLE beacon detection and emergency alerts.

## **8. Pseudocode**

```c
// Define system states
enum SystemState {
    SLEEP_MODE,
    BLE_SCAN_MODE,
    UPLINK_MODE
};

struct battery_data // 2 bytes
{
  uint8_t power_mode;    // 0: normal , 1: power saving , 2: charging(under test if it could be detect or not)  ,1 byte
  uint8_t battery_level; // 0-255 ,1 byte
};

struct peripheral_readable_beacon_data // 16 bytes
{
  uint8_t uuid[8];   // 8 bytes
  uint16_t rssi;     // 2 bytes
  uint16_t major;    // 2 bytes
  uint16_t minor;    // 2 bytes
  uint16_t reserved; // fill 0, 2 bytes
};

struct peripheral_hook_sensor_data // 12 bytes (aligned)
{
  uint8_t hook_id[6];
  uint8_t mode;   // 0: normal 1: un-know ( if device is not in dangerous zone, we may by detect reed switch ,it should be un-know)
  uint8_t status; // 0: normal 1: alarm
  uint16_t reserved;
  struct battery_data hook_battery;
};

struct reed_switch_data // 2 bytes
{
  uint8_t mode;   // 0: normal 1: un-know ( if device is not in dangerous zone, we may by detect reed switch ,it should be un-know)
  uint8_t status; // 0: normal 1: alarm
};

struct peripheral_sensor_status // 26 bytes
{
  struct peripheral_hook_sensor_data hooks[2]; // 24 bytes
  struct reed_switch_data reed_switch;         // 2 bytes
};

struct report_data // 46 bytes
{
  struct battery_data battery_status;                                                           // 2 bytes
  struct peripheral_readable_beacon_data biggest_rssi_readable_in_past_one_minutes_beacon_data; // 16 bytes
  struct peripheral_sensor_status sensor_status;                                                // 26 bytes
};

// Initialize variables
SystemState currentState = SLEEP_MODE;
bool isDangerousZone = false;
bool isHookDetected = false;

struct report_data global_report_data;
void update_report_data(struct battery_data battery) {
  global_report_data.battery_status=battery;
}
void update_report_data(struct peripheral_readable_beacon_data beacon_data) {
  global_report_data.biggest_rssi_readable_in_past_one_minutes_beacon_data=beacon_data;
}
void update_report_data(struct peripheral_hook_sensor_data hook_data[2]) {
  global_report_data.sensor_status.peripheral_sensor_status[2]=hook_data[2];
}
void update_report_data(struct reed_switch_data reed_switch) {
  global_report_data.sensor_status.reed_switch=reed_switch;
}


// Function to enable GPIO interrupt for the reed switch
void enableReedSwitchInterrupt() {
    attachInterrupt(REED_SWITCH_PIN, onReedSwitchTrigger, RISING);  // Enable interrupt on HIGH
}

// Function to disable GPIO interrupt for the reed switch
void disableReedSwitchInterrupt() {
    detachInterrupt(REED_SWITCH_PIN);  // Disable interrupt
}

// Function to handle timer event (wakes up for BLE scan)
void onTimerEvent() {
    wakeUp();
    currentState = BLE_SCAN_MODE;
}

// Function to handle reed switch GPIO interrupt
void onReedSwitchTrigger() {
    wakeUp();
    debounceReedSwitch();
}

// Function to debounce reed switch
void debounceReedSwitch() {
    delay(50);  // Simple debounce logic
    if (readGPIO(REED_SWITCH_PIN) == HIGH) {
        update_report_data(set_reed_switch_alarm);
        currentState = UPLINK_MODE;
    }else{
      currentState = SLEEP_MODE;
    }
}

// Combined BLE beacon scan function
void scanBLEBeaconsAndCheckHook() {
    BeaconData beacon = scanForBLEBeacon(); 
    update_report_data(beacon);

    // Check if the device is in a dangerous zone
    if (beacon.isDangerousZone) {
        isDangerousZone = true;
      
        change_period_wakeup_time_to_10sec();
        turn_on_index_led(Green);  // Ensure LED is turned ON instead of toggling
      
        // Enable Reed Switch Interrupt only in Dangerous Zone
        enableReedSwitchInterrupt();

        // Now scan for the hook BLE beacon and validate ID
        HookBeaconData hookBeacons[2] = {0};
        bool foundValidHooks = beacon.searchBeaconPoolforHook(matchID, hookBeacons);

        update_report_data(hookBeacons); // Update the status here.
        
        if (foundValidHooks) {
            if (!hookBeacons[0].isDetected || !hookBeacons[1].isDetected) {
                currentState = EMERGENCY_ALERT;  // At least one hook missing, send emergency alert
            } else if (hookBeacons[0].status == HOOK_ALARM || hookBeacons[1].status == HOOK_ALARM) {
                currentState = EMERGENCY_ALERT;  // Hook triggered an alarm
            } else {
                currentState = SLEEP_MODE;
            }
        } else {
            currentState = EMERGENCY_ALERT;  // No valid hook beacon found
        }
    } else {
        isDangerousZone = false;

        change_period_wakeup_time_to_60sec();
        turn_off_index_led();  // Ensure LED is OFF when leaving the zone
      
        // Disable Reed Switch Interrupt outside Dangerous Zone
        disableReedSwitchInterrupt();

        currentState = SLEEP_MODE;
    }
}

// Function to handle emergency alert
void triggerEmergencyAlert() {
    activateBuzzer();
    turnOnRedLED();
    sendLoRaWANMessage("EMERGENCY_ALERT");

    // After emergency, disable reed switch interrupt to prevent repeated triggers
    disableReedSwitchInterrupt();
}

// Main event-driven loop
void loop() {
    check_battery_power(battery);
    update_report_data(battery);

    if (power_mode == POWER_SAVING) {  // Power under 3.1V
        enterSleepMode();
        return;  // Prevent further processing
    }

    switch (currentState) {
        case SLEEP_MODE:
            enterSleepMode();
            break;

        case BLE_SCAN_MODE:
            scanBLEBeaconsAndCheckHook();
            break;

        case UPLINK_MODE:
            uplinkDataMode();
            currentState = SLEEP_MODE;
            break;

        case EMERGENCY_ALERT:
            triggerEmergencyAlert();
            break;
    }
}


// Interrupt service routines
ISR(TIMER_INTERRUPT) {
    onTimerEvent();
}

ISR(GPIO_INTERRUPT) {
    onReedSwitchTrigger();
}
```

## **9. Conclusion**

This remote sensor system is designed to enhance security by detecting control zone entry, monitoring reed switch activations, and processing alerts from other BLE sensors. With BLE and LoRaWAN communication, it ensures immediate alarm reporting and periodic system updates, making it suitable for various security applications.

