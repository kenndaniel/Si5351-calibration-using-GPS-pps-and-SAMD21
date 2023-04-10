// Setup TC4 in 32-bit mode to count incoming pulses on digital pin D1 or D12 using the Event System
// for D1 or D12
#include <si5351.h>
/* 
Code used to calibrate the S!5351 crystal using GPS pulse per second signals as a reference and a SAMD21 proceor
as found on the Seeduino XIAO or Arduino MKR Zero.
Sets the frequency of the Si5351 to 2.5 MHz and counts for 10 seconds (longer periods givs no appreciable
increase in accuracy)
Author Ken Daniel K9YO
Original code https://github.com/ocrdu/Arduino_SAMD21_pulse_frequency_counters
Uses Eitherkit Si5351 library v 2.1.4
Si5351 Clock2 - D1 XIOA  or A4 MKR Zero (PA04) 
GPS pps signal - D0  Can be moved to other pins.
*/

Si5351 si5351;
const byte interruptPinPPS =0;  // GPS Pulse Per Second Signal
//#define COUNTER_PIN 1  // Set to 1 for XIOA pin 1.
#define COUNTER_PIN 18  //  Set to 18 for MKE Zero pin A3 
#define EIC_EVCtrl EIC_EVCTRL_EXTINTEO4 // Enable event output on external interrupt 4 
#define EIC_Config EIC_CONFIG_SENSE4_HIGH // Set event detecting a HIGH level on interrupt 4
#define EIC_IntenClr EIC_INTENCLR_EXTINT4 // Disable interrupts on interrupt 4 


void setup() {
  SerialUSB.begin(115200);
  while(!SerialUSB);

  // Generic Clock 
  GCLK->CLKCTRL.reg = GCLK_CLKCTRL_CLKEN |                 // Enable the generic clock
                      GCLK_CLKCTRL_GEN_GCLK0 |             // on GCLK0 at 48MHz
                      GCLK_CLKCTRL_ID_TC4_TC5;             // Route GCLK0 to TC4 and TC5
  while (GCLK->STATUS.bit.SYNCBUSY);                       // Wait for synchronization

  // Enable the port multiplexer on digital pin COUNTER_PIN
  PORT->Group[g_APinDescription[COUNTER_PIN].ulPort].PINCFG[g_APinDescription[COUNTER_PIN].ulPin].bit.PMUXEN = 1;
  // Set-up the pin as an EIC (interrupt) peripheral on D1
  PORT->Group[g_APinDescription[COUNTER_PIN].ulPort].PMUX[g_APinDescription[COUNTER_PIN].ulPin >> 1].reg |= PORT_PMUX_PMUXO_A;

  // External Interrupt Controller (EIC)
  EIC->EVCTRL.reg |= EIC_EVCtrl;                 // Enable event output on external interrupt
  EIC->CONFIG[0].reg |= EIC_Config;            // Set event detecting a HIGH level on interrupt 
  EIC->INTENCLR.reg = EIC_IntenClr;                // Disable interrupts on interrupt 
  EIC->CTRL.bit.ENABLE = 1;                                // Enable the EIC peripheral
  while (EIC->STATUS.bit.SYNCBUSY);                        // Wait for synchronization

  // Event System
  PM->APBCMASK.reg |= PM_APBCMASK_EVSYS;                                  // Switch on the event system peripheral
  EVSYS->USER.reg = EVSYS_USER_CHANNEL(1) |                               // Attach the event user (receiver) to channel 0 (n + 1)
                    EVSYS_USER_USER(EVSYS_ID_USER_TC4_EVU);               // Set the event user (receiver) as timer TC4
  EVSYS->CHANNEL.reg = EVSYS_CHANNEL_EDGSEL_NO_EVT_OUTPUT |               // No event edge detection
                       EVSYS_CHANNEL_PATH_ASYNCHRONOUS |                  // Set event path as asynchronous
                       EVSYS_CHANNEL_EVGEN(EVSYS_ID_GEN_EIC_EXTINT_4) |   // Set event generator (sender) as external interrupt 4
                       EVSYS_CHANNEL_CHANNEL(0);                          // Attach the generator (sender) to channel 0                                 
 
  // Timer Counter TC4
  TC4->COUNT32.EVCTRL.reg |= TC_EVCTRL_TCEI |              // Enable asynchronous events on the TC timer
                             TC_EVCTRL_EVACT_COUNT;        // Increment the TC timer each time an event is received
  TC4->COUNT32.CTRLA.reg = TC_CTRLA_MODE_COUNT32;          // Configure TC4 together with TC5 to operate in 32-bit mode
  TC4->COUNT32.CTRLA.bit.ENABLE = 1;                       // Enable TC4
  while (TC4->COUNT32.STATUS.bit.SYNCBUSY);                // Wait for synchronization
  TC4->COUNT32.READREQ.reg = TC_READREQ_RCONT |            // Enable a continuous read request
                             TC_READREQ_ADDR(0x10);        // Offset of the 32-bit COUNT register
  while (TC4->COUNT32.STATUS.bit.SYNCBUSY); 

  // setup pulse per second GPS interrupt
  pinMode(interruptPinPPS, INPUT_PULLUP); 
  attachInterrupt(digitalPinToInterrupt(interruptPinPPS), PPSinterrupt, RISING);               // Wait for synchronization
  si5351_calibrate_init();

}


  volatile float correction=1.;
  volatile bool CalibrationDone = false;
  volatile  unsigned long  SiCnt=0;
  int count = 0;
