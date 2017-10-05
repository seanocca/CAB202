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

#define FREQ (8000000.0)
#define PRESCALE (1024.0)
#define THRESHOLD (1000)

void serial_outputs(void);

Sprite player;
Sprite tower;
Sprite key;
Sprite enemy;
Sprite door;
Sprite left;
Sprite right;
Sprite top;
Sprite bottom;

int flr = 0, score = 0, lives = 3, boolKey = 0, 
	pausing = 0, gameOver = 0, gameStart = 0, cDown = 11, cDownOut = 3,
	received = 0, loading = 0;

double time = 0, min = 0;
 
volatile int oflow_counter = 0, gameTime = 0;
 
char buffer[20];

char *flr_msg = "Floor: %d";
char *tim_msg = "Time: %02d:%02d";
char *score_msg = "Score: %d";
char *load_score_msg = "Score: %d";
char *load_flr_msg = "Floor: %d";
char *liv_msg = "Lives: %d";

char *name = "Sean O'Connell";
char *studentNum = "n10000569";
char *readyCount = "Ready?";

char *keyCollect = "[%d] Player found the Key";

void draw_in_border(void);

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

void draw_in_border(void){
	for (int h = 0; h < 48; h++){
		set_pixel(0,h,FG_COLOUR);
		set_pixel(1,h,FG_COLOUR);
		set_pixel(83,h, FG_COLOUR);
		set_pixel(82,h,FG_COLOUR);
	}
	for (int w = 0; w < 84; w++){
		set_pixel(w,0,FG_COLOUR);
		set_pixel(w,1,FG_COLOUR);
		set_pixel(w,47, FG_COLOUR);
		set_pixel(w,46,FG_COLOUR);
	}
}

void create_scroll_border(void){
	sprite_init(&left,-21,-12,VERWIDTH,VERHEIGHT,verBitmaps);
	sprite_init(&right,105 - VERWIDTH,-12,VERWIDTH,VERHEIGHT,verBitmaps);
	sprite_init(&top,-21,-12,HORWIDTH, HORHEIGHT,horBitmaps);
	sprite_init(&bottom,-21,60,HORWIDTH,HORHEIGHT,horBitmaps);

}

void draw_scroll_border(void){
	sprite_draw(&left);
	sprite_draw(&right);
	sprite_draw(&top);
	sprite_draw(&bottom);
}

void start_screen(void){
	int countPos = 28;

	clear_screen();

	if(BIT_IS_SET(PINF,5) || BIT_IS_SET(PINF, 6)){
		while (cDown > 0){
			cDown--;

			clear_screen();

			if (cDown % 3 == 0){			
				countPos = LCD_X/2;
				sprintf(readyCount, "%d", cDownOut);
				cDownOut--;
				draw_string(countPos,LCD_Y / 4 * 3 + 2, readyCount, FG_COLOUR);
			}

			_delay_ms(100);

			draw_in_border();

			draw_string(17,2, "ANSI-TOWER", FG_COLOUR);
			draw_string(8,LCD_Y / 4 + 2, name, FG_COLOUR);
			draw_string(19,LCD_Y / 4 * 2 + 2, studentNum, FG_COLOUR);
			draw_string(countPos,LCD_Y / 4 * 3 + 2, readyCount, FG_COLOUR);

			show_screen();


		}
		gameStart = 1;
	}

	draw_in_border();

	draw_string(17,2, "ANSI-TOWER", FG_COLOUR);
	draw_string(8,LCD_Y / 4 + 2, name, FG_COLOUR);
	draw_string(19,LCD_Y / 4 * 2 + 2, studentNum, FG_COLOUR);
	draw_string(countPos,LCD_Y / 4 * 3 + 2, readyCount, FG_COLOUR);

	show_screen();

}


 
void draw_double(uint8_t x, uint8_t y, double seconds, double minutes, colour_t colour) {
    snprintf(buffer, sizeof(buffer), "Time: %02.0f:%02.0f", minutes, seconds);
    draw_string(x, y, buffer, colour);
}
 
ISR(TIMER0_OVF_vect) {
    oflow_counter ++;
    gameTime++;
    if (round(oflow_counter) == 1800){
    	oflow_counter = 0;
    	min++;
    }
}

void run_time(void){
	time = ( oflow_counter * 256.0 + TCNT0 ) * PRESCALE  / FREQ;
}

