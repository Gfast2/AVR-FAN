/*
 * AVR_Shift_Register.c
 * Inspired by : https://www.youtube.com/watch?v=d7Au3I4ZdZc
 *
 * Created		: 2015-7-7
 * last Edite	: 2015-7-7
 *  Author		: Su Gao
 */ 

// In first try it shows about 4 sec. per digit under 8000000UL. And I think the 8 divider (CKDI8) in fuse works.
// This time I deactivate this divider fuse in order to speed up PWM (original with divided prescaler PWM working at 500K)
#define F_CPU 8000000UL 

#include <avr/io.h>
#include <avr/interrupt.h>
#include "attiny25_74HC595.h"
#include <util/delay.h>

volatile uint16_t voltage_reading;
volatile uint8_t  firstdigit;
volatile uint8_t  pwmFAN;
volatile uint8_t  dutyCycle = 127; //percent value of PWM dutyCycle.

void initialise_ADC(void)
{
	ADCSRA |= 1<<ADEN | 1<<ADIE | 1<<ADPS0 | 1<<ADPS2;
	ADMUX  |= 1<<MUX1;						// PB3 as ADC pin
}

void start_conversion(void)
{
	ADCSRA |= 1<<ADSC;
}


int main(void)
{
	sei();

	initialise_ADC();
	start_conversion();
	
	DDRB |= (1 << PB0) | (1 << PB1);						// PWM control pin, as output
	//TCCR0A |= (1<<COM0A1) | (1<<COM0B1) | (1<<WGM00) | (1<<WGM01);	// fast PWM mode + clear OC0A & CO0B on compare match
	
	//TIMSK |= (1<<TOIE0);								// Timer/Counter0 Overflow Interrupt Enable
	
	//OCR0A = 125;										// should later be comment out to let interrupt handle this problem.
	//OCR0B = 125;
	
	//TCCR0B |= (1<<CS00);								// no prescaler
	
	OCR1C = 159;
	TCCR1 |= (1 << PWM1B) | (1 << PWM1A) | (1 << COM1A1) | (1 << CS11); // PWM mode + clear OC1A output line + CK/2
	
	TIMSK |= (1 << TOIE1); //P.92 Timer/Counter Overflow Interrupt Enable
	
	while(1)
	{
	}
}

ISR(ADC_vect)
{
	voltage_reading = ADC;
	
	dutyCycle = voltage_reading / 4; // voltage_reading: 0 ~ 1023, dutyCycle: 0 ~ 255
	ADCSRA |= 1<<ADSC;
}

/* //works parts with timer/counter 0
ISR(TIMER0_OVF_vect)
{
	OCR0A = dutyCycle;
	OCR0B = dutyCycle;
}
*/


ISR(TIMER1_OVF_vect)
{
	OCR1A = dutyCycle;
}

