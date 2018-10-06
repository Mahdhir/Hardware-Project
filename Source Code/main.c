/*
 * Max_Final_max_without.c
 *
 * Created: 16-May-18 4:36:20 PM
 * Author : Mahdhi
 */ 
#define F_CPU 1000000UL //1Mhz Clock Speed
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define BIT0 0b00000001
#define	BIT1 0b00000010
#define BIT2 0b00000100
#define BIT3 0b00001000
#define BIT4 0b00010000
#define BIT5 0b00100000
#define BIT6 0b01000000
#define BIT7 0b10000000

#define CS_OUTP PORTB  //Chip select output port selection
#define CS_DDR DDRB		//Chip select output port data direction selection
#define CS_PIN PORTB2   //Chip select output pin selection

#define SPI_BUSY_WAIT() while(!(SPSR & (1<<SPIF))) //Wait until transmission complete
#define SEL_MAX6675() ((CS_OUTP) &= ~(1<<CS_PIN))	//Enable Chip select pin
#define DESEL_MAX6675() ((CS_OUTP) |= (1<<CS_PIN))	//Disable Chip select pin
#define spi_get_byte() spi_put_byte(0)				//Data transmission
	
int temperature,value;

void routine1(void);
void cutter(void);
void frying(void);
void ready(void);
int flag=0,flaga=0,temp_flag=0;

uint8_t spi_put_byte(uint8_t data)
{
	SPDR = data;
	SPI_BUSY_WAIT();
	return SPDR;
	
}

int get_max6675_temp(void)
{
	int act_temp;
	SEL_MAX6675();
	act_temp = (spi_get_byte() << 8);
	act_temp |= spi_get_byte();
	DESEL_MAX6675();
	return act_temp >> 3;
}

void check_temp(void)
{
	value = temperature * 0.25;
	//value = ((temperature/10)+405)/0.2;
	if(temp_flag==1)
	{
		//Maintaining temperature within 200 and 180 celsius 
		if(value < 180)
		PORTD|= (1<<PORTD3);
		
		if(value > 200)
		PORTD &= ~BIT3;
		
	}
	
}
void init_spi(void)
{
	//Port Initialization
	DDRB |= (1<<PORTB5)|(1<<PORTB2); // Output SCK and CS
	/* set to < 4.3MHz; Mode 0 */
	SPCR = (1<<SPE)|(1<<SPR0)| (1<<MSTR); // Enabling SPI,Master and SPR0 register
	sei();	//Enabling global interrupt
}


//Interrupt Service Routine
ISR(PCINT0_vect)
{	
	if(!bit_is_set(PINB,0))
	{
		temp_flag=2;
		PORTD = 0x00;
		PORTC=0x00;
		flag=0;
		flaga=0;
	}
}

void ready (void)
{
	if(flag==2)
			{
				flaga=3;
				temp_flag=2;
				PORTD = 0x00;
				PORTC=BIT3;//ready	
			}
}

void frying (void)
{
	if(flag==1)
	{
		flag=2;
		PORTC=BIT2;//frying
		PORTD &= ~BIT1;
		PORTD |= BIT0| BIT2;
		int i=0,j;
		j=25;//25 minutes of frying
			for(i=0;i<j*60;i++)
			{
				temperature = get_max6675_temp();
				check_temp();
				if(flag!=0)
				_delay_ms(1000);
				else break;
			}
	}
	
}

void cutter (void)
{
	if(!(bit_is_set(PINB,1)) && flag==0)
	{	
		flag=1;	
		PORTD |= BIT1 | BIT2;
		PORTC=BIT1;	//cutting
		int i=0,j;
		j=1;//1 minute of cutting
			for(i=0;i<j*60;i++)
			{
				temperature = get_max6675_temp();
				check_temp();
				if(flag!=0)
				_delay_ms(1000);
				else break;
			}
	}
			
}

void routine1 (void)
{
	
	if(bit_is_set(PINB,0)&& flaga!=3)
	{
		temperature = get_max6675_temp();
		check_temp();
		PORTC=BIT4;//stop
		temp_flag=1;
		cutter();//Call Cutter function
		frying();//Call frying function
		ready();//Call ready function
	}
	if(!(bit_is_set(PINB,0)))
	{
		temp_flag=2;
		PORTD =0x00;
		PORTC=0x00;
		flag=0;
		flaga=0;
	}
	
}



int main(void)
{

	DDRB=0x00; //input B port
	DDRC=~BIT0; //output C port
	DDRD=0xff; //output D port
	
	PCMSK0=0xff;	//Pin Change Interrupt registers
	PCICR = BIT0 ;
	init_spi(); //Initialize SPI
    while (1) 
    {
		temperature = get_max6675_temp();
		check_temp();	
		routine1();  //Call Routine 1
    }	
}

