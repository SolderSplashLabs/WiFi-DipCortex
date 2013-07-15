#include "SolderSplashLpc.h"

void DelayUs ( uint32_t usec )
{
uint32_t i;
    for (i = (SystemCoreClock / 1000000) * usec; i; i--)
    {
      __NOP();
    }
}
