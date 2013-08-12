
#define CONSOLE_MAX_INPUT	75
#define CONSOLE_MAX_ARGS	5

#define CONSOLE_FORE_BLACK		30
#define CONSOLE_FORE_RED		31
#define CONSOLE_FORE_GREEN		32
#define CONSOLE_FORE_YELLOW		33
#define CONSOLE_FORE_BLUE		34
#define CONSOLE_FORE_MAGENTA	35
#define CONSOLE_FORE_CYAN		36
#define CONSOLE_FORE_WHITE		37

#define CONSOLE_BACK_BLACK		40
#define CONSOLE_BACK_RED		41
#define CONSOLE_BACK_GREEN		42
#define CONSOLE_BACK_YELLOW		43
#define CONSOLE_BACK_BLUE		44
#define CONSOLE_BACK_MAGENTA	45
#define CONSOLE_BACK_CYAN		46
#define CONSOLE_BACK_WHITE		47

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
void ConsoleInsertDebugPrintf(const char *format, ...);

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

char ConsoleBuffer[2][CONSOLE_MAX_INPUT];
uint8_t ConsoleCurBuf = 0;
uint16_t ConsoleBufferPos = 0;
bool actioningCommand = false;


const char CONSOLE_CLEARLINE[] = {0x1b, '[', '1', 'K'};
const char CONSOLE_STARTLINE[] = {0x1b, 'E'};

char outstring[512];

#endif

