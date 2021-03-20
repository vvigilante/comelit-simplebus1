A web interface that:
- shows the bus status (clear / busy in a call, etc)
- logs the messages received on the bus
- allows to start a call to an internal unit
- shows the waveform read from the bus
- plays the waveform in real time through the browser


## Wiring

![Wiring diagram](wiring.png)

For reading the bus we use a voltage divider (R2,R3) connected to IO23, the pin that will handle interrupts.

For writing to the bus we short the two ends of the bus through Q1, which is controlled by the pin IO18 through a capacitor.

For reading the analog voltages through the ADC (on pin GPIO35, SVP) we use the C3 capacitor to remove the 24V direct component, we then bias the signal again using the voltage divider (R5,R6).
The clipper circuit (D2, R4) is meant to limit the voltage spikes caused by the digital communication on the bus. The audio signal has an amplitude of 2.5 v peak to peak.

