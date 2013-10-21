/*
  ____        _     _           ____        _           _		 _          _
 / ___|  ___ | | __| | ___ _ __/ ___| _ __ | | __ _ ___| |__	| |    __ _| |__  ___
 \___ \ / _ \| |/ _` |/ _ \ '__\___ \| '_ \| |/ _` / __| '_ \	| |   / _` | '_ \/ __|
  ___) | (_) | | (_| |  __/ |   ___) | |_) | | (_| \__ \ | | |	| |__| (_| | |_) \__ \
 |____/ \___/|_|\__,_|\___|_|  |____/| .__/|_|\__,_|___/_| |_|	|_____\__,_|_.__/|___/
                                     |_|
 (C)SolderSplash Labs 2013 - www.soldersplash.co.uk - C. Matthews - R. Steel


	@file     main.c
	@author   Carl Matthews (soldersplash.co.uk)
	@date     19 May 2013

    @section LICENSE

	Software License Agreement (BSD License)

    Copyright (c) 2013, C. Matthews - R. Steel (soldersplash.co.uk)
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    1. Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    3. Neither the name of the copyright holders nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ''AS IS'' AND ANY
    EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


	@section DESCRIPTION

	Based upon NXP example application
	http://docs.lpcware.com/usbromlib/v1.0/_page__developing_with_u_s_b_d.html

	@section TODO

	Add USB Serial Number
	Work out how to detect disconnects

*/

/*
***********************************************************************
*   Copyright(C) 2011, NXP Semiconductor
*   All rights reserved.
*
* Software that is described herein is for illustrative purposes only
* which provides customers with programming information regarding the
* products. This software is supplied "AS IS" without any warranties.
* NXP Semiconductors assumes no responsibility or liability for the
* use of the software, conveys no license or title under any patent,
* copyright, or mask work right to the product. NXP Semiconductors
* reserves the right to make changes in the software without
* notification. NXP Semiconductors also make no representation or
* warranty that such application will be suitable for the specified
* use without further testing or modification.
* Permission to use, copy, modify, and distribute this software and its
* documentation is hereby granted, under NXP Semiconductors'
* relevant copyright in the software, without fee, provided that it
* is used in conjunction with NXP Semiconductors microcontrollers.  This
* copyright, permission, and disclaimer notice must appear in all copies of
* this code.
**********************************************************************/

#include <string.h>
#include "SolderSplashLpc.h"
#include "power_api.h"
#include "mw_usbd_rom_api.h"

#include "usbcdc_fifo.h"
#include "usbcdc.h"

//#define UART_BRIDGE 0

bool UsbConnected = false;

extern uint8_t VCOM_DeviceDescriptor[];
extern uint8_t VCOM_StringDescriptor[];
extern uint8_t VCOM_ConfigDescriptor[];

USBD_API_T* pUsbApi;

/* VCOM defines */
#define VCOM_BUFFERS    		4
#define VCOM_BUF_EMPTY_INDEX  	(0xFF)
#define VCOM_BUF_FREE  			0
#define VCOM_BUF_ALLOC  		1
#define VCOM_BUF_USBTXQ 		2
#define VCOM_BUF_UARTTXQ  		3
#define VCOM_BUF_ALLOCU  		4

struct VCOM_DATA;
typedef void (*VCOM_SEND_T) (struct VCOM_DATA* pVcom);

typedef struct VCOM_DATA {
  USBD_HANDLE_T hUsb;
  USBD_HANDLE_T hCdc;
  uint8_t* rxBuf;
  uint8_t* txBuf;
  volatile uint8_t ser_pos;
  volatile uint16_t rxlen;
  volatile uint16_t txlen;
  VCOM_SEND_T send_fn;
  volatile uint32_t sof_counter;
  volatile uint32_t last_ser_rx;
  volatile uint16_t break_time;
  volatile uint16_t usbrx_pend;
} VCOM_DATA_T; 

VCOM_DATA_T g_vCOM;
ErrorCode_t VCOM_bulk_out_hdlr(USBD_HANDLE_T hUsb, void* data, uint32_t event); 


// ------------------------------------------------------------------------------------------------------------
/*!
    @brief USB_pin_clk_init - Set up the USB specific clocks
*/
// ------------------------------------------------------------------------------------------------------------
void USB_pin_clk_init(void)
{
  /* Enable AHB clock to the USB block and USB RAM. */
  LPC_SYSCON->SYSAHBCLKCTRL |= ((0x1<<14)|(0x1<<27));

  /* Pull-down is needed, or internally, VBUS will be floating. This is to
  address the wrong status in VBUSDebouncing bit in CmdStatus register.  */
  LPC_IOCON->PIO0_3   &= ~0x1F; 
  //  LPC_IOCON->PIO0_3   |= ((0x1<<3)|(0x01<<0));	/* Secondary function VBUS */
  LPC_IOCON->PIO0_3   |= (0x01<<0);			/* Secondary function VBUS */
  LPC_IOCON->PIO0_6   &= ~0x07;
  LPC_IOCON->PIO0_6   |= (0x01<<0);			/* Secondary function SoftConn */
}

