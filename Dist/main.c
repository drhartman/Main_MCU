//*****************************************************************************
// MAIN MICROCONTROLLER ON LED BOARD
//*****************************************************************************
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "lm4f120h5qr.h"
#include "board_config.h"
#include "timer0.h"
#include "uart.h"


/******************************************************************************
 * Defines
 *****************************************************************************/
#define PORTA                 0x40004000
#define PORTB                 0x40005000
#define PORTC                 0x40006000
#define PORTD                 0x40007000
#define PORTE                 0x40024000
#define PORTF                 0x40025000

#define COLOFFSET			  3
#define BLUE				  0
#define	RED					  1
#define GREEN				  2



/******************************************************************************
 * Global Variables
 *****************************************************************************/
volatile GPIO_PORT *GpioPortA = (GPIO_PORT *)PORTA;
volatile GPIO_PORT *GpioPortB = (GPIO_PORT *)PORTB;
volatile GPIO_PORT *GpioPortC = (GPIO_PORT *)PORTC;
volatile GPIO_PORT *GpioPortD = (GPIO_PORT *)PORTD;
volatile GPIO_PORT *GpioPortE = (GPIO_PORT *)PORTE;
volatile GPIO_PORT *GpioPortF = (GPIO_PORT *)PORTF;
volatile UART_PERIPH *UartBoard0 = (UART_PERIPH *)UART3;
volatile UART_PERIPH *UartBoard1 = (UART_PERIPH *)UART2;
volatile UART_PERIPH *UartBoard2 = (UART_PERIPH *)UART4;
volatile UART_PERIPH *UartBoard3 = (UART_PERIPH *)UART5;
volatile UART_PERIPH *UartBoard4 = (UART_PERIPH *)UART6;
volatile UART_PERIPH *UartBoard5 = (UART_PERIPH *)UART7;

//software represention of ENTIRE game board's LEDs
//3 boards long/2 boards wide = 8*3=24, 24*2=48
volatile uint8_t colorMatrix[24][48];
//software represention of ENTIRE game board's Sensors
//0 = off, 1 = on
volatile uint8_t sensorMatrix[12][8];


//////golobals used in Timer0////// can change to your liking
volatile bool      UpdateDisplay = false; 
volatile bool      CheckADC = false;
volatile uint16_t  DutyCycle=100;
volatile bool      CheckButton = false;
uint16_t      ucTensCount=0;
uint16_t      ucOnesCount=0;
volatile bool OneSecond = false;
extern volatile bool       resetISRCounts;
extern volatile uint16_t   DutyCycle;
//////golobals used in Timer0//////


//*****************************************************************************
// External Functions
//*****************************************************************************
extern void PLL_Init(void);
extern void EnableInterrupts(void);
extern void DisableInterrupts(void);
extern void sysTickInitInt(uint32_t count, uint8_t priority);
extern void output_grb(uint32_t gpPortAddr, uint8_t *matrixPtr);


/******************************************************************************
 * Functions
 *****************************************************************************/
 
