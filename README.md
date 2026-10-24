# Smart Medicine Reminder Box

An ESP32-based smart medication reminder system designed to help patients follow their medication schedules, monitor medication stock, and receive SMS notifications in case of a missed dose or a low-stock condition.

The project combines embedded programming, RFID identification, real-time scheduling, automated medication access, and GSM communication.

## Project Overview

The Smart Medicine Reminder Box was developed as my final-year Licence project in Electronics at the University of Science and Technology Houari Boumediene (USTHB), Algeria.

The system uses an ESP32 as its main controller and integrates an RFID reader, a real-time clock (RTC), an LCD, a GSM module, a servo motor, a buzzer, and LED indicators.

The project includes a physical prototype, an electronic circuit designed using Proteus, and an interactive simulation developed using Wokwi.

## Main Features

- Automatic medication reminders at 8:00 AM and 8:00 PM.
- RFID-based identification and access authorization.
- Servo-controlled medication compartment.
- Medication dosage and remaining-stock tracking.
- Missed-dose detection with SMS notifications to the patient and a family member.
- Medication stock monitoring and refill notifications.
- LCD interface, buzzer, and three LED indicators.
- Demonstration mode for testing the main operating scenarios.
- Battery-powered prototype with a DC-DC buck converter.

## Hardware Components

- ESP32 development board
- MFRC522 RFID reader and RFID tag
- DS3231 real-time clock (RTC)
- 16×2 LCD with I²C interface
- SIM800L GSM module
- Servo motor
- Buzzer
- Green, orange, and red LEDs
- Push button
- Two rechargeable 3.7 V lithium batteries
- DC-DC buck converter
- Perforated prototyping board
- Electronic components and connecting wires

## Software and Development Tools

- Embedded C/C++
- Arduino IDE
- Proteus — electronic circuit design and simulation
- Wokwi — ESP32 and embedded-system simulation
- GitHub — source code and project documentation

## System Operation

### 1. Automatic Medication Reminders

The DS3231 real-time clock keeps track of the time and allows the ESP32 to trigger medication reminders at the programmed times: 8:00 AM and 8:00 PM.

When a scheduled reminder is triggered, the system activates the reminder procedure. The patient is given 20 seconds to respond using the push button.

If the patient responds within the allowed time, the system proceeds to the medication identification and access procedure.

If there is no response within 20 seconds, the system detects a missed dose. The red LED turns on, and the LCD displays the missed-dose status. The GSM module sends an SMS notification to the patient and a family member.

### 2. RFID Identification and Medication Access

After responding to the reminder, the patient presents the RFID tag to the reader.

The ESP32 reads the tag identifier and checks whether it is authorized.

- **Invalid tag:** The system displays an error message and requests another attempt.
- **Valid tag:** The system confirms identification and proceeds with the medication access procedure.

For the demonstration, the system uses Amlodipine as an example medication stored in compartment 1.

The LCD displays the compartment number, medication name, and prescribed quantity. In the example configuration, the initial stock is four pills and the prescribed dose is two pills per intake.

When access is authorized, the servo motor opens the compartment and the green LED indicates the authorized access state. The LCD displays the relevant medication information.

After the intake is recorded, the system updates the remaining pill quantity according to the configured dosage.

For example, when the initial stock is four pills and the prescribed dose is two pills, the remaining stock becomes two pills after one recorded intake.

### 3. Medication Stock Management

The system tracks the remaining medication quantity according to the initial stock and the recorded doses.

When the stock reaches the configured empty-stock condition, the orange LED turns on and the LCD displays a stock-related message indicating that the medication needs to be replenished.

The system provides a stock-management procedure to update the medication quantity after replenishment.

In the demonstration scenario, the Amlodipine stock is restored to four pills. The buzzer sounds three times, and the LCD confirms that the stock has been updated and displays the current quantity.

The GSM module sends a stock-related SMS notification to inform the patient about the medication supply.

### 4. Missed-Dose Notifications

The missed-dose procedure is designed to notify the patient and a family member if the patient does not respond to a scheduled reminder within 20 seconds.

The sequence is as follows:

1. The system triggers the scheduled medication reminder.
2. The patient has 20 seconds to respond using the push button.
3. If no response is detected, the red LED turns on.
4. The LCD displays the missed-dose status and the SMS notification information.
5. The GSM module sends an alert to the patient and a family member.

This feature provides a way to alert the patient's family when a scheduled medication intake has not been acknowledged.

### 5. Demonstration Mode

A dedicated demonstration mode allows the main system functions to be presented without waiting for the actual medication schedule.

#### Activating Demonstration Mode

- A short press on the push button turns on the LCD and displays the current date and time.
- Pressing and holding the button for approximately three seconds, until the buzzer beeps, activates demonstration mode.
- For the morning demonstration, the displayed time starts at 07:59:50 and advances toward the 08:00 medication reminder.
- The demonstration allows the scheduled reminder and the subsequent system responses to be observed.
- Holding the button for approximately three seconds again exits demonstration mode. The system restores the real date and time while accounting for the elapsed demonstration period.

The demonstration mode makes it easier to present the reminder, patient response window, RFID identification, medication access, stock management, and notification scenarios.

#### LCD Power-Saving Behavior

The LCD normally remains off to reduce power consumption.

- A short button press turns on the LCD and displays the current date and time.
- If no further interaction occurs, the LCD turns off again after seven seconds of inactivity.
- A long press of approximately three seconds activates or exits demonstration mode, accompanied by a buzzer beep.

## Electronic Circuit Design and Simulation

### Proteus

Proteus was used to design and simulate the electronic circuit. The schematic documents the connections between the ESP32 and the main electronic components and serves as a reference for the physical assembly.

### Wokwi

Wokwi was used to simulate the ESP32 program and demonstrate the system's operating scenarios, including RFID identification, medication reminders, missed-dose detection, and notification behavior.

[Open the interactive Wokwi simulation](https://wokwi.com/projects/459119627614864385)

## Physical Prototype and Assembly

The physical prototype was assembled using a perforated prototyping board. The electronic components were soldered onto the board, while selected modules were mounted using headers to facilitate replacement and maintenance.

The RFID tag is attached to the **outside of the lid**, where it can be presented to the RFID reader. It is not installed inside the lid.

The power supply consists of two rechargeable 3.7 V lithium batteries connected in series, providing a nominal 7.4 V supply. A DC-DC buck converter provides the required regulated voltage. The RFID reader is powered directly from the ESP32's 3.3 V supply, while the other components use their required operating voltages.

## Future Improvements

- Design a dedicated PCB to improve compactness and reliability.
- Improve the enclosure and mechanical design.
- Enhance medication stock configuration and monitoring.
- Further develop the system's connectivity and monitoring capabilities.

## Author

**Meroua Ennaceur**

Electronics Graduate — University of Science and Technology Houari Boumediene (USTHB), Algeria.

GitHub: [Ennaceur-Meroua](https://github.com/Ennaceur-Meroua)

