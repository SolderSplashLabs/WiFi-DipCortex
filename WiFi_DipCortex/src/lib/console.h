
#define CONSOLE_MAX_INPUT	75
#define CONSOLE_MAX_ARGS	5

typedef int (*ConsoleFn)(int argc, char *argv[]);

typedef struct CONSOLE_CMDS_STRUCT
{
	const char 	*Command;
	ConsoleFn	ConsoleFncPtr;
	const char 	*help;

} CONSOLE_CMDS_STRUCT ;

void Console_Task ( void );
void Console_DataIn ( void );
void Console_Process ( void );
void Console_Init ( CONSOLE_CMDS_STRUCT *appCommandList );
void ConsolePrintf(const char *format, ...);
void ConsoleInsertPrintf(const char *format, ...);

#ifdef _CONSOLE_

const char WELCOME_MSG[] = "\r\n ____        _     _           ____        _           _     \r\n\
/ ___|  ___ | | __| | ___ _ __/ ___| _ __ | | __ _ ___| |__ \r\n\
\\___ \\ / _ \\| |/ _` |/ _ \\ '__\\___ \\| '_ \\| |/ _` / __| '_ \\ \r\n\
 ___) | (_) | | (_| |  __/ |   ___) | |_) | | (_| \\__ \\ | | | \r\n\
|____/ \\___/|_|\\__,_|\\___|_|  |____/| .__/|_|\\__,_|___/_| |_| \r\n\
 _          _                       |_| \r\n\
| |    __ _| |__  ___ \r\n\
| |   / _` | '_ \\/ __| \r\n\
| |__| (_| | |_) \\__ \\ \r\n\
|_____\\__,_|_.__/|___/ \r\nSolderSplash Labs - WiFi DipCortex - Alpha 0.1\r\n";


CONSOLE_CMDS_STRUCT *commandList;

uint8_t ConsoleBuffer[2][CONSOLE_MAX_INPUT];
uint8_t ConsoleCurBuf = 0;
uint16_t ConsoleBufferPos = 0;
bool actioningCommand = false;


const char CONSOLE_CLEARLINE[] = {0x1b, '[', '1', 'K'};
const char CONSOLE_STARTLINE[] = {0x1b, 'E'};

#endif

