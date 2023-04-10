Code used to calibrate the Si5351 crystal using GPS pulse per second signals as a reference and a SAMD21 proceor
as found on the Seeduino XIAO or Arduino MKR Zero.
Sets the frequency of the Si5351 to 2.5 MHz and counts for 10 seconds (longer periods givs no appreciable
increase in accuracy)

Author Ken Daniel K9YO

Adapted from https://github.com/ocrdu/Arduino_SAMD21_pulse_frequency_counters

Uses Eitherkit Si5351 library v 2.1.4 

Si5351 Clock2 - D1 on XIAO or A3 on the MKE Zero (using #define)

Si5351 Clock1 set to center of WSPR 10m ham band for verification of calibration

GPS pps signal - D0  Can be moved to other pins.
