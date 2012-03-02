/*
 * lab4part1.c
 *
 * Created: 02/03/2012 3:50:15 PM
 *  Author: Ethan
 */ 

#include<avr/io.h>
#include<avr/interrupt.h>

int pwm_val[]={0, 26, 51, 77, 102, 128, 153, 179, 204, 230, 255};
int index=0;

void setupPWM()
{
	TCNT2=0;
	OCR2A=0;
	TCCR2A=0b10000001;
	TCCR2B=0b00000000;
	TIMSK2|=0b10;
}

ISR(TIMER2_COMPA_vect)
{
	index=(index+1)%11;
	OCR2A=pwm_val[index];
}

void startPWM()
{
	TCCR2B=0b00000100;
	sei();
}

int main(void)
{
    while(1)
    {
        //TODO:: Please write your application code 
    }
}