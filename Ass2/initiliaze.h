#include <avr/io.h> 
#include <avr/interrupt.h>
#include "lcd_model.h"
#include <macros.h>

void init_teensy(void){
	//INITILIAZE BUTTONS AND JOYSTICKS
	CLEAR_BIT(DDRD, 1); //Up
	CLEAR_BIT(DDRB, 7); //Down
	CLEAR_BIT(DDRB, 1); //Right
	CLEAR_BIT(DDRD, 0); //Left
	CLEAR_BIT(DDRB, 0); //Centre

	CLEAR_BIT(DDRF, 5); //Left Swicth
	CLEAR_BIT(DDRF, 6); //RIght Switch

	CLEAR_BIT(DDRF, 0); //Left Pometer
	CLEAR_BIT(DDRF, 1); //Right Pometer

	SET_BIT(DDRB, 2); //Left LED
	SET_BIT(DDRB, 3); //Right LED	
	SET_BIT(DDRD, 6); //Teensy LED

	//SETUP TIMER
	TCCR0A = 0;
	TCCR0B = 5;
	TIMSK0 = 1;
}