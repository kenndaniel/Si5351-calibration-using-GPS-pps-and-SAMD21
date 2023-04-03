Code used to calibrate the S!5351 crystal using GPS pulse per second signals as a reference and a SAMD21 proceor
as found on the Seeduino XIAO or Arduino MKR Zero.
Sets the frequency of the Si5351 to 2.5 MHz and counts for 10 seconds (longer periods givs no appreciable
increase in accuracy)

Author Ken Daniel K9YO

Original Author https://github.com/ocrdu/Arduino_SAMD21_pulse_frequency_counters

Uses Eitherkit Si5351 library v 2.1.4 

Si5351 Clock2 - D1 or D12 (only) Changed by commenting #define COUNTER_ON_PIN_1 

GPS pps signal - D0  Can be moved to other pins.
