/*
 * GccApplication2.c
 *
 * Created: 7/13/2016 11:10:18 PM
 * Author : Kripton
 */ 

#define F_CPU 11059200

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>


#define A 0
#define B 1
#define C 2
#define D 3

#define RCK 4
#define SERIN 5
#define SRCK 5


uint8_t const allRows[8] = {0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0xF0}; // last one for clearing previous
uint8_t const numbers[][7] = { 
	{0b00001111, 0b00001001, 0b00001001, 0b00001001, 0b00001001, 0b00001001, 0b00001111 },//0
	{0b00000001, 0b00000001, 0b00000001, 0b00000001, 0b00000001, 0b00000001, 0b00000001 },//1
	{0b00001111, 0b00000001, 0b00000001, 0b00001111, 0b00001000, 0b00001000, 0b00001111 },//2
};


#define ERASE 7


void Shift(uint8_t state)
{
	if(state)
		PORTB |= (1 << SERIN);
	else
		PORTB &= ~(1 << SERIN);
		
	PORTC |= (1 << SRCK);
	PORTC &= ~(1 << SRCK);
}


void clear()
{
	for(uint8_t br = 0; br < 16; ++br)
	{
		Shift(0);
	}
}


void TurnOnRow(uint8_t row)
{
	PORTB &= allRows[ERASE];
	PORTB |= allRows[row];
}


void PutData(uint8_t data)
{
	for(uint8_t br = 0; br < 16; ++br)
	{
		if(data & (1 << br))
		Shift(1);
		else
		Shift(0);
	}
	PORTB |= (1 << RCK);
	PORTB &= ~(1 << RCK);	
}


int main()
{
	
	//setup
	DDRB = 0xFF;
	DDRC |= (1 << 5);
	clear();
	
	while(1)
	{
			for(uint8_t br = 0; br < 7; ++br)
			{
				PutData(numbers[2][br]);
				TurnOnRow(br);
			}
	}
}