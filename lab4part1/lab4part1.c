/*
 * lab4part1.c
 *
 * Created: 28/2/2012 1:04:46 PM
 *  Author: Desmond Wee
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#define F_CPU 16000000
#include<util/delay.h>
#include <math.h>

unsigned adc_val, channel1Ready, channel0Ready, nowScanning, pwm_val;;

//ADC Section
ISR(ADC_vect)
{
	unsigned loval, hival;
	loval = ADCL;
	hival = ADCH;
	
	//adc_val is 10 bits number 	
	adc_val = hival*256 + loval;
}

void setupADC()
{	 	
	//Set up Power Reduction Register
	PRR &= 0b11111110;
	
	//Enable ADC and Prescaler (64) => Freq = 256kHz 
	ADCSRA = 0b10000110;
		
	//Enable ADCSRA interrupt flag
	ADCSRA |= 0b00001000;
}

void startADC(int chan)
{
	//Select Channel to Read and Ref Voltage
	if(chan==0)
	{
		ADMUX = 0b01000000;
	}	
	else
	{
		ADMUX = 0b01000001;
	}
	
	// Starts ADC conversion for channel	
	ADCSRA |= 0b01000000;
}

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
	OCR2A = pwm_val;
}

void startPWM()
{
	TCCR2B=0b00000100; //Set Prescaler (256)
}

//Buzzer Output
int remap(int val, int min_val, int max_val)
{
	double num = val-min_val;
	double dom = max_val-min_val;
	
	int result = (num/dom)*255;
	
	if(result<0) //If ADC value is below min value
		return 0;	
	if(result>255) //If ADC value is above max value
		return 255;
		
	return result;
}

void tone(int input)
{
	//i rep the number of wave within 1 sec	
	int freq;
	double period, division;
	
	division = input/255.0;

	freq = division*400+200; //200 is the least freq (fully dark), 500 is the highest freq (fully bright)	
	period = 1.0/freq; //eg. 0.01s

	double delay = period/2*pow(10,3);

	for(int i=0; i<freq; i++)
	{
		// Write a 1 to digital pin 13 (PINB 5)		
		PINB|= 0b00100000;
		_delay_ms(delay); //Delay is half the period (in millisecond)
	
		// Write a 0 to digital pin 13 (PINB 5)
		PINB &= 0b11011111;
		_delay_ms(delay); //Delay is half the period (in millisecond)
	}
}

int main(void)
{
	unsigned constrained = 0, shutOffBuzzer = 0;
	
	//Set GPIO Pins
	if(!shutOffBuzzer)
		DDRB|=0b00100000; //PIN 13 output to buzzer
	
	DDRB|=0b00001000;

	sei();
		
	setupADC();
	setupPWM();
	startPWM();
	
    while(1)
    {
		//Poll ADC channel 0
    	startADC(0);
		pwm_val= remap(adc_val,100,900);
		
		startADC(1);
		//0-255 steps for tone strength
		constrained = remap(adc_val,619,890);			
		//Output tone
		tone(constrained);
    }
}