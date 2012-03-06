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

/*0% to 100% duty cycle value*/
			  //0                                           10//
int pwm_val[]={0, 26, 51, 77, 102, 128, 153, 179, 204, 230, 255};
int index=0;

void setupPWM()
{
	//Set initial count register to zero
	TCNT2=0;
	
	//Initialised Duty Cycle to zero
	OCR2A=0;
	
	//Fixed by lab sheet
	
	TCCR2A	=0b10000001;
	TCCR2B	=0b11000000;
	
	//Enabled Interrupt 
	TIMSK2|=0b10;
}

void setupPWM0()
{
	// Set initial timer value
TCNT0=0;
// Set the initial OCR0A values
OCR0A=0;
// and choose mode 1 Phase correct PWM
TCCR0A=0b10000001;
// Enable compare interrupt
TIMSK0 |= 0b10;
}

int pwm_values[]={0, 26, 51, 77, 102, 128, 153, 179, 204, 230, 255};
int index0=0;

void startPWM0()
{
// Set prescaler of 0b011, or 64.
TCCR0B=0b00000011;
// Set global interrupts
sei();
}

ISR(TIMER0_COMPA_vect)
{
// Increment the index to go to next duty cycle value.
index=(index+1)%11;
OCR0A=pwm_values[index0];
}

ISR(TIMER2_COMPA_vect)
{
	//Progressive Light Up
	//index=(index+1)%11;
	//OCR2A=pwm_val[index]; 	
}

void startPWM()
{
	//Set Prescaler (256) & Start PWM generation
	TCCR2B=0b00000100;
	
	sei();
}

//Buzzer Output

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
	
	//Testing
	setupPWM0();
	startPWM0();
	
	while(1) {}
	
	//Set GPIO Pins
	if(!shutOffBuzzer)
		DDRB|=0b00100000;  //PIN 13 output to buzzer
		
	DDRB|=0b00001000;
	
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