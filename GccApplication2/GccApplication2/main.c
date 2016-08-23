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

//logical
#define A 0
#define B 1
#define C 2
#define D 3

#define RCK 4
#define SERIN 5
#define SRCK 5

#define PLATE_NUM 1

//number of movements
volatile uint8_t shiftNum = 0;
//this will turn on A B C and D
volatile uint8_t const allRows[8] = {0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0xF0}; // last one for clearing previous
//no line turned on from the previous
#define ERASE 7
//character we are using
volatile uint8_t const numbers[][7] = { 
	{0b00001110, 0b00010001, 0b00010011, 0b00010101, 0b00011001, 0b00010001, 0b00001110 },//0
};
//what should be put in the register
volatile uint16_t bufer[PLATE_NUM][7];

//what will be written out
char text[] = "0";

//puts either 1 or 0 into the register 
void Shift(uint8_t state)
{
	if(state)
		PORTB |= (1 << SERIN);
	else
		PORTB &= ~(1 << SERIN);
	
	//the actual shift	
	PORTC |= (1 << SRCK);
	PORTC &= ~(1 << SRCK);
}

//clears the registers (shift 0, 16 times
void Clear()
{
	for(uint8_t br = 0; br < (16 * PLATE_NUM); ++br)
	{
		Shift(0);
	}
}

//buffer neads to be made (it'll consist only from what is on screen)
void SetBufer()
{
	//clear bufer
	for(uint8_t br = 0; br < PLATE_NUM; ++br)
	{
		for(uint8_t br1 =  0; br1 < 7; ++br1)
		{
			bufer[br][br1] = 0;
		}
	}
	
	uint16_t temp[7] = {0};
		
	
	
	for(uint8_t br = 0; br < PLATE_NUM; ++br )
	{
		int8_t lastSign = (shiftNum - (16 * br)) / 6;
		int8_t firstSign = lastSign - 3;
		if(firstSign < 0 )
		firstSign = 0;
		if(lastSign > strlen(text) - 1 )
		lastSign = strlen(text) - 1;
		
		for(uint8_t br1 = firstSign; br1 <= lastSign; ++br1 )
		{
			//set temp
			for(uint8_t br2 = 0; br2 < 7; ++br2 )
			{
				temp[br2] = numbers[text[br1] - 48][br2];
			}

			//set bufer
			for(uint8_t br2 = 0; br2 < 7; ++br2 )
			{
				if((shiftNum - 5 - (br * 16) - (br1 * 6) ) > 0 )
				bufer[br][br2] |= temp[br2] << (shiftNum - 5 - (br * 16) - (br1 * 6) );
				else
				bufer[br][br2] |= temp[br2] >> ((shiftNum - 5 - (br * 16) - (br1 * 6)) *(-1) );
			}
		}
	}
}


//firs turns off all existing rows then turns on one
void TurnOnRow(uint8_t row)
{
	PORTB &= allRows[ERASE];
	PORTB |= (1 << RCK);
	PORTB &= ~(1 << RCK);
	PORTB |= allRows[row];
}

//sets up timer with a prescaler of 1024 (currently counts to 250 / 2 ms)
void SetupTimer()
{
	TIMSK1 |= (1 << OCIE1A);
	TCCR1B |= (1 << WGM12) | (1 << CS12) | (1 << CS10);
	OCR1A = 1350;
	sei();
}

//reads data and puts into the register
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
}


int main()
{
	
	//setup
	DDRB = 0xFF;
	DDRC |= (1 << SRCK);
	SetupTimer();
	Clear();
	
	while(1)
	{
		SetBufer();
			for(uint8_t br = 0; br < 7; ++br)
			{
				for(uint8_t br1 = 0; br1 < PLATE_NUM; ++br1)
				{
					PutData(bufer[br1][br]);
				}
				TurnOnRow(br);
			}
	}
}

//interupts evry 500 ms and moovs the letters by 1 (goes until (5 * strlen(text) ) + 16 ) and then starts all over )
ISR(TIMER1_COMPA_vect)
{
	shiftNum++;
	if (shiftNum > ((5 * strlen(text) ) + (16 * PLATE_NUM) ) )
	{
		shiftNum = 0;
	}
}
//extra new line so that compiler is happy :^)