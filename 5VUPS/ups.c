#include "ups.h"

#include <avr/io.h>
// KDevelop workaround
#if !defined (__AVR_ATmega328P__)
	#include <avr/iom328p.h>
#endif

volatile uint16_t main_voltage[30];
volatile uint16_t battery_voltage[30];
volatile uint16_t last_battery_voltage = 0;
volatile uint8_t write_idx = 0;

void ups_init(void)
{
	ups_connect_battery();
}

void ups_refresh(void)
{
	PORTD &= ~(_BV(PD3)|_BV(PD4));
	
	ADMUX &= 0xF0;
	ADCSRA|=(1<<ADSC);
	while(ADCSRA & (1<<ADSC)); //Wait for ADC to complete
	
	main_voltage[write_idx] = (ADCL + (ADCH << 8));;
	
	
	ADMUX |= 0x01;
	ADCSRA|=(1<<ADSC);
	while(ADCSRA & (1<<ADSC)); //Wait for ADC to complete
	
	battery_voltage[write_idx] = (ADCL + (ADCH << 8));;
	
	write_idx++;
	if (write_idx > 29)
	{
		write_idx = 0;
	}
}

uint16_t ups_main_voltage(void)
{
	uint_fast32_t voltage=0;
	for (int i = 0; i < 30; i++)
	{
		voltage += main_voltage[i];
	}
	return voltage * 1600 / 30609;
}

uint16_t ups_battery_voltage(void)
{
	uint_fast32_t voltage=0;
	for (int i = 0; i < 30; i++)
	{
		voltage += battery_voltage[i];
	}
	last_battery_voltage = voltage * 1500 / 30609;
	return last_battery_voltage;
}

uint8_t ups_battery_charge(void)
{
	if (last_battery_voltage > 1240)
		return 100;
	else if (last_battery_voltage > 1235)
		return 90;
	else if (last_battery_voltage > 1225)
		return 80;
	else if (last_battery_voltage > 1215)
		return 70;
	else if (last_battery_voltage > 1195)
		return 60;
	else if (last_battery_voltage > 1180)
		return 50;
	else if (last_battery_voltage > 1165)
		return 40;
	else if (last_battery_voltage > 1140)
		return 30;
	else if (last_battery_voltage > 1120)
		return 20;
	else if (last_battery_voltage > 1080)
		return 10;
	else
		return 0;
}

void ups_connect_battery(void)
{
	PORTD &= ~_BV(PD4);
	PORTD |= _BV(PD3);
}

void ups_disconnect_battery(void)
{
	PORTD &= ~_BV(PD3);
	PORTD |= _BV(PD4);
}