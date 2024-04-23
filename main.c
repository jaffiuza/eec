//Includes
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include "LCD_driver.h"
#include "LCD_functions.h"
#include "usart.h"

//Defines
#define BUTTON_A    6   // UP
#define BUTTON_B    7   // DOWN
#define BUTTON_C    2   // LEFT
#define BUTTON_D    3   // RIGHT
#define BUTTON_O    4   // PUSH
#define KEY_NULL    0
#define KEY_ENTER   1
#define KEY_NEXT    2
#define KEY_PREV    3
#define KEY_PLUS    4
#define KEY_MINUS   5
#define sbiBF(port,bit)  (port |= (1<<bit))   //set bit in port
#define cbiBF(port,bit)  (port &= ~(1<<bit))  //clear bit in port
#define PINB_MASK ((1<<PINB4)|(1<<PINB6)|(1<<PINB7))
#define PINE_MASK ((1<<PINE2)|(1<<PINE3))
#define FOSC 8000000 // Clock Speed
#define BAUD 9600 // BAUD Rate
#define MYUBRR FOSC/16/BAUD-1

//Variaveis_Globais
char buttons, key, display[] = "000000", receive[]= "            ";
char a;
int pos = 0, showTemp = 0, showHum = 0;
int i = 0;
int writet = 0;
int writeh = 0;



void PinChangeInterrupt()
{
	buttons = (~PINB) & PINB_MASK;
	buttons |= (~PINE) & PINE_MASK;
	EIFR = (1<<PCIF1) | (1<<PCIF0);

	// Output virtual keys
	if (buttons & (1<<BUTTON_A))
	key = KEY_PLUS;
	else if (buttons & (1<<BUTTON_B))
	key = KEY_MINUS;
	else if (buttons & (1<<BUTTON_C))
	key = KEY_PREV;
	else if (buttons & (1<<BUTTON_D))
	key = KEY_NEXT;
	else if (buttons & (1<<BUTTON_O))
	key = KEY_ENTER;
	else
	key = KEY_NULL;
	
	//Ação do botão para cima
	if (key == KEY_PLUS)
	{
		if(pos == 0 || pos == 1)
		{
			showTemp = 1;
			showHum = 0;
			pos = 2;
		}
	}
	
	//Ação do botão para baixo
	if (key == KEY_MINUS)
	{
		if(pos == 0 || pos == 2)
		{
			showHum = 1;
			showTemp = 0;
			pos = 1;
		}
	}
}

void setup()
{
	unsigned int br = MYUBRR;
	USART_Init(br);
		
	//TIMER 2
	ASSR = (1<<AS2);
	ASSR; 
	TCNT2 = 0; 
	TCCR2A |= (1<<CS22) | (0<<CS21) | (1<<CS20);
	TIMSK2 = 1<<TOIE2;
	
	// Init port pins
	cbiBF(DDRB,7);
	cbiBF(DDRB,6);
	cbiBF(DDRB,4);
	PORTB |= PINB_MASK;
	DDRE = 0x00;
	PORTE |= PINE_MASK;

	// Enable pin change interrupt on PORTB and PORTE
	PCMSK0 = PINE_MASK;
	PCMSK1 = PINB_MASK;
	EIFR = (1<<PCIF0)|(1<<PCIF1);
	EIMSK = (1<<PCIE0)|(1<<PCIE1);
	
	LCD_Init();
	sei();
}

ISR(PCINT0_vect)
{
	PinChangeInterrupt();
}

ISR(PCINT1_vect)
{
	PinChangeInterrupt();
}

ISR(TIMER2_OVF_vect)
{
	if(showTemp == 1)
	{
		display[5]= 'C';
		display[4]= receive[5];
		display[3]= receive[4];
		display[2]= receive[3];
		display[1]= receive[2];
		display[0]= receive[1];
	}
	
	else if(showHum == 1)
	{
		display[5]= 'H';
		display[4]= receive[11];
		display[3]= receive[10];
		display[2]= '.';
		display[1]= receive[8];
		display[0]= receive[7];
	}
}

ISR(USART0_RX_vect)
{
	a = Usart_Rx();

	if(a == 't')
	{
		writet = 1;
		writeh = 0;
		i = 0;
	}
	
	else if(a == 'h')
	{
		writeh = 1;
		writet = 0;
	}
	
	if(writet == 1 && i < 6)
	{
		receive[i] = a;
		i++;
	}
	else if (writeh == 1 && i < 12)
	{
		receive[i] = a;
		i++;
	}
}

int main(void)
{
	setup();
	
	while (1)
	{
		LCD_puts(display, 1);
	}
}

