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
uint8_t newChar;
static uint8_t escapeCmdCharCnt = 0;
static bool firstTime = TRUE;

	if (! actioningCommand)
	{
		while ( UsbCdcRxFifo_GetByte(&newChar) )
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

							UsbCdcTxFifo_AddBytes(CONSOLE_CLEARLINE, sizeof(CONSOLE_CLEARLINE));
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

							UsbCdcTxFifo_AddBytes(CONSOLE_CLEARLINE, sizeof(CONSOLE_CLEARLINE));
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
char outstring[50];
uint8_t *buffer = ConsoleBuffer[ConsoleCurBuf];
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
char outstring[512];

	vsnprintf(&outstring[0], 512, format, args);
	UsbCdcTxFifo_AddBytes((uint8_t *)&outstring[0], strlen(outstring));
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
	UsbCdcTxFifo_AddBytes(CONSOLE_CLEARLINE, sizeof(CONSOLE_CLEARLINE));

	UsbCdcTxFifo_AddBytes("\r",1);

	// attribute, fore, back
	ConsolePrintf("\x1b[%d;%d;%dm", 2, 2 + 30, 0 + 40);

	// Print the new data
	ConsoleVPrintf(format, args);

	// back to white on black - attribute, fore, back
	ConsolePrintf("\x1b[%d;%d;%dm", 0, 37, 0 + 40);

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

