
# **alastair-baronessalpha-fw**

https://github.com/user-attachments/assets/c5e2a554-1462-4827-9930-e5457603cde2

### Overview
The STM32 FDCAN Vehicle Gateway uses an STM32H7 microcontroller with dual FDCAN interfaces to acquire and process automotive sensor data. It supports inputs for RPM, speed, fuel level, coolant temperature, battery voltage, and various vehicle status signals. Processed data is transmitted over CAN for reliable instrumentation, monitoring, and diagnostics.

### Hardware
- MCU: STM32H723GZ (48 MHz HSE)
- FDCAN1: 25 kbps CAN bus (PUKK DISPLAY CAN)
- FDCAN2: 25 kbps CAN bus (BIKE CAN BUS)
- Interfaces: I2C1, USART1 (115200), TIM1/5, GPIO

### Features

- Tachometer - Engine RPM
- Speedometer - Vehicle speed
- Fuel Level Sensor - Tank level via ADS1115 ADC
- Coolant NTC - Temperature monitoring via ADS1115 ADC
- Battery voltage measurement

- Alert Signals

        1. Left Turn - Turn signal indicator
        2. Right Turn - Turn signal indicator
        3. Engine Temperature - Overheat warning
        4. Oil Temperature - Oil overheat warning
        5. H_Beam - High beam headlight status
        6. Oil Pressure - Low oil pressure warning

- Gear Signals

        1. Neutral
        2. Gear 1
        3. Gear 2
        4. Gear 3
        5. Gear 4
        6. Gear 5

### Debug
- #### ITM: 
    Real-time SWO debugging
- #### UART: 
    Console interface
- #### LED: 
    System status blink
    

### Status LED
-   **Normal Blink**: System is powered on and operating in default/idle mode.
    
-   **Fast Blink**: Active operation with CAN communication ongoing.
    
-   **Off**: System halted or fault detected.
### Build

- Open in STM32CubeIDE
- Build project
- Flash via ST-Link

### CAN Communication
The STM32 FDCAN Vehicle Gateway uses two independent CAN interfaces operating at 25 kbps for communication with separate vehicle networks:

**1. FDCAN1** – PUKK DISPLAY CAN (instrument cluster / display module)
**2. FDCAN2** – BIKE CAN BUS (main vehicle ECU / sensors)

#### FDCAN Configuration

- #### Frame Format: 
    Classic CAN, normal mode, 25 kbps
- #### Nominal Bit Timing: 
    Prescaler 120, SJW 1 tq, TSEG1 13 tq, TSEG2 2 tq
- #### Data Bit Timing: 
    Prescaler 1, no bit rate switching
- #### Receive Filters: 
    Standard ID filter accepts all IDs; global filter rejects remote frames
- #### Message RAM: 
    **FDCAN1:** Rx FIFO0 – *8×8 bytes*; Tx FIFO – *10×8 bytes*
    **FDCAN2**: Rx FIFO1 – *8×8 bytes*; Tx FIFO – *10×8 bytes*
- #### Error Handling: 
    Automatic retransmission enabled; protocol exception handling disabled

#### 1. FDCAN1 **(PUKK DISPLAY CAN)**
#### **Purpose**
Handles communication between the display module and the sensor/control logic. Supports transmitting sensor data to the display and receiving data from the bike via FDCAN2.
#### **Message Format (3 messages)**

#### **Message 1:** TX identifier = *0x101*
- *Byte 0*: Warning indicators (oil pressure, oil temperature, engine temperature, high beam status, left/right turn signals)
- *Byte 1*: Transmission gear flags (neutral position, gears 1-5 engagement status)
- *Bytes 2-3*: Battery voltage measurement in mV (16-bit unsigned integer)
- *Bytes 4-5*: Engine tachometer reading in RPM (16-bit unsigned integer)
- *Bytes 6-7*: Reserved for future implementation
  
**Example:** `00 00 F9 2E 00 00 00`

#### **Message 2:** TX identifier = *0x102*
- *Bytes 0-3*: Speed
- *Bytes 4-7*: Coolant Temperature in °C
  
**Example:** `00 00 00 00 50 93 C4`

#### **Message 3:** TX identifier = *0x103*
- *Bytes 0-3*: Fuel tank level percentage 0-100% (32-bit float)
- *Bytes 4-7*: Reserved for future implementation
  
**Example:** `34 15 52 42 00 00 00`

#### **Receive Example:** (from BIKE CAN BUS via FDCAN2)

    > Received Header Identifier:  512  
    > Received Data:  00 00 EC 2E 00 00 00 00  
    > Received Header Identifier:  513  
    > Received Data:  00 00 00 00 00 34 D5 41

---
#### 2. FDCAN2 **(BIKE CAN BUS)**
**Purpose**
Acts as the communication link between the Sensor Board and the Bike’s main CAN bus. Supports **both transmitting** mirrored data from FDCAN1 and **receiving** messages from the bike for processing or display.
#### **Message Format (2 messages example):** 

#### **Message 4:** TX identifier = *0x200*
- Forwarded version of FDCAN1 Message 1 (Indicators, Gears, Battery Volts, Tachometer)
**Example:** `00 00 F9 2E 00 00 00`

#### **Message 5:** TX identifier = *0x201*
- Forwarded version of FDCAN1 Message 2 (Speed, Coolant Temperature)
- Additional identifiers can be added for bike-specific messages.
**Example:** `00 00 00 00 50 93 C4`
#### **Receive Example:** (from PUKK DISPLAY CAN via FDCAN1)

		> Received Header Identifier:  259  
		> Received Data:  86 CF 55 42 00 00 00 00

### **Testing Summary**

-   Verified **SWV debug output** for both **transmission** and **reception** on **FDCAN1** and **FDCAN2**.
    
-   Confirmed correct **CAN message forwarding** between PUKK DISPLAY CAN and BIKE CAN BUS in both directions.
    
-   Sensor input validation:
    
    -   **Fuel level**
        
    -   **Coolant temperature**
        
    -   **Battery voltage**
    - Other sensors also implemented in firmware, including tachometer, speedometer, alert indicators, and gear position signals.
---

