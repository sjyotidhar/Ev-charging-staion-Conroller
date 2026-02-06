# Battery Charging Locker System

## Overview
The Battery Charging Locker System is an IoT-based smart charging and storage solution designed for safe and automated charging of lithium-ion batteries. The system enables remote control and monitoring of multiple charging lockers using an ESP32 controller and Firebase cloud connectivity.

Each locker slot can independently control battery charging and door locking, making the system suitable for EV battery swap stations and charging infrastructure.

---

## Features
- Multi-slot battery charging control
- Remote locker control via Firebase
- Automatic charging timeout
- Door unlock control
- Wi-Fi auto reconnect
- Offline operation mode
- LED status indicators
- Power failure recovery
- Cloud-based monitoring and control

---

## System Architecture
Mobile App / Dashboard  
        ↓  
Firebase Cloud Database  
        ↓  
ESP32 Locker Controller  
        ↓  
Chargers & Door Locks

---

## Hardware Components
- ESP32 microcontroller
- High-current relays
- Battery chargers
- Solenoid door locks
- Power supply system
- Locker enclosure
- Status LEDs

---

## Software Components
- Arduino IDE firmware
- Firebase Realtime Database
- Web dashboard or mobile app
- ESP32 Wi-Fi communication

---

## Firebase Data Structure Example
locker/
slot1/
startCharge
stopCharge
openDoor
charging