void create_floor(int level){

	create_scroll_border();

	if (level == 0){
		sprite_init(&player, round((LCD_X - PLAYERWIDTH) / 2), round((LCD_Y - PLAYERHEIGHT) / 2), PLAYERWIDTH, PLAYERHEIGHT, playerBitmaps);
		sprite_init(&tower, 5.5, -12, TOWERWIDTH, TOWERHEIGHT, towerBitmaps);
		sprite_init(&door, (TOWERWIDTH + 1) / 2, TOWERHEIGHT - DOORHEIGHT - 12, DOORWIDTH, DOORHEIGHT, doorBitmaps);
		sprite_init(&enemy, 90, -3, ENEMYWIDTH, ENEMYHEIGHT, enemyBitmaps);
		sprite_init(&key, -6, -3, KEYWIDTH, KEYHEIGHT, keyBitmaps);
	}
	if (level == 1){
		sprite_init(&player, LCD_X / 2 - 3, LCD_Y - PLAYERHEIGHT - 3, PLAYERWIDTH, PLAYERHEIGHT, playerBitmaps);
		sprite_init(&door, (TOWERWIDTH + 1) / 2, TOWERHEIGHT - DOORHEIGHT, DOORWIDTH, DOORHEIGHT, doorBitmaps);
		sprite_init(&enemy, LCD_X - 8, LCD_Y / 2, ENEMYWIDTH, ENEMYHEIGHT, enemyBitmaps);
		sprite_init(&key, 5, LCD_Y / 2, KEYWIDTH, KEYHEIGHT, keyBitmaps);
	}
}

void draw_level(int lvl){

	if (lvl == 0){
		sprite_draw(&tower);
		sprite_draw(&door);
		sprite_draw(&enemy);
	}
	if (lvl == 0){
		sprite_draw(&tower);
		sprite_draw(&door);
		sprite_draw(&enemy);
		sprite_draw(&key);
	}
}

void setup(void){
	//SET CLOCK SPEED
	set_clock_speed(CPU_8MHz);

	//INITIALIZE SCREEN TO DEFAULT CONTRAST
	lcd_init(LCD_DEFAULT_CONTRAST);

	lcd_clear();

	sei();

	serial_USB();

	//INITIALIZE JOYSTICK, SWITCHES, POMETERS AND LEDS
	init_teensy();

	flr = 0; 
	score = 0; 
	lives = 3; 

	boolKey = 0; 
	pausing = 0; 
	
	time = 0;
	min = 0;
	
	gameOver = 0; 
	gameStart = 0;

	create_floor(flr);

	cDownOut = 3;
	cDown = 11;

	serial_outputs();
}

int pause_status(void){
	if (BIT_IS_SET(PINB, 0)){

		sprintf(flr_msg, "Floor: %d", flr);
		sprintf(score_msg, "Score: %d", score);
		sprintf(liv_msg, "Lives: %d", lives);

		clear_screen();

		draw_in_border();

		draw_string(15,2, "P A U S E D", FG_COLOUR);
		draw_string(15, LCD_Y / 5 + 2, flr_msg, FG_COLOUR);
		draw_double(15,LCD_Y / 5 * 2 + 2, time, min, FG_COLOUR);
		draw_string(15,LCD_Y / 5 * 3 + 2, score_msg, FG_COLOUR);
		draw_string(15,LCD_Y / 5 * 4 + 2, liv_msg, FG_COLOUR);

		player.is_visible = 0;

		show_screen();
		return 1;
	}

	return 0;
}

void loading_screen(void){
	clear_screen();

	sprintf(load_score_msg, "Score: %d", score);
	sprintf(load_flr_msg, "Floor: %d", flr);

	draw_string(19, 3, "LOADING ...", FG_COLOUR);
	draw_string(20, LCD_Y / 3 + 2, flr_msg, FG_COLOUR);
	draw_string(20,LCD_Y / 3 * 2 + 2, score_msg, FG_COLOUR);

	LCD_CMD(lcd_set_display_mode, lcd_display_inverse);
	show_screen();

	loading = 0;
}

