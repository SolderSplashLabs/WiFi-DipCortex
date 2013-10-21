/*
  ____        _     _           ____        _           _		 _          _
 / ___|  ___ | | __| | ___ _ __/ ___| _ __ | | __ _ ___| |__	| |    __ _| |__  ___
 \___ \ / _ \| |/ _` |/ _ \ '__\___ \| '_ \| |/ _` / __| '_ \	| |   / _` | '_ \/ __|
  ___) | (_) | | (_| |  __/ |   ___) | |_) | | (_| \__ \ | | |	| |__| (_| | |_) \__ \
 |____/ \___/|_|\__,_|\___|_|  |____/| .__/|_|\__,_|___/_| |_|	|_____\__,_|_.__/|___/
                                     |_|
 (C)SolderSplash Labs 2013 - www.soldersplash.co.uk - C. Matthews - R. Steel


	@file     console.c
	@author   Carl Matthews (soldersplash.co.uk)
	@date     03 July 2013

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


*/

#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include "SolderSplashLpc.h"
#include "usbcdc_fifo.h"

#define _CONSOLE_
#include "console.h"

// ------------------------------------------------------------------------------------------------------------
/*!
    @brief Console_Task
*/
// ------------------------------------------------------------------------------------------------------------
void Console_Task ( void )
{
	Console_DataIn();
	//UsbCdcRxFifo_GetString(ConsoleInBuffer[ConsoleBufferPos], CONSOLE_MAX_INPUT-ConsoleBufferPos);
}

// ------------------------------------------------------------------------------------------------------------
/*!
    @brief Console_DataIn - Called whenever data arrives
*/
// ------------------------------------------------------------------------------------------------------------
void Console_DataIn ( void )
{
uint8_t i = 0;
bool processBuffer = false;
char newChar;
static uint8_t escapeCmdCharCnt = 0;
static bool firstTime = TRUE;

	if (! actioningCommand)
	{
		while ( UsbCdcRxFifo_GetChar(&newChar) )
		{
			// Really really basic escape code processing to catch the arrow keys
			if ( escapeCmdCharCnt )
			{
				switch ( escapeCmdCharCnt )
				{
					case 1 :
						if ('[' == newChar)
						{
							escapeCmdCharCnt ++;
						}
						else
						{
							escapeCmdCharCnt = 0;
						}
					break;
					case 2 :
						if ('A' == newChar)
						{
							// move up
							if ( ConsoleCurBuf )
							{
								ConsoleCurBuf = 0;
							}
							else
							{
								ConsoleCurBuf = 1;
							}

							UsbCdcTxFifo_AddBytes((char *)CONSOLE_CLEARLINE, sizeof(CONSOLE_CLEARLINE));
							UsbCdcTxFifo_AddBytes("\r>",2);

						}
						else if ('B' == newChar)
						{
							// Down
							if ( ConsoleCurBuf )
							{
								ConsoleCurBuf = 0;
							}
							else
							{
								ConsoleCurBuf = 1;
							}

							UsbCdcTxFifo_AddBytes((char *)CONSOLE_CLEARLINE, sizeof(CONSOLE_CLEARLINE));
							UsbCdcTxFifo_AddBytes("\r>",2);
						}
						else if ('C' == newChar)
						{
							// Right
						}
						else if ('D' == newChar)
						{
							//left
						}

						ConsoleBufferPos = strlen(ConsoleBuffer[ConsoleCurBuf]);
						ConsolePrintf(ConsoleBuffer[ConsoleCurBuf]);

						escapeCmdCharCnt = 0;
					break;
				}
			}
			else
			{
				if (( '\b' == newChar ) || ( 0x7F == newChar ))
				{
					// Remove the last character if we can
					if (ConsoleBufferPos)
					{
						ConsoleBufferPos --;
						// echo it back
						UsbCdcTxFifo_AddBytes(&newChar, 1);
					}
				}
				else if ('\e' == newChar )
				{
					// escape recvd
					escapeCmdCharCnt = 1;

					// Clear the screen & move cursor to home
					//ConsolePrintf("\e[2J\e[H \r\n>");
					break;
				}
				else if ( '\r' == newChar )
				{
					// user hit enter time to process what we have
					ConsoleBuffer[ConsoleCurBuf][ConsoleBufferPos] = 0;

					if (ConsoleBufferPos)
					{
						// There's something in the buffer
						processBuffer = true;
					}
					else
					{
						// Show the user a new line
						if ( firstTime )
						{
							ConsolePrintf(WELCOME_MSG);
							ConsolePrintf("Type '?' for help\r\n");
							firstTime = false;
						}

						ConsolePrintf("\r\n>");
					}

					break;
				}
				else
				{
					if ( ConsoleBufferPos >= CONSOLE_MAX_INPUT-1 )
					{
						// Full, no echo
					}
					else
					{
						ConsoleBuffer[ConsoleCurBuf][ConsoleBufferPos] = newChar;
						ConsoleBufferPos++;
						// make sure theres always a null byte after the string
						ConsoleBuffer[ConsoleCurBuf][ConsoleBufferPos] = 0;
						UsbCdcTxFifo_AddBytes(&newChar, 1);
					}
				}

				i++;
			}
		}

		if ( processBuffer )
		{
			actioningCommand = true;
			Console_Process();

			//switch buffers
			if ( ConsoleCurBuf )
			{
				ConsoleCurBuf = 0;
			}
			else
			{
				ConsoleCurBuf = 1;
			}

			ConsoleBufferPos = 0;
			ConsoleBuffer[ConsoleCurBuf][ConsoleBufferPos] = 0;
			actioningCommand = false;

			ConsolePrintf("\r\n>");
		}
	}
}

