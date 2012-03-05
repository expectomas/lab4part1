/*
 * lab4part1.c
 *
 * Created: 02/03/2012 3:50:15 PM
 *  Author: Ethan
 */ 

#include<avr/io.h>
#define  F_CPU 16000000
#include<avr/interrupt.h>
#include<util/delay.h>
#include <math.h>

//Attributes
unsigned adc_val = 0, channel1Ready=0, channel0Ready=0, nowScanning=0;

//ADC Section

ISR(ADC_vect)
{
	unsigned loval = 0, hival=0;
	
	loval = ADCL;
	hival = ADCH;
	
	//adc_val is 10 bits number 
	adc_val = hival*256 + loval;
	
	if(nowScanning)
	{
		channel1Ready=1;
	}		
	else
	{
		channel0Ready=1;
	}	
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
	nowScanning = chan;
	
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

//Output

int remap(int val, int min_val, int max_val)
{
	double num = val-min_val;
	double dom = max_val-min_val;
	
	int result = (num/dom)*255;

	if(result<0)		//If ADC value is below min value
		return 0;
	
	if(result>255)		//If ADC value is above max value
		return 255;
	
	return result;
}

void tone(int input)
{
	//i rep the number of wave within 1 sec
	int freq; 
	double period, division;
	
	division = input/255.0;
	
	freq = division*400+200;   //200 is the least freq (fully dark), 500 is the highest freq (fully bright)
	period = 1.0/freq;			  //eg. 0.01s
	
	double delay = period/2*pow(10,3); 
	
	for(int i=0; i<freq; i++)
	{
		// Write a 1 to digital pin 13 (PINB 5)
		PINB|= 0b00100000;
		_delay_ms(delay);
		//Delay is half the period (in millisecond)
	
		// Write a 0 to digital pin 13 (PINB 5)
		PINB &= 0b11011111;
		_delay_ms(delay);
		//Delay is half the period (in millisecond)
	}
}

int main(void)
{
	unsigned constrained = 0, shutOffBuzzer = 1;
	
	//Set GPIO Pins
	if(!shutOffBuzzer)
		DDRB|=0b00100000;  //PIN 13 output to buzzer
	
	//Set up PWM and ADC
	setupADC();
	setupPWM();
	
	//Poll ADC channel 0
			
		startADC(0);
		
    while(1)
    {
		/* Buzzer Output */
		if(channel0Ready)
		{
			//0-255 steps for tone strength 
			constrained = remap(adc_val,619,890);
		
			//Output tone )
			tone(constrained);
			startADC(1);
			channel0Ready = 0;
		}
					
		if(channel1Ready)
		{
			startPWM();
			startADC(0);
			channel1Ready = 0;
		}			
    }
}