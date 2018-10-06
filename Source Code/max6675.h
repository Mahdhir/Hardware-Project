#define CS_OUTP PORTB  //Chip select output port selection
#define CS_DDR DDRB		//Chip select output port data direction selection
#define CS_PIN PORTB2   //Chip select output pin selection

#define SPI_BUSY_WAIT() while(!(SPSR & (1<<SPIF))) //Wait until transmission complete
#define SEL_MAX6675() ((CS_OUTP) &= ~(1<<CS_PIN))	//Enable Chip select pin
#define DESEL_MAX6675() ((CS_OUTP) |= (1<<CS_PIN))	//Disable Chip select pin
#define spi_get_byte() spi_put_byte(0)				//Data transmission

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

void init_spi(void)
{
	/* port initialisation 
	make sure /SS is output or high level*/
	DDRB |= (1<<PORTB5)|(1<<PORTB2) | (1<<PORTB0);//|(1<<PB5) make MOSI output if necessary

#if CS_PIN != PORTB2
	DESEL_MAX6675();
	CS_DDR  |= CS_PIN;
#endif
	/* set to < 4.3MHz; Mode 0 */
	SPCR = (1<<SPE)|(1<<SPR0)| (1<<MSTR);
}