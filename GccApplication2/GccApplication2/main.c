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

//number of movements
volatile uint8_t shiftNum = 0;
//this will turn on A B C and D
volatile uint8_t const allRows[8] = {0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0xF0}; // last one for clearing previous
//no line turned on from the previous
#define ERASE 7
//character we are using
volatile uint8_t const numbers[][7] = { 
	{0b00001111, 0b00001001, 0b00001001, 0b00001001, 0b00001001, 0b00001001, 0b00001111 },//0
	{0b00000001, 0b00000011, 0b00000101, 0b00001001, 0b00000001, 0b00000001, 0b00000001 },//1
	{0b00001111, 0b00000001, 0b00000001, 0b00001111, 0b00001000, 0b00001000, 0b00001111 },//2
};
//what should be put in the register
volatile uint16_t bufer[7] = {0};

//what will be written out
char text[] = "012";

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
void clear()
{
	for(uint8_t br = 0; br < 16; ++br)
	{
		Shift(0);
	}
}

//buffer neads to be made (it'll consist only from what is on screen)
void SetBufer()
{
	//clear bufer
	for(uint8_t br1 =  0; br1 < 7; ++br1)
	{
		bufer[br1] = 0;
	}
	
	uint16_t temp[7] = {0};
	//go through all of the letters
	for(uint8_t br = 0; br < strlen(text); ++br)
	{
		//if shiftNum - br * 5 - 5 is bigger than 16 the letters left the screen
		if ((shiftNum - br * 5 - 5) < 16)
		{
			//if shiftNum - br * 5 is smaller than 0 the letters didnt yet come to screen(we can discard them, brake is for that)
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
				//because  shifting with a negative is not defined in c there are two cases
				if((shiftNum - 4 - br * 5) > 0)//for positiv
				{
					bufer[br1] |= (temp[br1] << (shiftNum - 4 - br * 5));
				}
				else // and negative
					bufer[br1] |= (temp[br1] >> (shiftNum - 4 - br * 5) * (-1));
				/*the letters in a bufer will be mooved either left or right(right if it's beig cut off) this mutch: shiftNum - 4 - br * 5 
				/shift num is logical, 4 is so that we dont start with the first letter already on screen, br * 5 is the letter in the row times 5
				/ 5 is used because a letter consists of 4 diods + 'space'
				 */
			}
		}
	}
}


//firs turns off all existing rows then turns on one
void TurnOnRow(uint8_t row)
{
	PORTB &= allRows[ERASE];
	PORTB |= allRows[row];
}

//sets up timer with a prescaler of 1024 (currently counts to 500 ms)
void setupTimer()
{
	TIMSK1 |= (1 << OCIE1A);
	TCCR1B |= (1 << WGM12) | (1 << CS12) | (1 << CS10);
	OCR1A = 5400;
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
	setupTimer();
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

//interupts evry 500 ms and moovs the letters by 1 (goes until (5 * strlen(text) ) + 16 ) and then starts all over )
ISR(TIMER1_COMPA_vect)
{
	shiftNum++;
	if (shiftNum > ((5 * strlen(text) ) + 16 ) )
	{
		shiftNum = 0;
	}
}
//extra new line so that compiler is happy :^)
