#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "uart.h"
#include "ups.h"

// KDevelop workaround
#if !defined (__AVR_ATmega328P__)
	#include <avr/iom328p.h>
#endif

char cmd_buf[64];
volatile uint8_t cnt = 0;
uint8_t shutdown = 0;
uint8_t hold = 0;

void adc_init(void);
void port_init(void);
void timer_init(void);
void print_status(void);

int8_t read_cmd(char buf[], uint8_t len)
{
	int i = 0;
	char ch = getchar();
	while(ch != '\r' && ch != '\n')
	{
		/*putchar(ch);*/
		buf[i++] = ch;
		if (i > len)
			return -1;
		ch = getchar();
	}
	buf[i] = '\0';
	return i;
}



void main(void) __attribute__((noreturn));
void main(void)
{
	/* setup the various functions we need */
	adc_init();
	port_init();
	timer_init();
	uart_init();
	ups_init();
	
	/* enable interrupts */
	sei();
	
	/* and start doing the boring stuff */
	while (true)
	{
		uint8_t len = 0;
		if ((len = read_cmd(cmd_buf, 63)) > 0)
		{
			if (len > 5 && strncmp(cmd_buf, "status", 6) == 0)
			{
				print_status();
			}
			else if (len > 7 && strncmp(cmd_buf, "shutdown", 8) == 0)
			{
				shutdown = atoi(cmd_buf+7);
				if (shutdown == 0)
					shutdown = 1;
				printf("ok\r\n");
			}
			else if (len > 5 && strncmp(cmd_buf, "reboot", 6) == 0)
			{
				shutdown = atoi(cmd_buf+7);
				char* n = strchr(cmd_buf+7, ' ');
				hold = (n == NULL) ? 0 : atoi(n);
				if (shutdown == 0) shutdown = 1;
				if (hold == 0) hold = 1;
				printf("ok\r\n");
			}
			else
			{
				printf("invalid command\r\n");
			}
		}
		else
		{
			printf("read error\r\n");
		}
	}
}

// Timer1 interrupt ~30 hz
ISR(TIMER1_COMPA_vect)
{
	ups_refresh();
	cnt++;
	if (cnt > 29)
	{
		/* 1 second blinker so we know that stuff is firing */
		cnt = 0;
		PORTB ^= _BV(PB5);
		/* shutdown countdown, triggers on the 1 -> 0 transition */
		if (shutdown > 0)
		{
			shutdown--;
			if (shutdown == 0)
				ups_disconnect_battery();
		}
		/* once shutdown, hold is > 1 if we are to boot up again otherwise stay like this until powercycle */
		else if (shutdown == 0 && hold > 0)
		{
			hold--;
			if (hold == 0)
				ups_connect_battery();
		}
	}
}

void print_status(void)
{
	/* status output 
	 * ups.status
			OL      - On line (mains is present)
			OB      - On battery (mains is not present)
			LB      - Low battery
			RB      - The battery needs to be replaced
			CHRG    - The battery is charging
			BYPASS  - UPS bypass circuit is active - no battery protection is available
		*/
	uint16_t mains = ups_main_voltage();
	uint16_t battery = ups_battery_voltage();
	printf("ups.status=");
	if (mains > battery)
	{
		printf("OL");
		/* Value 170 is measured from the specific diodes used */
		if ((battery+170) < mains)
		{
			printf(" CHRG");
		}
	}
	else
	{
		printf("OB");
		if (ups_battery_charge() <= 10)
		{
			printf(" LB");
		}
	}
	printf("\r\n");
	
	printf("input.voltage=%d\r\n", mains);
	printf("battery.voltage=%d\r\n", battery);
	printf("battery.charge=%d\r\n", ups_battery_charge());
}

void adc_init(void)
{
	ADCSRA = _BV(ADEN) | _BV(ADPS1) | _BV(ADPS2) | _BV(ADPS0);
	ADMUX = _BV(REFS0);
	DIDR0 = 0x3f;
}

void port_init(void)
{
	DDRD = _BV(PD4) | _BV(PD3);
	DDRB = _BV(PB5);
}

void timer_init(void)
{
	TCCR1B |= _BV(WGM12) | _BV(CS11) | _BV(CS10);
	TIMSK1 |= _BV(OCIE1A);
	OCR1AH = (4165>>8);
	OCR1AL = (4165&0xff);
}
