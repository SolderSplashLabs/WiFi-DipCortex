#include "SolderSplashLpc.h"
#include "gpio.h"

/*****************************************************************************
** Function name:		GPIOSetPinInterrupt
**
** Descriptions:		Set interrupt sense, event, etc.
**						sense: edge or level, 0 is edge, 1 is level
**						event/polarity: 0 is active low/falling, 1 is high/rising.
**
** parameters:			channel #, port #, bit position, sense, event(polarity)
**
** Returned value:		None
**
*****************************************************************************/
void GPIOSetPinInterrupt( uint32_t channelNum, uint32_t portNum, uint32_t bitPosi,
		uint32_t sense, uint32_t event )
{
  switch ( channelNum )
  {
	case CHANNEL0:
	  if ( portNum )
	  {
		LPC_SYSCON->PINSEL[0] = bitPosi + 24;
	  }
	  else
	  {
		LPC_SYSCON->PINSEL[0] = bitPosi;
	  }
	  NVIC_EnableIRQ(PIN_INT0_IRQn);
	break;
	case CHANNEL1:
	  if ( portNum )
	  {
		LPC_SYSCON->PINSEL[1] = bitPosi + 24;
	  }
	  else
	  {
		LPC_SYSCON->PINSEL[1] = bitPosi;
	  }
	  NVIC_EnableIRQ(PIN_INT1_IRQn);
	break;
	case CHANNEL2:
	  if ( portNum )
	  {
		LPC_SYSCON->PINSEL[2] = bitPosi + 24;
	  }
	  else
	  {
		LPC_SYSCON->PINSEL[2] = bitPosi;
	  }
	  NVIC_EnableIRQ(PIN_INT2_IRQn);
	break;
	case CHANNEL3:
	  if ( portNum )
	  {
		LPC_SYSCON->PINSEL[3] = bitPosi + 24;
	  }
	  else
	  {
		LPC_SYSCON->PINSEL[3] = bitPosi;
	  }
	  NVIC_EnableIRQ(PIN_INT3_IRQn);
	break;
	case CHANNEL4:
	  if ( portNum )
	  {
		LPC_SYSCON->PINSEL[4] = bitPosi + 24;
	  }
	  else
	  {
		LPC_SYSCON->PINSEL[4] = bitPosi;
	  }
	  NVIC_EnableIRQ(PIN_INT4_IRQn);
	break;
	case CHANNEL5:
	  if ( portNum )
	  {
		LPC_SYSCON->PINSEL[5] = bitPosi + 24;
	  }
	  else
	  {
		LPC_SYSCON->PINSEL[5] = bitPosi;
	  }
	  NVIC_EnableIRQ(PIN_INT5_IRQn);
	break;
	case CHANNEL6:
	  if ( portNum )
	  {
		LPC_SYSCON->PINSEL[6] = bitPosi + 24;
	  }
	  else
	  {
		LPC_SYSCON->PINSEL[6] = bitPosi;
	  }
	  NVIC_EnableIRQ(PIN_INT6_IRQn);
	break;
	case CHANNEL7:
	  if ( portNum )
	  {
		LPC_SYSCON->PINSEL[7] = bitPosi + 24;
	  }
	  else
	  {
		LPC_SYSCON->PINSEL[7] = bitPosi;
	  }
	  NVIC_EnableIRQ(PIN_INT7_IRQn);
	break;
	default:
	  break;
  }
  if ( sense == 0 )
  {
	LPC_GPIO_PIN_INT->ISEL &= ~(0x1<<channelNum);	/* Edge trigger */
	if ( event == 0 )
	{
	  LPC_GPIO_PIN_INT->IENF |= (0x1<<channelNum);	/* faling edge */
	}
	else
	{
	  LPC_GPIO_PIN_INT->IENR |= (0x1<<channelNum);	/* Rising edge */
	}
  }
  else
  {
	LPC_GPIO_PIN_INT->ISEL |= (0x1<<channelNum);	/* Level trigger. */
	LPC_GPIO_PIN_INT->IENR |= (0x1<<channelNum);	/* Level enable */
	if ( event == 0 )
	{
	  LPC_GPIO_PIN_INT->IENF &= ~(0x1<<channelNum);	/* active-low */
	}
	else
	{
	  LPC_GPIO_PIN_INT->IENF |= (0x1<<channelNum);	/* active-high */
	}
  }
  return;
}

/*****************************************************************************
** Function name:		GPIOPinIntEnable
**
** Descriptions:		Enable Interrupt
**
** parameters:			channel num, event(0 is falling edge, 1 is rising edge)
** Returned value:		None
**
*****************************************************************************/
void GPIOPinIntEnable( uint32_t channelNum, uint32_t event )
{
  if ( !( LPC_GPIO_PIN_INT->ISEL & (0x1<<channelNum) ) )
  {
	if ( event == 0 )
	{
	  LPC_GPIO_PIN_INT->SIENF |= (0x1<<channelNum);	/* faling edge */
	}
	else
	{
	  LPC_GPIO_PIN_INT->SIENR |= (0x1<<channelNum);	/* Rising edge */
	}
  }
  else
  {
	LPC_GPIO_PIN_INT->SIENR |= (0x1<<channelNum);	/* Level */
  }
  return;
}

/*****************************************************************************
** Function name:		GPIOPinIntDisable
**
** Descriptions:		Disable Interrupt
**
** parameters:			channel num, event(0 is falling edge, 1 is rising edge)
**
** Returned value:		None
**
*****************************************************************************/
void GPIOPinIntDisable( uint32_t channelNum, uint32_t event )
{
  if ( !( LPC_GPIO_PIN_INT->ISEL & (0x1<<channelNum) ) )
  {
	if ( event == 0 )
	{
	  LPC_GPIO_PIN_INT->CIENF |= (0x1<<channelNum);	/* faling edge */
	}
	else
	{
	  LPC_GPIO_PIN_INT->CIENR |= (0x1<<channelNum);	/* Rising edge */
	}
  }
  else
  {
	LPC_GPIO_PIN_INT->CIENR |= (0x1<<channelNum);	/* Level */
  }
  return;
}