// ------------------------------------------------------------------------------------------------------------
/*!
    @brief
*/
// ------------------------------------------------------------------------------------------------------------
void VCOM_usb_send(VCOM_DATA_T* pVcom)
{
	/*
  pVcom->txlen -= pUsbApi->hw->WriteEP (pVcom->hUsb, USB_CDC_EP_BULK_IN, 
    pVcom->txBuf, pVcom->txlen);   
  */
}

// ------------------------------------------------------------------------------------------------------------
/*!
    @brief
*/
// ------------------------------------------------------------------------------------------------------------
ErrorCode_t VCOM_sof_event(USBD_HANDLE_T hUsb)
{
	/*
  VCOM_DATA_T* pVcom = &g_vCOM;
//  uint8_t lcr;
  uint32_t diff = pVcom->sof_counter - pVcom->last_ser_rx;

  pVcom->sof_counter++;

  if ( pVcom->last_ser_rx && (diff > 5))
  {
	  VCOM_usb_send(pVcom);
  }
*/
  return LPC_OK;
}

// ------------------------------------------------------------------------------------------------------------
/*!
    @brief UsbCdc_TriggerTx
*/
// ------------------------------------------------------------------------------------------------------------
void UsbCdc_TriggerTx ( void )
{
	/*
uint8_t bufferSize = 0;

	// TODO : for now we use the constant interrupt method as it's more reliable


	if ( UsbConnected )
	{
		bufferSize = UsbCdcTxFifo_CopyTo(g_vCOM.txBuf, USB_HS_MAX_BULK_PACKET);

		if ( bufferSize )
		{
			// Something to send
			if ( bufferSize != (pUsbApi->hw->WriteEP (g_vCOM.hUsb, USB_CDC_EP_BULK_IN, g_vCOM.txBuf, bufferSize)) )
			{
				// it wasn't sent!
				// TODO : Something
			}
		}

		// This zero length transaction that will trigger the bulk out handler, needs testing
		//pUsbApi->hw->WriteEP(g_vCOM.hUsb, USB_CDC_EP_BULK_IN, NULL, 0);
	}
	*/
}

// ------------------------------------------------------------------------------------------------------------
/*!
    @brief - I think this is triggered after a send, sending 0 also re-triggers the function, this lets us stream data out quickly
*/
// ------------------------------------------------------------------------------------------------------------
ErrorCode_t VCOM_bulk_in_hdlr(USBD_HANDLE_T hUsb, void* data, uint32_t event)
{
uint8_t bufferSize = 0;

	if (USB_EVT_IN == event)
	{
		bufferSize = UsbCdcTxFifo_CopyTo(g_vCOM.txBuf, USB_HS_MAX_BULK_PACKET);

		// TODO : writing zero when we have nothing to send keeps the regular interrupt going, pulling any data
		// that we add to the fifo
		pUsbApi->hw->WriteEP (g_vCOM.hUsb, USB_CDC_EP_BULK_IN, g_vCOM.txBuf, bufferSize );

		/*
		if ( bufferSize )
		{
			// Something to send
			if ( bufferSize != (pUsbApi->hw->WriteEP (g_vCOM.hUsb, USB_CDC_EP_BULK_IN, g_vCOM.txBuf, bufferSize)) )
			{
				// it wasn't sent!
				// TODO : Something
			}
		}
		*/
	}

	return LPC_OK;
}

// ------------------------------------------------------------------------------------------------------------
/*!
    @brief
*/
// ------------------------------------------------------------------------------------------------------------
ErrorCode_t VCOM_bulk_out_hdlr(USBD_HANDLE_T hUsb, void* data, uint32_t event) 
{
  VCOM_DATA_T* pVcom = (VCOM_DATA_T*) data;

  if (USB_EVT_OUT == event)
  {
	  pVcom->rxlen = pUsbApi->hw->ReadEP(hUsb, USB_CDC_EP_BULK_OUT, pVcom->rxBuf);

	  UsbCdcRxFifo_AddBytes((char *)pVcom->rxBuf, pVcom->rxlen );

	  //CdcDataIn( pVcom->rxBuf, pVcom->rxlen );
  }
  return LPC_OK;
}

// ------------------------------------------------------------------------------------------------------------
/*!
    @brief UsbCdcTx Send data via the serial port
*/
// ------------------------------------------------------------------------------------------------------------
/*
uint8_t UsbCdcTx( uint8_t *buffer, uint8_t len )
{
uint8_t *pbuf;
uint8_t i = 0;
uint8_t result = 0;

pbuf = g_vCOM.txBuf;

	if ( len < USB_HS_MAX_BULK_PACKET)
	{
		memcpy(pbuf, buffer, len);

		// Send it
		result = pUsbApi->hw->WriteEP (g_vCOM.hUsb, USB_CDC_EP_BULK_IN, g_vCOM.txBuf, g_vCOM.txlen);
	}
	else
	{
		result = 0;
	}

	g_vCOM.last_ser_rx = g_vCOM.sof_counter;

	return ( result );
}
*/

/**************************************************************************/
/*!
    @brief  USB_IRQHandler
*/
/**************************************************************************/
void USB_IRQHandler(void)
{
	// Pass the interrupt to the ROM
	pUsbApi->hw->ISR(g_vCOM.hUsb);
}

