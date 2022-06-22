#include "stm32f4xx.h"

#define RS 0x01     /* PA0 mask for LCD reg select */
#define EN 0x02     /* PA1 mask for LCD enable */

void delayMs(int n);

void LCD_print(char data[]);
void LCD_nibble_write(char data, unsigned char control);
void LCD_command(unsigned char command);
void LCD_data(char data);
void LCD_init(void);
void PORTS_init(void);

int static current_RPM = 0;
int static saved_RPM = 0;
int period;
float frequency;

int main(void) {
		int speed_control = 0; //o=off, 1=on
		uint32_t output = 0;	
		uint32_t max = 0xFFF;
		char A[8];
		double power = 0;
		int count = 0;
	
    int last = 0;
    int current;
	
		LCD_init();
	
		//set up clock
	  
		LCD_print("5RPM:");

		while(1)
		{
			speed_control = ((GPIOB -> IDR)	& 0x1);  //get mode
			
			if(speed_control == 0)
			{
				saved_RPM = 0;

				ADC1->CR2 |= 0x40000000;        // start a conversion 
				while(!(ADC1->SR & 2)) {}       // wait for conv complete 
				output = ADC1->DR;              // read conversion result 
				power = 3800*((double)output/(double)max);
				if(power < 800) power = 800;
				DAC->DHR12R1 = (uint32_t)power;
			}
			else
			{
				if(saved_RPM == 0) saved_RPM = current_RPM;

				if(saved_RPM < current_RPM) power = power-10;
				if(saved_RPM > current_RPM) power = power+10;
				if(power > 4095) power = 4095;
				if(power < 0) power = 0;
				DAC->DHR12R1 = (uint32_t)power;
			}

			////////////////////////////////////////////////////////////////////////////// get current_RPM
				while (!(TIM3->SR & 2)) {}  /* wait until input edge is captured */
        current = TIM3->CCR1;       /* read captured counter value */
        period = current - last;    /* calculate the period */
        last = current;
        frequency = 1000.0f / period;
        last = current;
					
			  if(count == 20)
				{
				count = 0;
				current_RPM = frequency*60;	
				
				sprintf (A,"5%d ", current_RPM);
				LCD_print(A);
				//delayMs(500);
				LCD_command(0x02);
				LCD_command(0x14); 
				LCD_command(0x14); 
				LCD_command(0x14); 
				LCD_command(0x14); 
				}
				count++;
		}
}

/* initialize ports then initialize LCD controller */
void LCD_init(void) 
{
    PORTS_init();

    delayMs(20);                /* LCD controller reset sequence */
    LCD_nibble_write(0x30, 0);
    delayMs(5);
    LCD_nibble_write(0x30, 0);
    delayMs(1);
    LCD_nibble_write(0x30, 0);
    delayMs(1);

    LCD_nibble_write(0x20, 0);  /* use 4-bit data mode */
    delayMs(1);
    LCD_command(0x28);          /* set 4-bit data, 2-line, 5x7 font */
    LCD_command(0x06);          /* move cursor right */
    LCD_command(0x01);          /* clear screen, move cursor to home */
    LCD_command(0x0F);          /* turn on display, cursor blinking */
}
void PORTS_init(void) 
{
    RCC->AHB1ENR =  7;             /* enable GPIOA, GPIOB and GPIOC clock */
    
		GPIOC->MODER &= ~0x0000FF00; /* clear pin mode */
		GPIOA->MODER &= ~0x00000F0F; /* clear pin mode */
	
		GPIOC->MODER =  0x00005500;   /* set pin output mode for LCD PC4-PC7 for D4-D7*/
		GPIOB->PUPDR =  0x00000001;   /* INPUT: PB0 - mode */
		GPIOA->MODER =  0x0000C305;   /* PA0-RS, PA1-En for LCD | PA4,7 analog*/
		GPIOA->BSRR  =  0x00020000;   /* turn off EN */
	
		/* setup DAC */
		RCC->APB1ENR |= 1 << 29;
		DAC->CR |= 1;
		
    /* setup ADC1 */
    RCC->APB2ENR |= 0x00000100;     /* enable ADC1 clock */
    ADC1->CR2 = 0;                  /* SW trigger */
    ADC1->SQR3 = 7;                 /* conversion sequence starts at ch 7 */
    ADC1->SQR1 = 0;                 /* conversion sequence length 1 */
    ADC1->CR2 |= 1;                 /* enable ADC1 */
		//-----------------------------------------------------------------------------------
					
			// configure PA6 as input of TIM3 CH1
			RCC->AHB1ENR |=  1;             /* enable GPIOA clock */
			GPIOA->MODER &= ~0x00003000;    /* clear pin mode */
			GPIOA->MODER |=  0x00002000;    /* set pin to alternate function */
			GPIOA->AFR[0] &= ~0x0F000000;   /* clear pin AF bits */
			GPIOA->AFR[0] |= 0x02000000;    /* set pin to AF2 for TIM3 CH1 */

			// configure TIM3 to do input capture with prescaler ...
			RCC->APB1ENR |= 2;              /* enable TIM3 clock */
			TIM3->PSC = 16000 - 1;          /* divided by 16000 */
			TIM3->CCMR1 = 0x41;             /* set CH1 to capture at every edge */
			TIM3->CCER = 0x0B;              /* enable CH 1 capture both edges */
			TIM3->CR1 = 1;                  /* enable TIM3 */
}

void LCD_nibble_write(char data, unsigned char control) 
{
    /* populate data bits */
    GPIOC->BSRR = 0x00F00000;       /* clear data bits */
    GPIOC->BSRR = data & 0xF0;      /* set data bits */

    /* set R/S bit */
    if (control & RS)
        GPIOA->BSRR = RS;
    else
        GPIOA->BSRR = RS << 16;

    /* pulse E */
    GPIOA->BSRR = EN;
    delayMs(0);
    GPIOA->BSRR = EN << 16;
}

void LCD_command(unsigned char command) 
{
    LCD_nibble_write(command & 0xF0, 0);    /* upper nibble first */
    LCD_nibble_write(command << 4, 0);      /* then lower nibble */

    if (command < 4)
        delayMs(2);         /* command 1 and 2 needs up to 1.64ms */
    else
        delayMs(1);         /* all others 40 us */
}

void LCD_data(char data) 
{
    LCD_nibble_write(data & 0xF0, RS);      /* upper nibble first */
    LCD_nibble_write(data << 4, RS);        /* then lower nibble */

    delayMs(1);
}

void LCD_print(char data[])
{
	int size = data[0] - 0x30;		//use first character as size
	for(int i=1;i<size;i++)			
	{
		if(data[i] != 0)
		{
			LCD_data(data[i]);		//print each character
			delayMs(30);
		}else break;
	}
}


/* 16 MHz SYSCLK */
void delayMs(int n) 
{
    int i;

    /* Configure SysTick */
    SysTick->LOAD = 16000;  /* reload with number of clocks per millisecond */
    SysTick->VAL = 0;       /* clear current value register */
    SysTick->CTRL = 0x5;    /* Enable the timer */

    for(i = 0; i < n; i++) {
        while((SysTick->CTRL & 0x10000) == 0) /* wait until the COUNTFLAG is set */
            { }
    }
    SysTick->CTRL = 0;      /* Stop the timer (Enable = 0) */
}