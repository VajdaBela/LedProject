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
#include <string.h>


#define A 0
#define B 1
#define C 2
#define D 3

#define RCK 4
#define SERIN 5
#define SRCK 5


volatile uint8_t shiftNum = 0;
volatile uint8_t const allRows[8] = {0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0xF0}; // last one for clearing previous
#define ERASE 7
volatile uint8_t const numbers[][7] = { 
	{0b00001111, 0b00001001, 0b00001001, 0b00001001, 0b00001001, 0b00001001, 0b00001111 },//0
	{0b00000001, 0b00000001, 0b00000001, 0b00000001, 0b00000001, 0b00000001, 0b00000001 },//1
	{0b00001111, 0b00000001, 0b00000001, 0b00001111, 0b00001000, 0b00001000, 0b00001111 },//2
};
volatile uint16_t bufer[7] = {0};

char text[] = "012";


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


void SetBufer()
{
	//clear bufer
	for(uint8_t br1 =  0; br1 < 7; ++br1)
	{
		bufer[br1] = 0;
	}
	
	uint16_t temp[7] = {0};
	for(uint8_t br = 0; br < strlen(text); ++br)
	{
		if ((shiftNum - br * 5 - 5) < 16)
		{
			if ((shiftNum - br * 5) < 0)
			{
				break;
			}
		
			//set temp
			for(uint8_t br1 = 0; br1 < 7; ++br1)
			{
				temp[br1] = numbers[text[br] - 48][br1];
			}
			
			//set bufer
			for (uint8_t br1 = 0; br1 < 7; ++br1)
			{
				if((shiftNum - 4 - br * 5) > 0)
				{
					bufer[br1] |= (temp[br1] << (shiftNum - 4 - br * 5));
				}
				else
					bufer[br1] |= (temp[br1] >> (shiftNum - 4 - br * 5) * (-1));
				
			}
		}
	}
}



void TurnOnRow(uint8_t row)
{
	PORTB &= allRows[ERASE];
	PORTB |= allRows[row];
}


void PutData(uint16_t data)
{
	//acording to Vajda
	for(uint8_t br = 0; br < 16; ++br)
	{
		if(data & (1 << br))
		Shift(1);
		else
		Shift(0);
	}
	PORTB |= (1 << RCK);
	PORTB &= ~(1 << RCK);
	
	
	//according to tutorial
	/*PORTB &= ~(1 << RCK) & ~(1 << SERIN);
	PORTC &= ~(1 << SRCK);
	
	for(uint8_t br = 0; br < 16; ++br)
	{
		PORTC &= ~(1 << SRCK);
		
		if(data & (1 << br))
			PORTB |= (1 << SERIN);
		else
			PORTB &= ~(1 << SERIN);
			
		PORTC |= (1 << SRCK);
		PORTB &= ~(1 << SERIN);
	} 
	
	PORTB |= (1 << RCK);
	PORTB &= ~(1 << SERIN);
	PORTC &= ~(1 << SRCK);*/
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
				SetBufer();
				PutData(bufer[br]);
				TurnOnRow(br);
			}
	}
}