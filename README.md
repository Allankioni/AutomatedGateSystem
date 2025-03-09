# Automated Access Control Gate System with Solenoid Lock

## Table of Contents
- [Introduction](introduction)
- [Problem Statement](problem-statement)
- [Features](features)
- [Components Required](components-required)
  - [Main Components](main-components)
  - [Additional Components](additional-components)
- [Installation & Environment Setup](installation--environment-setup)
- [Usage](usage)
- [Hardware Setup & Circuitry](hardware-setup--circuitry)
- [Future Improvements](future-improvements)
- [Contributing](contributing)
- [License](license)
- [Contact](contact)

## Introduction

This project is an **Automated Access Control Gate System** that secures entry through a physical gate using multi-factor authentication. The system primarily uses fingerprint authentication, with a fallback to RFID and OTP verification when needed. Instead of a servo-driven gate, this version uses a **solenoid lock mechanism** that functions like a valve—allowing only one person to enter at a time. Additionally, it includes system management features such as a restart prompt (secured by a PIN) and an assistance call function, with user feedback provided by a scrolling LCD display.

## Problem Statement

Access control is critical for preventing unauthorized entry and tailgating in secure facilities. Traditional systems may allow multiple persons to enter after one authentication event, increasing security risks. The challenges addressed in this project include:

- **Accurate Authentication:** Verifying each individual's identity using reliable biometric methods.
- **Tailgating Prevention:** Ensuring that only one person enters per authentication event.
- **Redundancy:** Providing a backup authentication method if the primary fingerprint sensor fails.
- **Physical Security:** Implementing a robust locking mechanism.
- **System Management:** Allowing secure system restart and assistance calls.

## Features

- **Primary Authentication:**  
  Uses fingerprint data (input via Serial in simulation) to verify users against an internal database. On a match, a welcome message is displayed and the solenoid lock is activated.

- **Fallback Authentication (RFID + OTP):**  
  After five failed fingerprint attempts, the system switches to RFID-based authentication. A matching RFID prompts the user to enter a 4-digit OTP via a keypad. Only three RFID attempts are allowed before cancellation.

- **Physical Locking:**  
  A solenoid lock mechanism acts as a valve-like system that unlocks the door for a brief period (e.g., 300 ms) to allow one person to enter, then re-locks automatically.

- **User Feedback:**  
  A non-I2C 16×2 LCD displays messages with scrolling capability for texts longer than 16 characters.

- **System Management:**  
  - **Button 1 (Restart):**  
    - Short press: Initiates a restart prompt that requires a secure PIN (maximum of 3 attempts).
    - Long press: Cancels the restart prompt.
  - **Button 2 (Assistance):**  
    - When pressed, displays "Calling Assistance" and logs an alert via Serial.

- **Debouncing:**  
  Utilizes the Bounce2 library to ensure reliable button input.

## Components Required

### Main Components
- **Arduino Uno (or compatible microcontroller):**  
  Central controller of the system.

- **Non-I2C 16×2 LCD Display:**  
  (e.g., LM016L) – for displaying messages. Uses parallel wiring (RS, EN, D4–D7).

- **4×3 Matrix Keypad:**  
  For entering OTP and restart PIN.

- **Solenoid Lock Mechanism:**  
  Controls physical access by unlocking the door briefly.  
  *Note:* Requires a driver circuit (transistor/relay with flyback diode).  
  **Valve-Like Gate System:**  
  Designed to allow only one person to enter at a time.  
  *[Attach photos/diagrams of your solenoid lock and gate mechanism here]*

- **Fingerprint Sensor:**  
  For biometric authentication (or simulated via Serial input with "FP:" prefix).

- **RFID Reader:**  
  For fallback authentication (or simulated via Serial input with "RFID:" prefix).

- **Two Push-Buttons:**  
  - **Button 1:** For initiating/canceling the restart prompt.
  - **Button 2:** For calling assistance.

- **Libraries:**  
  - Bounce2  
  - Keypad  
  - LiquidCrystal  
  - Servo (if applicable; if using a digital output for the solenoid driver, adjust accordingly)

### Additional Components
- **Power Supply/Battery Pack:**  
  To power the system.
- **Driver Circuit for the Solenoid Lock:**  
  Includes a transistor or relay and flyback diode.
- **Wiring, Breadboard/PCB, and Connectors:**  
  For circuit assembly.
- **Enclosure:**  
  To house and protect the system.
- **Valve-Like Gate Mechanism:**  
  A physical gate engineered to allow only one person at a time.  
  *[Attach images or diagrams here]*

## Installation & Environment Setup

### 1. Install the Arduino IDE
- Download the latest Arduino IDE from the [Arduino website](https://www.arduino.cc/en/software).
- Follow the installation instructions for your operating system.

### 2. Install Required Libraries
In the Arduino IDE, open the Library Manager via *Sketch > Include Library > Manage Libraries…* and install:
- **Bounce2** (search for "Bounce2" and click Install)
- **Keypad** (by Mark Stanley and Alexander Brevig)
- **LiquidCrystal** (included by default)
- **Servo** (included by default)

### 3. Clone or Download the Repository
Clone the repository using:
```bash
git clone https://github.com/yourusername/automated-access-control.git

Or download the ZIP file and extract it.

### 4. Open and Upload the Code
- Open the `.ino` file in the Arduino IDE.
- Connect your Arduino board.
- Select the appropriate board and port under *Tools*.
- Click "Upload" to compile and upload the code.

## Usage

- **Primary Mode:**  
  On startup, the LCD displays "Scan Fingerprint". To simulate fingerprint input, send data via Serial with the prefix "FP:" (e.g., "FP:FP1").

- **Fallback Mode:**  
  After 5 failed fingerprint attempts, the system switches to fallback mode and expects RFID input (prefix "RFID:"). On matching RFID, it prompts for a 4-digit OTP via the keypad. Correct OTP unlocks the door.

- **Access Control:**  
  Successful authentication activates the solenoid lock to unlock the door for a brief period (e.g., 300 ms), then re-locks to prevent tailgating.

- **System Management:**  
  - **Button 1:**  
    Short press: Initiates a restart prompt (requires a secure PIN, maximum 3 attempts).  
    Long press: Cancels the active restart prompt.
  - **Button 2:**  
    Triggers "Calling Assistance" on the LCD and logs an alert via Serial.

## Hardware Setup & Circuitry

**Circuit Diagram:**  
Paste your circuit diagram or attach images of your schematic here.

## Future Improvements

- **Scalability:**  
  - Integrate a centralized user database (e.g., MySQL, PostgreSQL, or cloud-based NoSQL) for managing user credentials.
  - Develop a remote management portal for real-time monitoring and updates.
- **Networking:**  
  Equip each access control unit with WiFi/Ethernet/cellular connectivity for remote monitoring, logging, and OTA updates.
- **Industrial Enhancements:**  
  Use industrial-grade components, add additional sensors (door position, motion), and incorporate backup power.
- **Software Architecture:**  
  Implement a robust state machine for managing authentication states and secure communications (using encryption).

## Contributing

Contributions are welcome! Please fork this repository and submit pull requests. For major changes, open an issue to discuss your proposal first.

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

## Contact

For questions or further information, please contact:  
**Your Name**  
**Email:** your.email@example.com  
**GitHub:** [yourusername](https://github.com/yourusername)