void loop() {
  //Serial.println(TC4->COUNT32.COUNT.reg);                  // Output the results
  delay(4000);
  count++;

  if(CalibrationDone == true)
    {
    si5351.output_enable(SI5351_CLK2, 0);   // disable output 
    unsigned long calfreq = 28126100UL*correction;
    si5351.set_freq(calfreq*100, SI5351_CLK0);  // set frequency
    si5351.output_enable(SI5351_CLK0, 1);   // enable output  
    Serial.print(" Xtal count = ");
    Serial.print(SiCnt);
    Serial.print(" correction = ");
    Serial.println(correction);
    } 
    
}

void si5351_calibrate_init()
{
  // Initialize SI5351 for gps calibration
  //digitalWrite(RFPIN, HIGH);
  //delay(2000);
  //POUTPUTLN((F(" SI5351 Initialized ")));
  bool siInit = si5351.init(SI5351_CRYSTAL_LOAD_8PF, 0, 0);    //  25MHz
  delay(2000);
  if (siInit == false) {Serial.println(" XXXXXXXXX Si5351 init failure XXXXXX");}
  //else {POUTPUTLN((F(" SI5355  Init Success")));}
  //int32_t cal_factor = 1000000000  - 1000125640;
  //si5351.set_correction(cal_factor, SI5351_PLL_INPUT_XO);
  si5351.drive_strength(SI5351_CLK2, SI5351_DRIVE_2MA); //  Check datasheet.
  si5351.drive_strength(SI5351_CLK1, SI5351_DRIVE_8MA); 
  //unsigned long calfreq = 2500000UL;
  unsigned long calfreq = 2500000UL;
  si5351.set_freq(calfreq*100, SI5351_CLK2);  // set calibration frequency to 2.5 MHz
  si5351.output_enable(SI5351_CLK2, 1);   // Enable output  
  si5351.output_enable(SI5351_CLK0, 0);   // disable output 

}


volatile int tcount = 0;

volatile unsigned int XtalFreq;
void PPSinterrupt()
{
  // Calibration function that counts SI5351 clock 2 for 10 seconds
  // Called on every pulse per second after gps sat lock
 
  tcount++;
 
/*   if (tcount % 2 == 0)
  {
   digitalWrite(DBGPIN, HIGH);
  }
  else
  {digitalWrite(DBGPIN, LOW);} */

  if (CalibrationDone == true) return;
  if (tcount == 4)  // Start counting the 2.5 MHz signal from Si5351A CLK0
  {
    TC4->COUNT32.CTRLBSET.reg = TC_CTRLBSET_CMD_RETRIGGER;   // Retrigger the TC4 timer
    while (TC4->COUNT32.STATUS.bit.SYNCBUSY);                // Wait for synchronization
  }
  //else if (tcount == 5)  {SiCnt=TC4->COUNT32.COUNT.reg;}
  else if (tcount == 14)  //The 10 second counting time has elapsed - stop counting
  {     

    SiCnt=TC4->COUNT32.COUNT.reg - SiCnt +40UL; // 40 is a fudge factor
    XtalFreq =  SiCnt; 
    correction = 25000000./(float)XtalFreq;
    // I found that adjusting the transmit freq gives a cleaner signal than setting ppb

    CalibrationDone = true;                  
  }
}