void check_lvl_hits(void){
	if (lives <= 0){
		gameOver = 1;
	}
	if (sprite_collision(player, key) == 1){
		boolKey = 1;
		sprintf(keyCollect,"[%d] Player found the Key", gameTime);
		usb_serial_write((uint8_t *) keyCollect, strlen(keyCollect));
	}
	if (sprite_collision(player,door) == 1 && boolKey == 1){		
		flr++;
		score = score + 100;
		boolKey = 0;
		loading = 1;
	} else if ((sprite_collision(player,door) == 1 && boolKey == 0) || (sprite_collision(player,tower))) {
		player.y++;
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

void move_player(void){
	player.is_visible = 1;

	//object movement is inversed to show correct player movement;

	//UP
	if (BIT_IS_SET(PIND, 1)){
		if (top.y + top.height <= 2 && player.y + round(PLAYERHEIGHT / 2) + 1 == LCD_Y / 2){
			tower.y++;
			key.y++;
			left.y++;
			right.y++;
			top.y++;
			bottom.y++;
			door.y++;
			enemy.y++;
		} else {
			if (wall_collision_movement(player) == 0){
				player.y--;
			}
		}
		
	} else 
	//DOWN
	if (BIT_IS_SET(PINB, 7)){
		if (bottom.y > LCD_Y - 2 && player.y + round(PLAYERHEIGHT / 2)  + 1 == LCD_Y / 2){
			tower.y--;
			key.y--;
			left.y--;
			right.y--;
			top.y--;
			bottom.y--;
			door.y--;
			enemy.y--;
		} else{
			if (wall_collision_movement(player) == 0){
				player.y++;
			}
		}
	} else
	//LEFT
	if (BIT_IS_SET(PINB, 1)){	
		if (left.x + left.width  < 2 && player.x + round(PLAYERWIDTH / 2) + 1 == LCD_X / 2){
			tower.x++;
			key.x++;
			left.x++;
			right.x++;
			top.x++;
			bottom.x++;
			door.x++;
			enemy.x++;
		} else {
			if (wall_collision_movement(player) == 0){
				player.x--;
			}
		}
		
	} else
	//RIGHT
	if (BIT_IS_SET(PIND, 0)){
		if (right.x + right.width > LCD_X - 2 && player.x + round(PLAYERWIDTH / 2) + 1 == LCD_X / 2){
			tower.x--;
			key.x--;
			left.x--;
			right.x--;
			top.x--;
			bottom.x--;
			door.x--;
			enemy.x--;
		} else {
			if (wall_collision_movement(player) == 0){
				player.x++;
			}
		}
	}

	if (player.is_visible == 1){
		sprite_draw(&player);
	}

}

void enemy_crawl(void){
	int play_x = player.x, play_y = player.y;

	if (sprite_collision(player, enemy) == 1){
		lives--;

		sprite_init(&player, round((LCD_X - PLAYERWIDTH) / 2), round((LCD_Y - PLAYERHEIGHT) / 2), PLAYERWIDTH, PLAYERHEIGHT, playerBitmaps);
	}

	if (enemy.x <= 83 && enemy.x >= 0){
		if (enemy.y <= 47 && enemy.y >= 0){
			if (enemy.x < play_x){
				enemy.x += 0.1;
			} 
			if (enemy.x > play_x){
				enemy.x -= 0.1;
			}
			if (enemy.y < play_y){
				enemy.y += 0.1;
			}
			if (enemy.y > play_y){
				enemy.y -= 0.1;
			}
		}
	}
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

void game_over(void);

int main(void){
	setup();

	serial_outputs();

	while(gameStart == 0){
		start_screen();
	}

	while (gameOver == 0) {
		if (loading == 0){
			run_time();
			if (pause_status() == 0){
				process();
				_delay_ms(10);
			} else {
				pause_status();
			}
			} else {
			loading_screen();
		}
	}	

	while(gameOver == 1){
		game_over();
	}
	return 0;
}

void game_over(void){

	clear_screen();

	sprintf(score_msg, "Score: %d", score);
	sprintf(flr_msg, "Floor: %d", flr);
	sprintf(liv_msg, "Lives: %d", lives);

	draw_string(19, 3, "GAME OVER", FG_COLOUR);
	draw_string(20, LCD_Y / 5 + 2, score_msg, FG_COLOUR);
	draw_string(20,LCD_Y / 5 * 2 + 2, flr_msg, FG_COLOUR);
	draw_string(20,LCD_Y / 5 * 3 + 2, liv_msg, FG_COLOUR);
	draw_string(22,LCD_Y / 5 * 4 + 1, "restart", BG_COLOUR);

	LCD_CMD(lcd_set_display_mode, lcd_display_inverse);
	show_screen();
	clear_screen();

	if (BIT_IS_SET(PINF,5) || BIT_IS_SET(PINF, 6)){
		gameOver = 0;
		main();
		clear_screen();
	}
}