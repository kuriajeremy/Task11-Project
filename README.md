# Task11-Project

MedAlert: Medication Reminder & Escalation System

Overview
This project presents a dual-unit embedded system designed to improve medication adherence in assisted-living environments. The system consists of a Patient Unit and a Caregiver Unit, both built using Arduino Uno microcontrollers.
The Patient Unit is responsible for:

•	Displaying medication reminders 

•	Triggering buzzer alerts 

•	Accepting user acknowledgment via a push button

If the patient does not acknowledge the reminder within a defined time window, the system escalates the alert by sending a signal to the Caregiver Unit.
The Caregiver Unit:

•	Receives escalation alerts via I2C communication 

•	Activates a buzzer and LED 

•	Displays a warning message 

How the System Works
The system operates using a state machine and timed events:

•	Idle State → Waiting for next reminder 

•	Reminder State → Alerts patient via OLED + buzzer 

•	Confirmed State → Patient presses button → system resets 

•	Escalation State → No response → caregiver is alerted 

Communication between the two units is achieved using the I2C protocol, where:

•	Patient Unit = Master 

•	Caregiver Unit = Slave (Address: 0x08) 

Wokwi Simulation
The system was first implemented and tested using Wokwi as a proof of concept.
The simulation validates:

•	Timing logic using millis() 

•	Button interaction 

•	OLED display output 

•	Buzzer patterns

•	I2C communication between two Arduino units 

This simulation also serves as a schematic reference for the physical implementation.
Wokwi simulation screenshot below:
![image alt](https://github.com/kuriajeremy/Task11-Project/blob/bd0b52e747fcbebba17ed7699de79682563abe37/wokwi_sim.png)
The simulation code (patient.ino, caregiver.ino) is included in this repository.


Hardware Requirements
Patient Unit:
1.	Arduino Uno 
2.	SSD1306 OLED Display 
3.	Piezo Buzzer 
4.	Push Button 
5.	Green LED 
6.	10kΩ Resistor (pull-down) 
7.	220Ω Resistor (LED) 
8.	Breadboard 
9.	Jumper wires 
Caregiver Unit:
1.	Arduino Uno 
2.	SSD1306 OLED Display 
3.	Piezo Buzzer 
4.	Red LED 
5.	220Ω Resistor 
6.	Breadboard 
7.	Jumper wires 
Additional (Real Build):

•	4.7kΩ resistors (I2C pull-ups) 

•	RTC Module (DS1307/DS3231) 

•	Power supply (USB)

Pin Mapping
Component	Patient Unit	Caregiver Unit
OLED SDA	A4	A4
OLED SCL	A5	A5
Button	D2	—
LED	D7 (Green)	D7 (Red)
Buzzer	D9	D9
I2C Address	Master	0x08 (Slave)

Part 1: Core System Testing
Before integrating full communication, individual components were tested:
Steps:
1.	Upload patient.ino to Patient Arduino 
2.	Upload caregiver.ino to Caregiver Arduino 
3.	Run simulation in Wokwi 
4.	Verify: 
o	OLED displays messages 
o	Buzzer produces sound 
o	LED responds correctly 

Part 2: Full System Integration
Steps:
1.	Connect I2C lines: 
o	A4 → SDA 
o	A5 → SCL 
o	GND → Common ground 
2.	Run simulation 
3.	Observe system behavior 

System Behavior
1.	System starts in idle mode 
2.	After interval → reminder is triggered 
3.	Patient hears buzzer and sees message 
4.	If button is pressed: 
o	Buzzer stops 
o	Green LED turns ON briefly 
o	System resets 
5.	If no response: 
o	Escalation triggered 
o	Caregiver unit receives signal 
o	Red LED and buzzer activate 
6.	Late acknowledgment clears caregiver alert 

Fault Tolerance Features
1.	Button Debounce – prevents multiple triggers 
2.	I2C Error Handling – detects communication failures 
3.	Non-blocking Code – no delays used 
4.	Interrupt Handling – ensures fast response 
5.	State Machine Design – prevents invalid states 
6.	OLED Initialization Check – detects hardware failure 

Transition to Physical System
In the real implementation:

•	Replace millis() with RTC module 

•	Use external pull-up resistors for I2C 

•	Increase timing intervals (minutes/hours) 

•	Use larger button for accessibility 

How to Run the Project
Simulation (Recommended)
1.	Open project in Wokwi 
2.	Load: 
o	patient.ino 
o	caregiver.ino 
3.	Run simulation 
Physical Setup
1.	Assemble components using schematic 
2.	Upload code to both Arduinos 
3.	Power both units 
4.	Test functionality 

Repository Contents

•	patient.ino – Patient Unit code 

•	caregiver.ino – Caregiver Unit code

•	diagram.json – Wokwi schematic 

•	README.md – Project documentation 


