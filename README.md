# TraceMaster Component Tester

TraceMaster is a multi-function electronic component tester and basic oscilloscope built for the Cypress/Infineon PSoC ecosystem. It’s designed to automatically characterize basic passive and active components, run DC sweeps, and plot the results in real-time on an SPI TFT display. 

If you've ever found a random component on your bench and needed to know exactly what its deal is, this firmware handles the heavy lifting.

## Features

*   **Capacitor Analysis:** Measures overall capacitance, Equivalent Series Resistance (ESR), leakage current, and dielectric absorption. Plots voltage over time as it charges.
*   **Resistor Testing:** Measures resistance, detects open/short circuits, and runs a DC voltage/current sweep to verify linearity.
*   **Inductor Testing:** Calculates inductance by pinging an LC tank circuit and counting the decay rings (resonance).
*   **Diode & LED Profiling:** Measures forward voltage drop to identify the diode type (Schottky, Silicon, LED) and plots the IV curve.
*   **Oscilloscope Mode:** A basic, real-time voltmeter/scope that plots voltage against time, complete with a "freeze" function for capturing waveforms.

## Under the Hood: File Structure

The project is broken down into modular files so you don't have to dig through thousands of lines of code to find a specific math routine:

*   **`main.c`:** The brains of the operation. It runs a finite state machine (FSM) that reads button inputs, handles debouncing, and navigates between the different UI menus and testing states.
*   **`lcd.c`:** The display driver and graphics engine. It handles standard SPI communication (likely expecting an ILI9341 or similar based on the init sequence). It includes a custom 5x7 font, primitive drawing functions (lines, rectangles), auto-scaling dynamic graphing, and hardcoded pixel art for the component icons.
*   **`cap.c`:** Capacitor testing logic. Uses an RC charge/discharge curve approach and dynamically adjusts the sample rate depending on how fast the capacitor charges.
*   **`res.c`:** Resistor testing logic. Uses an internal IDAC to push known currents and a Delta-Sigma ADC to read the resulting voltage drops.
*   **`ind.c`:** Inductor logic. Sends a 5µs impulse to an external LC circuit and uses the PSoC's hardware timer/capture block to time the distance between voltage peaks. 
*   **`diode.c`:** Pulses the component to find the forward voltage threshold and sweeps it to generate an IV curve.
*   **`volt.c`:** Contains the live ADC polling loop for the oscilloscope mode.
*   **`helper.c`:** General utility functions, including a custom integer-to-string converter (since standard `sprintf` can be memory-heavy) and the potentiometer polling logic for the main menu.

## Hardware Requirements

This code heavily relies on Cypress PSoC hardware APIs. To run this, your top-level schematic needs to include:

1.  **Delta-Sigma ADC (`ADC_DelSig`)** for high-res voltage measurements.
2.  **SAR ADC (`ADC_pot`)** for reading the menu potentiometer.
3.  **Current DAC (`IDAC8`)** for the DC sweeps.
4.  **Hardware Timer (`Timer_1`) & Comparator (`Comp_1`)** for catching inductor rings.
5.  **SPI Master (`SPIM_1`)** to drive the screen.
6.  **Various digital pins** (e.g., `res470`, `res47k`, `INDUCTOR_DRIVER`) connected to your analog front-end switching network.
7.  **Two UI Buttons** (`BUTTON1`, `BUTTON2`) for navigation.

## Usage

1.  On boot, turn the potentiometer to scroll through the available analysis modes (Resistor, Capacitor, Inductor, Diode/LED, Oscilloscope).
2.  Press **Button 1** to select a mode.
3.  Insert the component into your test socket and watch the display populate with the measured parameters and graphs.
4.  Press **Button 1** to exit back to the main menu, or **Button 2** to re-run the test (or freeze the graph, if you're in Scope mode).