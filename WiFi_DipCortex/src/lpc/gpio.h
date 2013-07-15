
#define CHANNEL0	0
#define CHANNEL1	1
#define CHANNEL2	2
#define CHANNEL3	3
#define CHANNEL4	4
#define CHANNEL5	5
#define CHANNEL6	6
#define CHANNEL7	7

#define PORT0		0
#define PORT1		1

#define GROUP0		0
#define GROUP1		1

void GPIOPinIntEnable( uint32_t channelNum, uint32_t event );
void GPIOPinIntDisable( uint32_t channelNum, uint32_t event );
void GPIOSetPinInterrupt( uint32_t channelNum, uint32_t portNum, uint32_t bitPosi,
		uint32_t sense, uint32_t event );;