// ------------------------------------------------------------------------------------------------------------
/*!
    @brief Console_Process - Process the InBuffer for commands and arguments
*/
// ------------------------------------------------------------------------------------------------------------
void Console_Process ( void )
{
char *buffer = ConsoleBuffer[ConsoleCurBuf];
bool nextCharIsParamStart = false;
uint16_t i = 0;
uint16_t argc = 0;
static char *argv[CONSOLE_MAX_ARGS+1];

	// we always assume the first argument is the command
	argv[0] = buffer;
	argc ++;

	// Loop until we get a zero or we get to the end of the input buffer
	while ((buffer[i]) && (i<ConsoleBufferPos))
	{
		if (' ' == buffer[i])
		{
			// found a space, replace with a 0 so that when using string functions with the argument pointer
			// they know where the argument ends
			buffer[i] = 0;

			nextCharIsParamStart = true;
		}
		else if (0 == buffer[i])
		{
			// Found the end of the Console string
			argv[argc] = 0;
			break;
		}
		else if ( nextCharIsParamStart )
		{
			argv[argc] = &buffer[i];
			argc ++;
			nextCharIsParamStart = false;

			if ( argc >= CONSOLE_MAX_ARGS)
			{
				break;
			}
		}

		i ++;
	}

	ConsoleBufferPos = 0;

	// Now is there a function to handle this command?
	i = 0;
	while (commandList[i].Command)
	{
		if(!strcmp(argv[0], commandList[i].Command) )
		{
			if (commandList[i].ConsoleFncPtr)
			{
				if ( commandList[i].ConsoleFncPtr(argc, argv) )
				{

				}
				else
				{
					ConsolePrintf("\r\nCommand parameter error");
				}
			}
			break;
		}
		i++;
	}

	if (!commandList[i].Command)
	{
		ConsolePrintf("\r\nUnknown command");
	}

}

// ------------------------------------------------------------------------------------------------------------
/*!
    @brief Console_Init -
*/
// ------------------------------------------------------------------------------------------------------------
void Console_Init ( CONSOLE_CMDS_STRUCT *appCommandList )
{
	commandList = appCommandList;
}

// ------------------------------------------------------------------------------------------------------------
/*!
    @brief ConsoleVPrintf
*/
// ------------------------------------------------------------------------------------------------------------
void ConsoleVPrintf(const char *format, va_list args)
{
	vsnprintf(&outstring[0], sizeof(outstring), format, args);
	UsbCdcTxFifo_AddBytes((char *)&outstring[0], strlen(outstring));
}


// ------------------------------------------------------------------------------------------------------------
/*!
    @brief ConsoleInsertPrintf - inserts this new line and then puts back any half entered data
*/
// ------------------------------------------------------------------------------------------------------------
void ConsoleInsertPrintf(const char *format, ...)
{
va_list args;

	// get the var args
	va_start(args, format);

	// clear line and move cursor to the start
	UsbCdcTxFifo_AddBytes((char *)CONSOLE_CLEARLINE, sizeof(CONSOLE_CLEARLINE));

	UsbCdcTxFifo_AddBytes("\r",1);

	// attribute, fore, back
	ConsolePrintf("\x1b[%d;%d;%dm", 2, CONSOLE_FORE_GREEN, CONSOLE_BACK_BLACK);

	// Print the new data
	ConsoleVPrintf(format, args);

	ConsolePrintf("\x1b[%d;%d;%dm", 0, CONSOLE_FORE_WHITE, CONSOLE_BACK_BLACK);

	// Now put back any data we cleared
	UsbCdcTxFifo_AddBytes("\r\n>",3);
	if ( ConsoleBufferPos )
	{
		ConsolePrintf(ConsoleBuffer[ConsoleCurBuf], args);
	}

	va_end(args);
}

// ------------------------------------------------------------------------------------------------------------
/*!
    @brief ConsoleInsertPrintf - inserts this new line and then puts back any half entered data
*/
// ------------------------------------------------------------------------------------------------------------
void ConsoleInsertDebugPrintf(const char *format, ...)
{
va_list args;

	// get the var args
	va_start(args, format);

	// clear line and move cursor to the start
	UsbCdcTxFifo_AddBytes((char *)CONSOLE_CLEARLINE, sizeof(CONSOLE_CLEARLINE));

	UsbCdcTxFifo_AddBytes("\r",1);

	// attribute, fore, back
	ConsolePrintf("\x1b[%d;%d;%dm", 2, CONSOLE_FORE_CYAN, CONSOLE_BACK_BLACK);

	// Print the new data
	ConsoleVPrintf(format, args);

	ConsolePrintf("\x1b[%d;%d;%dm", 0, CONSOLE_FORE_WHITE, CONSOLE_BACK_BLACK);

	// Now put back any data we cleared
	UsbCdcTxFifo_AddBytes("\r\n>",3);
	if ( ConsoleBufferPos )
	{
		ConsolePrintf(ConsoleBuffer[ConsoleCurBuf], args);
	}

	va_end(args);
}

// ------------------------------------------------------------------------------------------------------------
/*!
    @brief ConsolePrintf -
*/
// ------------------------------------------------------------------------------------------------------------
void ConsolePrintf(const char *format, ...)
{
va_list args;

	va_start(args, format);
	ConsoleVPrintf( format, args );
	va_end(args);
}

