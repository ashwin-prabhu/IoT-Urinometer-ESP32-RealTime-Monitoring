# Smart Urinometer for AKI Detection (ESP32-Based System)

This project implements a real-time urine output monitoring system intended for ICU use. It measures urine volume using a load cell and computes flow rate continuously to assist in the early detection of Acute Kidney Injury (AKI).

The system is built around an ESP32 microcontroller and provides both local display and remote monitoring through a web-based dashboard.

---

## Overview

Monitoring urine output is an important clinical parameter in critical care. Manual measurement methods are often periodic and prone to delay. This system automates the process by continuously measuring urine output and calculating flow rate in real time.

The measured data can be viewed locally on an OLED display and remotely through a web interface, enabling timely observation and decision-making.

---

## Features

* Continuous urine volume measurement using load cell
* Real-time urine flow rate calculation (ml/hr)
* Wireless data transmission using ESP32 WiFi
* Live visualization using a web dashboard
* OLED display for on-device monitoring
* RTC-based timestamping
* Threshold-based alert support for low urine output

---

## Hardware Components

* ESP32 development board
* Load cell with HX711 amplifier
* OLED display (I2C interface)
* RTC module
* Battery and charging module

---

## Software Stack

* Embedded C (Arduino / ESP-IDF)
* WebSockets for real-time data transfer
* HTML, CSS, JavaScript (Chart.js for visualization)

---

## System Architecture

![System Architecture](images/architecture.png)

The load cell measures the weight of collected urine, which is converted to a digital signal using the HX711 module. The ESP32 processes this data to calculate urine flow rate over time.

Processed data is transmitted over WiFi and displayed on a web dashboard. The same data is also shown locally on the OLED display.

---

## Working Principle

The system calculates urine output by monitoring the change in weight of the urine collection bag. The difference in weight over a defined time interval is used to compute flow rate.

This flow rate can be compared against standard clinical thresholds (such as KDIGO guidelines) to identify reduced urine output conditions.

---

## Project Structure

```id="3k8d2f"
AKI-Urinometer-ESP32/
│
├── firmware/         ESP32 source code  
├── hardware/         Schematics and PCB design    
├── images/           Project images  
├── docs/             Supporting documentation  
└── README.md
```

---

## Setup

1. Clone the repository
2. Open the firmware in Arduino IDE or ESP-IDF
3. Connect the hardware modules as per schematic
4. Update WiFi credentials in the code
5. Flash the firmware to ESP32
6. Open the web dashboard to monitor data

---

## Results

The system was able to:

* Track urine output continuously without manual intervention
* Provide stable real-time data transmission
* Detect low urine output conditions based on predefined thresholds

---

## Applications

* ICU patient monitoring
* Hospital automation systems
* Remote health monitoring setups

---

## Future Work

* Integration with mobile applications
* Cloud storage and long-term data analysis
* Multi-patient monitoring support

---

## Author

Ashwin P
Embedded Systems Engineer

---
