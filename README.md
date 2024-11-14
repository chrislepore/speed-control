# DC Motor Speed Controller

This project implements a DC Motor Speed Controller that maintains a constant RPM even when frictional resistance is applied to the motor. The system uses a Nucleo-64 microcontroller with an LCD display to show the RPM and a potentiometer to adjust speed when the speed control mode is disabled. A light diode/LED pair sensor monitors RPM, and the system can operate in two modes: manual and speed control.

## Features

- **Manual and Speed Control Modes**:
  - **Manual Mode**: RPM is adjusted directly via a potentiometer.
  - **Speed Control Mode**: The system maintains a constant RPM by adjusting the motor's power output.
  
- **LCD Display**:
  - Displays the current RPM, with an automatic update based on the motor's status.
  
- **Bang-Bang Control Method**:
  - In speed control mode, the motor's power is gradually adjusted until it matches the desired RPM, allowing slight oscillations for accurate RPM maintenance.

## Hardware and Circuit Design

The hardware setup includes:
- **Microcontroller**: STM32F446RE on a Nucleo-64 board.
- **Sensor**: Light diode/LED pair to detect RPM, paired with a disk with a hole to allow detection of each motor rotation.
- **LCD Display**: For real-time RPM feedback.
- **Potentiometer**: For RPM setting in manual mode.

### Circuit Configuration
- Port **PA4** is connected to the DC motor via DAC output.
- Port **PA7** reads the potentiometer input via ADC.
- **PB0** is connected to a switch that toggles between manual and speed control modes.
- Port **PA6** reads RPM through a light diode/LED pair sensor.
  
#### Hardware Block Diagram

![Block Diagram]()

## Software and Flowcharts

The controller software, written in C, uses analog input/output programming and implements the input capture feature of the STM32 microcontroller. Here’s an overview of the control logic:

### Control Logic Flowchart

![Flowchart]()

1. **Initialize Ports and Peripherals**: Sets up ports, clocks, and the LCD.
2. **Display Initialization**: Prints "RPM:" on the LCD.
3. **Mode Handling**:
   - In **Manual Mode**, the system reads the potentiometer through the ADC and adjusts the motor speed directly.
   - In **Speed Control Mode**, the system adjusts the motor power output via the DAC to maintain a constant RPM, implementing a bang-bang control method to handle fluctuations.

### LCD Setup

![LCD Setup]()

### Full Setup and Testing
The system was assembled on a breadboard with the motor, potentiometer, mode switch, and LCD. Below is the complete setup image:

![Full Setup]()

## Conclusion

The DC Motor Speed Controller project introduces analog I/O control and input capture in embedded systems. By simulating a real-world application of speed control, the project provides hands-on experience with microcontroller programming, DAC/ADC interfacing, and the implementation of effective feedback control.

## Acknowledgments

This project was completed with guidance from:
1. *Mazidi, M. A., Chen, S., & Ghaemi, E.* STM32 ARM Programming for Embedded Systems Using C Language with STM32 Nucleo. Mazidi, 2018.
2. *Mazidi, M. A., Naimi, S., & Chen, S.* ARM Assembly Language Programming & Architecture. MicroDigitalEd, 2016.