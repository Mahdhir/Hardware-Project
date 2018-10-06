/*
 * Hardware_Proj.c
 *
 * Created: 16-May-18 4:36:20 PM
 * Author : Mahdhi
 */ 
#define F_CPU 1000000UL //1Mhz Clock Speed
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "max6675.h"

#define BIT0 0b00000001
#define	BIT1 0b00000010
#define BIT2 0b00000100
#define BIT3 0b00001000
#define BIT4 0b00010000
#define BIT5 0b00100000
#define BIT6 0b01000000
#define BIT7 0b10000000

int temperature,value;

void routine1(void);
void cutter(void);
void frying(void);
void ready(void);
int flag=0,flaga=0,temp_flag=0;

void check_temp(void)
{
	value = temperature * 0.25;//actual temperature calculation 
	//value = ((temperature/10)+405)/0.2; //used for simulation in proteus
	if(temp_flag==1)
	{
		//Maintaining temperature within 200 and 180 celsius 
		if(value < 180)
		PORTD|= (1<<PORTD3);
		
		if(value > 200)
		PORTD &= ~BIT3;
		
	}
	
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
		j=1;//25 minutes of frying
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

	//DDRB=0x00; //input B port
	DDRC=BIT1 | BIT2 | BIT3 |BIT4; //output C port
	DDRD=0xff; //output D port
	
	PCMSK0=0xff;	//Pin Change Interrupt registers
	PCICR = BIT0 ;
	init_spi(); //Initialize SPI
	sei();
    while (1) 
    {
		temperature = get_max6675_temp();
		check_temp();	
		routine1();  //Call Routine 1
    }	
}

