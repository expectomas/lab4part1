/*
 * lab4part1.c
 *
 * Created: 02/03/2012 3:50:15 PM
 *  Author: Ethan
 */ 

#include<avr/io.h>
#define  F_CPU 16000000
#include<avr/interrupt.h>

//Attributes
int adc_val = 0, loval=0, hival=0;

//ADC Section

ISR(ADC_vect)
{
	loval = ADCL;
	hival = ADCH;
	
	//adc_val is 10 bits number 
	adc_val = hival*256 + loval;
}

void setupADC()
{
	 // Set up ADC using interrupt programming.
	 
	 //Set up Power Reduction Register
	 PRR |= 0b11111110;
	 
	 //Enable ADC and Prescaler (64) => Freq = 256kHz
	 ADCSRA = 0b10000110;
	 
	 //Enable ADCSRA interrupt flag
	 ADCSRA |= 0b00001000;
	 
	 //Enable global interrupt
	 sei();
}
	 
void startADC(int chan)
{
	//Select Channel to Read and Ref Voltage
	ADMUX	= 0b01000000 + chan;	
	
	// Starts ADC conversion for channel
	ADCSRA |= 0b01000000;
}

//PWM Section

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