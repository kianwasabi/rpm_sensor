# rpm_sensor
LM393 speedsensor, Arduino Uno & CAN-BUS Shield v2 | Submodule | SEA:ME DES

Reads the RPM sensor and calculates the RPM value.
This function should be called periodically at a specified sample rate.
It disables interrupts, calculates the RPM based on the pulse count and time elapsed,
and then re-enables interrupts.

