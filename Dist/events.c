#include "events.h"
#include "uart.h"
#include "gpio.h"

/////////////Global Variables//////////////// 
//add globals here 

//Global base address register structures
extern volatile GPIO_PORT *GpioPortA;
extern volatile GPIO_PORT *GpioPortB;
extern volatile GPIO_PORT *GpioPortC;
extern volatile GPIO_PORT *GpioPortD;
extern volatile GPIO_PORT *GpioPortE;
extern volatile GPIO_PORT *GpioPortF;
extern volatile UART_PERIPH *UartBoard0;
extern volatile UART_PERIPH *UartBoard1;
extern volatile UART_PERIPH *UartBoard2;
extern volatile UART_PERIPH *UartBoard3;
extern volatile UART_PERIPH *UartBoard4;
extern volatile UART_PERIPH *UartBoard5; 


 //Timer0
extern volatile uint8_t   ucActiveRow;
extern volatile bool      UpdateDisplay; 
extern volatile bool      CheckADC;
extern volatile uint16_t  DutyCycle;
extern volatile bool      DutyCycleOn;
extern volatile bool      OneSecond;
extern volatile bool      CheckButton;
volatile bool      resetISRCounts =  false;
//////////////////////////////////////////////

//pins 0-4 correspond with the reset signal 
// 	coming from boards 1-5
void PORTBIntHandler(void)
{
}

//pins 0, 1, 2, 3, and 4 correspond to the 
//  the pushbuttons Enter, Up, Right, Down, 
//  Left, respectively
void PORTFIntHandler(void)
{
}

//Uart communication from and to board 1
void UART2IntHandler(void)
{
}

//Uart communication from and to board 0
//  (this is the same board the main is
//   located on)
void UART3IntHandler(void)
{
}

//Uart communication from and to board 2
void UART4IntHandler(void)
{
}

//Uart communication from and to board 3
void UART5IntHandler(void)
{
}

//Uart communication from and to board 4
void UART6IntHandler(void)
{
}

//Uart communication from and to board 5
void UART7IntHandler(void)
{
}

 
//****************************************************************************
//
// The ADC gets sampled at a rate of 10HZ
//
// Each Row gets updated every 1.25ms
// 
// The pushbutton gets sampled every 5.25ms
//****************************************************************************
void TIMER0IntHandler(void)
{
  static int32_t secCount = 0;
  static int8_t refreshCount = 0;
  static int16_t ADCCount = 0;
  static int16_t ButtonCount = 0;
  
  // Clear the timer interrupt
  TIMER0_ICR_R |= TIMER_ICR_TATOCINT;
  
  // If we get a remote reset, we want to reset all of our counts so that
  // we count a full second before incrementing to the next digit.
  if( resetISRCounts == true)
  {
    secCount = 1;
    refreshCount = 1;
    ADCCount = 1;
    ButtonCount = 1;
    resetISRCounts = false;
  }
  else
  {
    secCount = (secCount + 1) % 80000;
    refreshCount = (refreshCount + 1) % 100;
    ADCCount = (ADCCount +1 ) % 8000;
    ButtonCount = (ButtonCount + 1) % 420;
  }
  
  // Indicate that the display should be updated
  UpdateDisplay = true;
  
  if( refreshCount == 0)
  {
    // Update which row is active
    ucActiveRow = (ucActiveRow +1) & 0x07;
  }
  
  // Duty Cycle Calculations
 if( refreshCount < DutyCycle)
   DutyCycleOn = true;
 else
   DutyCycleOn = false;
  
  // Check to see if the ADC should be sampled
  if ( ADCCount == 0 )
  {
      CheckADC = true;
  }
  
  if( ButtonCount == 0)
  {
    CheckButton = true;
  }
  
  // Check to see if 1 second has passed
  if (secCount == 0)
  {
    OneSecond = true;
  }
}