/**************************************************************************/
/*!
    @brief  Callback when the USB Set Configured request is received
*/
/**************************************************************************/
ErrorCode_t usb_cdc_configured(USBD_HANDLE_T hUsb)
{
uint8_t dummy=0;

	UsbConnected = true;

	// initial packet for IN endpoint, will not work if omitted
	pUsbApi->hw->WriteEP(hUsb, USB_CDC_EP_BULK_IN, &dummy, 1);

	UsbCdcTxFifo_Clear();

  return LPC_OK;
}

// ------------------------------------------------------------------------------------------------------------
/*!
    @brief UsbCdcInit
*/
// ------------------------------------------------------------------------------------------------------------
void UsbCdcInit (void)
{
  USBD_API_INIT_PARAM_T usb_param;
  USBD_CDC_INIT_PARAM_T cdc_param;
  USB_CORE_DESCS_T desc;
  USBD_HANDLE_T hUsb, hCdc;
  ErrorCode_t ret = LPC_OK;
  uint32_t ep_indx;

  // get USB API table pointer
  pUsbApi = (USBD_API_T*)((*(ROM **)(0x1FFF1FF8))->pUSBD);

  // enable clocks and pinmux
  USB_pin_clk_init();

  /* initilize call back structures */
  memset((void*)&usb_param, 0, sizeof(USBD_API_INIT_PARAM_T));
  usb_param.usb_reg_base = LPC_USB_BASE;
  usb_param.mem_base = 0x20004000;
  usb_param.mem_size = 0x7FF;
  usb_param.max_num_ep = 3;
  usb_param.USB_Configure_Event = usb_cdc_configured;

  /* init CDC params */
  memset((void*)&cdc_param, 0, sizeof(USBD_CDC_INIT_PARAM_T));
  memset((void*)&g_vCOM, 0, sizeof(VCOM_DATA_T));

  // user defined functions
  usb_param.USB_SOF_Event = VCOM_sof_event;
  cdc_param.SendBreak = VCOM_sof_event;

  /* Initialize Descriptor pointers */
  memset((void*)&desc, 0, sizeof(USB_CORE_DESCS_T));
  desc.device_desc = (uint8_t *)&VCOM_DeviceDescriptor[0];
  desc.string_desc = (uint8_t *)&VCOM_StringDescriptor[0];
  desc.full_speed_desc = (uint8_t *)&VCOM_ConfigDescriptor[0];
  desc.high_speed_desc = (uint8_t *)&VCOM_ConfigDescriptor[0];

  /* USB Initialization */
  ret = pUsbApi->hw->Init(&hUsb, &desc, &usb_param);  
  if (ret == LPC_OK) {

    // init CDC params
    cdc_param.mem_base = usb_param.mem_base;
    cdc_param.mem_size = usb_param.mem_size;
    cdc_param.cif_intf_desc = (uint8_t *)&VCOM_ConfigDescriptor[USB_CONFIGUARTION_DESC_SIZE];
    cdc_param.dif_intf_desc = (uint8_t *)&VCOM_ConfigDescriptor[USB_CONFIGUARTION_DESC_SIZE + \
                               USB_INTERFACE_DESC_SIZE + 0x0013 + USB_ENDPOINT_DESC_SIZE ];

    ret = pUsbApi->cdc->init(hUsb, &cdc_param, &hCdc);

    if (ret == LPC_OK)
    {
      // store USB handle
      g_vCOM.hUsb = hUsb;
      g_vCOM.hCdc = hCdc;
      g_vCOM.send_fn = VCOM_usb_send;

      // allocate transfer buffers
      g_vCOM.rxBuf = (uint8_t*)(cdc_param.mem_base + (0 * USB_HS_MAX_BULK_PACKET));
      g_vCOM.txBuf = (uint8_t*)(cdc_param.mem_base + (1 * USB_HS_MAX_BULK_PACKET));
      cdc_param.mem_size -= (4 * USB_HS_MAX_BULK_PACKET);

      // Register endpoint interrupt handler
      ep_indx = (((USB_CDC_EP_BULK_IN & 0x0F) << 1) + 1);
      ret = pUsbApi->core->RegisterEpHandler (hUsb, ep_indx, VCOM_bulk_in_hdlr, &g_vCOM);
      if (ret == LPC_OK)
      {
        /* register endpoint interrupt handler */
        ep_indx = ((USB_CDC_EP_BULK_OUT & 0x0F) << 1);
        ret = pUsbApi->core->RegisterEpHandler (hUsb, ep_indx, VCOM_bulk_out_hdlr, &g_vCOM);
        if (ret == LPC_OK)
        {
          /* enable IRQ */
          NVIC_EnableIRQ(USB_IRQ_IRQn); //  enable USB0 interrrupts 
          NVIC_SetPriority(USB_IRQ_IRQn, 0);

          /* USB Connect */
          pUsbApi->hw->Connect(hUsb, 1);
        }
      }
    }    
  }

}

/**********************************************************************
 **                            End Of File
 **********************************************************************/
