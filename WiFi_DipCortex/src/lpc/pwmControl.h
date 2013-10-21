
#define PWM_DEFAULT_PERIOD			48000			// 1KHz - before the prescaler
#define PWM_PRESCALE				1			// Clock Ticks are needed before incrementing the count register


#define PWM_NO_OF					3

#define	PWM_MASK_RED				1
#define	PWM_MASK_GREEN				2
#define	PWM_MASK_BLUE				4

// Green - CT32B1_MAT0
#define PWM1_PORT		0
#define PWM1_PIN		13
#define PWM1_MASK		1<<PWM1_PIN
#define PWM1_IOCON		{LPC_IOCON->TDO_PIO0_13 = 3 | 1<<7;}
#define PWM1_DUTY_REG	&(LPC_CT32B1->MR0)

// Red - CT32B1_MAT1
#define PWM2_PORT		0
#define PWM2_PIN		14
#define PWM2_MASK		1<<PWM2_PIN
#define PWM2_IOCON		{LPC_IOCON->TRST_PIO0_14 = 3 | 1<<7;}
#define PWM2_DUTY_REG	&(LPC_CT32B1->MR1)

// CT32B1_MAT2 Period Register

// Blue - CT32B0_MAT3
#define PWM3_PORT		0
#define PWM3_PIN		16
#define PWM3_MASK		1<<PWM3_PIN
#define PWM3_IOCON		{LPC_IOCON->PIO0_16 = 2 |  1<<7;}
#define PWM3_DUTY_REG	&(LPC_CT32B1->MR3)



void Pwm_Init ( void );
void Pwm_SetDuty ( uint8_t pwmMask, uint32_t duty );

void Pwm_On ( void );
void Pwm_DutyIncrease ( uint8_t pwmMask, uint32_t maxDuty );
void Pwm_DutyDecrease ( uint8_t pwmMask, uint32_t minDuty );
uint32_t Pwm_GetFreq ( void );

uint32_t Pwm_GetDuty ( uint8_t channel );

void Pwm_DutyStep ( uint8_t pwmMask, uint32_t targetDuty );

#ifdef _PWM_CON_

// PWM Duty control array
volatile uint32_t* Pwm_Duty[PWM_NO_OF] = { PWM1_DUTY_REG, PWM2_DUTY_REG, PWM3_DUTY_REG };

#endif
