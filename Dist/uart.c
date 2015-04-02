#include "uart.h"


/*************************************************************************
 * Initializes a UART peripheral
 *
 * This function assumes the the corresponding GPIO ports have already
 * been configured for thier alternate functions and the port control
 * register were configured correctly.
 ************************************************************************/
bool  InitializeUART(uint32_t base)
{
  uint32_t delay;
  UART_PERIPH *myPeriph = (UART_PERIPH *)base;
  
  // Validate that a correct base address has been passed
  // Turn on the Clock Gating Register
  switch (base) 
  {
    case UART0 :
    SYSCTL_RCGCUART_R |= SYSCTL_RCGCUART_R0;
      break;
    case UART1 :
      SYSCTL_RCGCUART_R |= SYSCTL_RCGCUART_R1;
      break;
    case UART2 :
      SYSCTL_RCGCUART_R |= SYSCTL_RCGCUART_R2;
      break;
    case UART3 :
      SYSCTL_RCGCUART_R |= SYSCTL_RCGCUART_R3;
      break;
    case UART4 :
      SYSCTL_RCGCUART_R |= SYSCTL_RCGCUART_R4;
      break;
    case UART5 :
      SYSCTL_RCGCUART_R |= SYSCTL_RCGCUART_R5;
      break;
  case UART6 :
      SYSCTL_RCGCUART_R |= SYSCTL_RCGCUART_R6;
      break;
  case UART7 :
      SYSCTL_RCGCUART_R |= SYSCTL_RCGCUART_R7;
      break;
    default:
      return false;
  }

  delay = SYSCTL_RCGC1_R;
  delay = SYSCTL_RCGC1_R;

  // Disable UART
  myPeriph->UARTControl &= ~UART_CTL_UARTEN; 
  
  // BAUD Rate pf 115200
  myPeriph->IntegerBaudRateDiv = 43; 
  myPeriph->FracBaudRateDiv = 26;

  // Configure UART Line control register for 8-n-1, no FIFOs
  myPeriph->LineControl = UART_LCRH_WLEN_8 | UART_LCRH_FEN; 

  // Enable RX inturrupts 
  myPeriph->IntMask = UART_IM_RXIM;

  // Inturrupt when RX FIFO is 1/8th full
  myPeriph->IntFIFOLevelSel = UART_IFLS_RX1_8;  ///should discuss this
  
  // Enable UART
  myPeriph->UARTControl = UART_CTL_UARTEN | UART_CTL_HSE | UART_CTL_TXE | UART_CTL_RXE ;

  return true;
}