void initPorts(void){
	uint32_t delay;
	
	SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R0 | SYSCTL_RCGCGPIO_R1| SYSCTL_RCGCGPIO_R2 | SYSCTL_RCGCGPIO_R3 | SYSCTL_RCGCGPIO_R4 | SYSCTL_RCGCGPIO_R5;
	delay = SYSCTL_RCGCGPIO_R;
	
	//PORT A
	//Only uses UART0 on pin 0 and 1
	GpioPortA->Direction = 0xFF;
	GpioPortA->AlternateFunctionSelect = 0x03;
	GpioPortA->DigitalEnable |= 0xFF;
	GpioPortA->PortControl |= (GPIO_PCTL_PA0_U0RX | GPIO_PCTL_PA1_U0TX);
	
	
	//PORT B
	//pins 0-4 is connected to the reset signal on the other
	//	5 boards.  it is active low so inturrupt on negative edge
	GpioPortB->Direction = 0xE0;
	GpioPortB->AlternateFunctionSelect = 0;
	GpioPortB->DigitalEnable |= 0xFF;
	GpioPortB->InterruptMask = 0;
	GpioPortB->InterruptSence = 0;
	GpioPortB->InterruptBothEdges = 0;
	GpioPortB->InterruptEvent = 0;
	GpioPortB->InterruptMask = 0x1F;
	

	//PORT C
	//pin 4 is U4RX, pin 5 is U4TX
	//pin 6 is U3RX, pin 7 is U3TX
	//careful not to adjust programming pins
	GpioPortC->Direction &= 0xAF;
	GpioPortC->Direction |= 0xA0;
	GpioPortC->AlternateFunctionSelect |= 0xF0;
	GpioPortC->DigitalEnable |= 0xF0;
	GpioPortC->PortControl |= (GPIO_PCTL_PC4_U4RX | GPIO_PCTL_PC5_U4TX);
	GpioPortC->PortControl |= (GPIO_PCTL_PC6_U3RX | GPIO_PCTL_PC7_U3TX);

	
	//PORT D
	//pin 4 is U6RX, pin 5 is U6TX
	//pin 6 is U2RX, pin 7 is U2TX
	//ok to write pins 0-3 because not using them
	GpioPortC->Direction = 0xAF;
	GpioPortC->AlternateFunctionSelect = 0xF0;
	GpioPortC->DigitalEnable = 0xF0;
	GpioPortC->PortControl |= (GPIO_PCTL_PD4_U6RX | GPIO_PCTL_PD5_U6TX);
	GpioPortC->PortControl |= (GPIO_PCTL_PD6_U2RX | GPIO_PCTL_PD7_U2TX);


	//PORT E
	//pin 0 is U7RX, pin 1 is U7TX
	//pin 4 is U5RX, pin 5 is U5TX
	//ok to write pins 2, 3, 6 and 7 because not using them
	GpioPortE->Direction = 0xEF;
	GpioPortE->AlternateFunctionSelect = 0x33;
	GpioPortE->DigitalEnable = 0x33;
	GpioPortE->PortControl |= (GPIO_PCTL_PE0_U7RX | GPIO_PCTL_PE1_U7TX);
	GpioPortE->PortControl |= (GPIO_PCTL_PE4_U5RX | GPIO_PCTL_PE5_U5TX);
	
	//PORT F
	//pins 0-4 are push buttons
	GpioPortF->Direction = 0xE0;
	GpioPortF->PullUpSelect |= 0x1F;
	GpioPortF->AlternateFunctionSelect = 0;
	GpioPortF->DigitalEnable |= 0x1F;
	
}

//*****************************************************************************
// Main Routine
//*****************************************************************************
int 
main(void)
{
    // Initialize the PLLs so the the main CPU frequency is 80MHz
    PLL_Init();

    // 80MHz/(100HZ * 8 * 100 PWM Steps) = 1000 CLK Cycles per Timer0 Interrupt    
    //      = 12.5uS
    timer0Init(1000, TIMER0_PERIODIC, TIMER0_ENABLE_INTS, TIMER0_PRIORITY);

	initPorts();
    
    // Since PD7 is shared with the NMI, we need to clear the lock register and 
    // set the commit register so that all the pins alternate functions can
    // be used.
    GPIO_PORTD_LOCK_R = 0x4C4F434B;
    GPIO_PORTD_CR_R = 0xFF;
	
	//Initialize Peripherals
	InitializeUART(UART2);
	InitializeUART(UART3);
	InitializeUART(UART4);
	InitializeUART(UART5);
	InitializeUART(UART6);
	InitializeUART(UART7);


	while(1)
    {
		//EXAMPLE ON HOW TO CHANGE A SPECIFIC LED'S COLOR VALUE
		//I want led's row 15, column 12, green value
		colorMatrix[15][12*COLOFFSET+BLUE] = 0x0F;
    }; 
}

// Main test commit
// testing cloned copy on 2nd computer for Main_MCU
