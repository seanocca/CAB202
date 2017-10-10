#include <avr/io.h> 
#include <avr/interrupt.h>
#include <util/delay.h>
#include <cpu_speed.h>
#include <graphics.h>
#include <lcd.h>
#include <macros.h>
#include <sprite.c>
#include <sprite.h>
#include <stdio.h>
#include <stdlib.h>
#include <usb_serial.h>
#include <stdint.h>
#include <string.h>
#include "lcd_model.h"
#include "char.h"
#include "collision.h"
#include "initiliaze.h"
#include "movement.h"
#include "screens.h"

#define FREQ (8000000.0)
#define PRESCALE (1024.0)
#define THRESHOLD (1000)

void serial_outputs(void);

int pausing = 0, gameOver = 0, gameStart = 0,
	received = 0, flr, score, lives;

double time = 0, min = 0;
 
volatile int oflow_counter = 0, gameTime = 0;
 
char buffer[20];

char *keyCollect = "[%d] Player found the Key";
 
ISR(TIMER0_OVF_vect) {
	if (gameStart == 1){
	    oflow_counter ++;
	    gameTime++;
	    if (round(oflow_counter) == 1800){
	    	oflow_counter = 0;
	    	min++;
	    }
	}
}

void run_time(void){
	time = ( oflow_counter * 256.0 + TCNT0 ) * PRESCALE  / FREQ;
}

void serial_USB(void){
	//Allow interruptions
	sei();

	usb_init();

	draw_in_border();
	char *usb = "Waiting for";
	char *usb_two = "USB SERIAL";
	draw_string(18, 15, usb, FG_COLOUR);
	draw_string(20, 25, usb_two, FG_COLOUR);
	show_screen();

	char *not_connect = "CAB202 - Assignment 2 2017 (SEM-2)\r\nWelcome to the USB Debugger!\r\n";
	char *ready = "Please indicate when you are ready to play by pressing either of the two buttons.\r\n";
	char *welcome = "The following is the output of the program:\r\n";
	
	while (!usb_configured() || !usb_serial_get_control()) {
			// Block until USB is ready.	
	}

	usb_serial_write((uint8_t *) not_connect, strlen(not_connect));

	usb_serial_write((uint8_t *) ready, strlen(ready));

	usb_serial_write((uint8_t *) welcome, strlen(welcome));

	clear_screen();
	draw_in_border();
	draw_string(10, LCD_Y / 2 - 2, "USB connected", FG_COLOUR);
	show_screen();
	_delay_ms(500);

}

void setup(void){
	//SET CLOCK SPEED
	set_clock_speed(CPU_8MHz);

	//INITIALIZE SCREEN TO DEFAULT CONTRAST
	lcd_init(LCD_DEFAULT_CONTRAST);

	lcd_clear();

	sei();

	//serial_USB();

	//INITIALIZE JOYSTICK, SWITCHES, POMETERS AND LEDS
	init_teensy();

	lives = 3;

	boolKey = 0; 
	pausing = 0; 

	flr = 0;
	score = 0;

	create_floor(flr);

	serial_outputs();
}

void check_lvl_hits(void){
	if (lives <= 0){
		gameOver = 1;
	}
	if (sprite_collision(player, key) == 1){
		boolKey = 1;
	}

	if (sprite_collision(player, door) == 1 && boolKey == 1){
		flr++;
		score += 100;
		get_load_screen(score, flr);
		create_floor(flr);
		loading = 1;
	}

	if (flr == 0){
		if ((sprite_collision(player,door) == 1 && boolKey == 0) || (sprite_collision(player,tower))) {
			player.y++;
		}
	}
	if (flr == 1){

	}
	if (flr >= 2){

	}
}

void key_follow(void){
	if (boolKey == 1){
		//UP
		if (BIT_IS_SET(PIND, 1)){
			key.y = player.y + player.height;
			key.x = player.x - 1;
		} else 
		//DOWN
		if (BIT_IS_SET(PINB, 7)){
			key.y = player.y - key.height - 1;
			key.x = player.x ;
		} else
		//LEFT
		if (BIT_IS_SET(PINB, 1)){	
			key.x = player.x + key.width;
			key.y = player.y + player.height - key.height;			
		} else
		//RIGHT
		if (BIT_IS_SET(PIND, 0)){
			key.x = player.x - key.width;
			key.y = player.y + player.height - key.height;	
		}
	}

	sprite_draw(&key);

}

void serial_outputs(void){

	char *x_pos = "[%d] Player Position: X = %f, Y = %f";
	sprintf(x_pos,"[%d] Player Position: (%.2f, %.2f)\r\n", gameTime, (double)player.x, (double)player.y);
	usb_serial_write((uint8_t *) x_pos, strlen(x_pos));
}

void process(void){
	clear_screen();

	draw_scroll_border();

	move_player();

	key_follow();

	draw_level(flr);

	enemy_crawl();

	check_lvl_hits();

	show_screen();
}

void restart_game(void);

int main(void){	
	setup();

	//serial_outputs();

	while(gameStart == 0){
		start_screen();
	}

	while (gameOver == 0 && gameStart == 1) {
		run_time();
		if (loading == 0){
			if (pause_status(flr, score, lives, time, min) == 0){
				process();
				_delay_ms(10);
			} else {
				pause_status(flr, score, lives, time, min);
			}
		} else {
			show_load_screen();
		}
	}	

	while(gameOver == 1){
		restart_game();
		game_over(flr, lives, score);
	}
	return 0;
}

void restart_game(void){	
	if (BIT_IS_SET(PINF,5) || BIT_IS_SET(PINF, 6)){	
		gameOver = 0;
		gameStart = 0;
		lives = 3;
		time = 0; min = 0;
		while(BIT_IS_SET(PINF,5) || BIT_IS_SET(PINF, 6)){};
		main();
		clear_screen();

	}

}