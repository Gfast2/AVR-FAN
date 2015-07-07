/*
 * attiny25_74HC595.c
 * Inspired by : https://www.youtube.com/watch?v=d7Au3I4ZdZc
 *
 * Created		: 2015-7-7
 * last Edite	: 2015-7-7
 *  Author		: Su Gao
 */ 

// I deactivate divider fuse (CKDIV8) in order to speed up PWM.
// According Datasheet Table 12-3 on page83, my code should run at 500KHz driven by Timer/Counter1
// But it not. When I set F_CPU = 8000000UL, PWM works @50KHz, 
// After I set it to final 1000000UL,		 PWM works @65KHz.
// On theory, When I deactivate CKDIV8 fuse, and set AVR uses inner 8MHz clock. It should works at 8M
// Till End I didn`t get the idea, why it run so slow. But It still fast enough to drive the FAN noiseless.
#define F_CPU 1000000UL 

#include <avr/io.h>
#include <avr/interrupt.h>
#include "attiny25_74HC595.h"
#include <util/delay.h>

volatile uint16_t voltage_reading;
volatile uint8_t  dutyCycle = 127; 

void initialise_ADC(void)
{
	ADCSRA |= 1<<ADEN | 1<<ADIE | 1<<ADPS0 | 1<<ADPS2;
	ADMUX  |= 1<<MUX1;	// PB3 as ADC pin
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
	
	// This block are using Timer/Counter0 to drive FAN and the LED @32.5KHz
	// Uncomment this block if you wanna drive PWM with timer 0
	//TCCR0A |= (1<<COM0A1) | (1<<COM0B1) | (1<<WGM00) | (1<<WGM01);	// fast PWM mode + clear OC0A & CO0B on compare match
	//TIMSK |= (1<<TOIE0);								// Timer/Counter0 Overflow Interrupt Enable
	//OCR0A = 125;										// should later be comment out to let interrupt handle this problem.
	//OCR0B = 125;
	//TCCR0B |= (1<<CS00);								// no prescaler
	
	OCR1C = 127;											// setting the PWM speed. In this case, it is working @500KHz
	TCCR1 |= (1 << PWM1A) | (1 << COM1A0) | (1 << CS10);	// PWM mode + clear OC1A output line + CK/2. COM1A0 decide if PB0 get inverted signal!! Page:87
	TIMSK |= (1 << TOIE1);									//P.92 Timer/Counter Overflow Interrupt Enable
	
	while(1){}
}

ISR(ADC_vect)
{
	voltage_reading = ADC;
	dutyCycle = voltage_reading / 8; // voltage_reading: 0 ~ 1023, dutyCycle: 0 ~ 127
	ADCSRA |= 1<<ADSC;
}

/*
// This block are using Timer/Counter0 to drive FAN and the LED @32.5KHz
// Uncomment this block if you wanna drive PWM with timer 0
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

