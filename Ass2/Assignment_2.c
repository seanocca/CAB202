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

/*-----------------------------------------------------------------------------------------------------*\

											CONSTANT DEFINE

\*-----------------------------------------------------------------------------------------------------*/

#define FREQ (8000000.0)
#define PRESCALE (256.0)
#define THRESHOLD (1000)

#define NUM_WALLS_DOWN (3)
#define NUM_WALLS_ACROSS (3)
#define MAX_ENEMY_TREASURE (5)

/*-----------------------------------------------------------------------------------------------------*\

									SETUP ALL FUNCTIONS TO BE CALLED

\*-----------------------------------------------------------------------------------------------------*/

void serial_USB(void);
double get_serial_time(void);
void serial_outputs(void);

void run_time(void);

int sprite_collision(Sprite* collide, Sprite* object);
int border_wall_collision_movement(Sprite* hero);
int sprite_drop_collision(Sprite* sprited);
void check_lvl_hits(void);
void item_follow(void);

void move_player(void);
void enemy_crawl(void);

void restart_game(void);

void init_teensy(void);

void random_level_generator(void);
void random_level_drawer(void);

void create_scroll_border(void);
void draw_scroll_border(void);
void draw_in_border(void);

void sprites_init(void);
void place_sprites(int lvl);

void draw_level(int lvl);

void show_load_screen(void);

void draw_double(int x, int y, double seconds, double minutes, colour_t colour);

int pause_status(double min, double sec);
void start_screen(void);
void game_over(void);

int rand_between(int min, int max);

/*-----------------------------------------------------------------------------------------------------*\

												VARIABLES

\*-----------------------------------------------------------------------------------------------------*/

Sprite player;
Sprite tower;

Sprite key;
Sprite door;

Sprite left;
Sprite right;
Sprite top;
Sprite bottom;

Sprite bomb;
Sprite bow;
Sprite shield;

Sprite enemy[MAX_ENEMY_TREASURE];
Sprite treasure[MAX_ENEMY_TREASURE];

Sprite walls_down[NUM_WALLS_DOWN];
Sprite walls_across[NUM_WALLS_ACROSS];

Sprite doors[4];

int num_enemies = 0, num_treasures,
	pausing = 0, gameOver = 0, gameStart = 0,
	received = 0, flr = 0, lives = 0, boolKey = 0,
	loading = 0, score = 0, bomb_check = 0, shield_check = 0, bow_check = 0;

int shield_likelihood, bomb_likelihood, bow_likelihood; 

double time = 0, min = 0;

double pl_x, pl_y;
 
volatile int oflow_counter = 0, gameTime = 0;
 
char buffer[81];

char *keyCollect = "Player has found the Key";
char *treasureCollect = "Player has found one of the Treasures";
char *shieldCollect = "Player has found the Shield";
char *bombCollect = "Player has found the Bomb";
char *bowCollect = "Player has found the Bow";

char *completeLvl = "Player has completed this Level";
char *gameDone = "GAME OVER";

char *flr_msg = "Floor: %d";
char *tim_msg = "Time: %02f:%02f";
char *score_msg = "Score: %0d";
char *liv_msg = "Lives: %d";
char *output = "[%f] Score: %i Floor: %i Lives: %i Pos: (%0.2f, %0.2f)";

/*-----------------------------------------------------------------------------------------------------*\

											UNAMED ASSIGNMENT STUFF

\*-----------------------------------------------------------------------------------------------------*/

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

	//SETUP 16 BIT TIMER
	TCCR1A = 0;
	TCCR1B = 4;
	TIMSK1 = 1;

	/*//SETUP 8 BIT TIMER
	TCCR0A = 0;
	TCCR0B = 4;
	TIMSK0 = 1;*/
	
}

void usb_serial_send(char*value){
	unsigned char count = 0;
	while(*value != '\0'){
		usb_serial_putchar(*value);
		value++;
		count++;
	}

	usb_serial_putchar('\r');
	usb_serial_putchar('\n');
}

void serial_outputs(void){

	sprintf(output, "%f Score: %0d Floor: %d Lives: %d Pos: %0.2f, %0.2f", get_serial_time(), score, flr, lives, pl_x, pl_y);

	usb_serial_send(output);

}
 
ISR(TIMER1_OVF_vect) {
	if (gameStart == 1){
	    oflow_counter++;
	    if (round(oflow_counter) == 1800){
	    	oflow_counter = 0;
	    	min++;
	    }
	}
}

/*ISR(TIMER0_OVF_vect){	
	gameTime++;	
	if (gameTime % 60 == 0 && gameStart == 1){
		serial_outputs();
	}
}*/

void run_time(void){
	time = ( oflow_counter * 65536.0 + TCNT1 ) * PRESCALE  / FREQ;
}

double get_serial_time(void){
	return ( gameTime * 256.0 + TCNT0 ) * PRESCALE  / FREQ;
}

void serial_USB(void){
	//Allow interruptions
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
	srand(get_serial_time());

	//SET CLOCK SPEED
	set_clock_speed(CPU_8MHz);

	//INITIALIZE SCREEN TO DEFAULT CONTRAST
	lcd_init(LCD_DEFAULT_CONTRAST);

	lcd_clear();

	sei();

	serial_USB();

	//INITIALIZE JOYSTICK, SWITCHES, POMETERS AND LEDS
	init_teensy();

	lives = 3;

	boolKey = 1; 
	pausing = 0; 

	flr = 0;
	score = 0;

	sprites_init();

	place_sprites(flr);
}

void check_lvl_hits(void){
	if (lives <= 0){
		gameOver = 1;
		gameStart = 0;
		usb_serial_send(gameDone);
	}
	if (sprite_collision(&player, &key) && boolKey == 0){
		boolKey = 1;
		usb_serial_send(keyCollect);
	}

	if (sprite_collision(&player, &door) == 1 && boolKey == 1){
		flr++;
		score += 100;
		boolKey = 0;
		bomb_check = 0;
		shield_check = 0;
		bow_check = 0;
		for(int i = 0; i < NUM_WALLS_ACROSS; i++){
			walls_down[i].x = -1000;
			walls_across[i].x = -1000;
		}
		show_load_screen();
		_delay_ms(5);
		usb_serial_send(completeLvl);
		_delay_ms(5);
		place_sprites(flr);
	}

	for (int i = 0; i < num_treasures; i++){
		if (sprite_collision(&player, &treasure[i])){
			usb_serial_send(treasureCollect);
			treasure[i].x = -1000;
			_delay_ms(5);
			score += 15;
		}
	}

	if (sprite_collision(&player, &bomb) && bomb_check == 0){
		usb_serial_send(bombCollect);
		bomb_check = 1;
		bow_check = 0;
		shield_check = 0;
	}

	if (sprite_collision(&player, &shield) && shield_check == 0){
		usb_serial_send(shieldCollect);
		shield_check = 1;
		bomb_check = 0;
		bow_check = 0;
	}

	if (sprite_collision(&player, &bow) && bow_check == 0){
		usb_serial_send(bowCollect);
		bow_check = 1;
		bomb_check = 0;
		shield_check = 0;
	}

}

void item_follow(void){
	if (boolKey == 1){
		//UP
		if (BIT_IS_SET(PIND, 1)){
			key.y = player.y + player.height + key.height;
			key.x = player.x - 3;
		} else 
		//DOWN
		if (BIT_IS_SET(PINB, 7)){
			key.y = player.y - key.height - key.height;
			key.x = player.x;
		} else
		//LEFT
		if (BIT_IS_SET(PINB, 1)){	
			key.x = player.x + key.width + 3;
			key.y = player.y + player.height - key.height;			
		} else
		//RIGHT
		if (BIT_IS_SET(PIND, 0)){
			key.x = player.x - key.width - 3;
			key.y = player.y + player.height - key.height;	
		}
	}

	if (bomb_check == 1){
		//UP
		if (BIT_IS_SET(PIND, 1)){
			if (boolKey == 0){
				bomb.y = player.y + player.height;
				bomb.x = player.x - 2;
			}else {
				bomb.y = player.y + player.height + key.height;
				bomb.x = player.x - 2;
			}
		} else 
		//DOWN
		if (BIT_IS_SET(PINB, 7)){
			if (boolKey == 0){
				bomb.y = player.y - bomb.height;
				bomb.x = player.x - 2;
			}else {
				bomb.y = player.y - key.height - bomb.height - 1;
				bomb.x = player.x - 2;
			}
		} else
		//LEFT
		if (BIT_IS_SET(PINB, 1)){	
			if (boolKey == 0){
				bomb.y = player.y + player.height - bomb.height - 1;
				bomb.x = player.x + player.width;
			}else {
				bomb.y = player.y - key.height - 1;
				bomb.x = player.x + player.width;
			}			
		} else
		//RIGHT
		if (BIT_IS_SET(PIND, 0)){
			if (boolKey == 0){
				bomb.y = player.y + player.height - bomb.height - 1;
				bomb.x = player.x - bomb.width;
			}else {
				bomb.y = player.y + player.height - key.height - 1;
				bomb.x = player.x - bomb.width - key.height;
			}
		}
	}

	if (shield_check == 1){
		//UP
		if (BIT_IS_SET(PIND, 1)){
			if (boolKey == 0){
				shield.y = player.y + player.height;
				shield.x = player.x - 2;
			}else {
				shield.y = player.y + player.height + key.height;
				shield.x = player.x - 2;
			}
		} else 
		//DOWN
		if (BIT_IS_SET(PINB, 7)){
			if (boolKey == 0){
				shield.y = player.y - shield.height;
				shield.x = player.x - 2;
			}else {
				shield.y = player.y - key.height - shield.height - 1;
				shield.x = player.x - 2;
			}
		} else
		//LEFT
		if (BIT_IS_SET(PINB, 1)){	
			if (boolKey == 0){
				shield.y = player.y + player.height - shield.height - 1;
				shield.x = player.x + player.width;
			}else {
				shield.y = player.y - key.height - 1;
				shield.x = player.x + player.width;
			}			
		} else
		//RIGHT
		if (BIT_IS_SET(PIND, 0)){
			if (boolKey == 0){
				shield.y = player.y + player.height - shield.height - 1;
				shield.x = player.x - shield.width;
			}else {
				shield.y = player.y + player.height - key.height - 1;
				shield.x = player.x - shield.width - key.width - 3;
			}
		}
	}

	if (bow_check == 1){
		//UP
		if (BIT_IS_SET(PIND, 1)){
			if (boolKey == 0){
				bow.y = player.y + player.height;
				bow.x = player.x - 2;
			}else {
				bow.y = player.y + player.height + key.height;
				bow.x = player.x - 2;
			}
		} else 
		//DOWN
		if (BIT_IS_SET(PINB, 7)){
			if (boolKey == 0){
				bow.y = player.y - bow.height;
				bow.x = player.x - 2;
			}else {
				bow.y = player.y - key.height - bow.height - 1;
				bow.x = player.x - 2;
			}
		} else
		//LEFT
		if (BIT_IS_SET(PINB, 1)){	
			if (boolKey == 0){
				bow.y = player.y + player.height - bow.height - 1;
				bow.x = player.x + player.width;
			}else {
				bow.y = player.y - key.height - 1;
				bow.x = player.x + player.width;
			}			
		} else
		//RIGHT
		if (BIT_IS_SET(PIND, 0)){
			if (boolKey == 0){
				bow.y = player.y + player.height - bow.height - 1;
				bow.x = player.x - bow.width;
			}else {
				bow.y = player.y + player.height - key.height - 1;
				bow.x = player.x - bow.width - key.width;
			}
		}
	}
}

void process(void){

	pl_x = player.x - left.x - left.width;
	pl_y = player.y - top.y - top.height;

	clear_screen();

	draw_scroll_border();

	move_player();

	item_follow();

	draw_level(flr);

	enemy_crawl();

	check_lvl_hits();

	show_screen();
}

int main(void){	
	srand(get_serial_time());
	setup();

	while(gameStart == 0){start_screen();}

	while (gameOver == 0 && gameStart == 1) {
		run_time();	
		if (pause_status(time, min) == 0){
			process();
		} else {
			pause_status(time, min);
		}
	}	
	while(gameOver == 1){
		restart_game();
		game_over();
	}
	return 0;
}

void restart_game(void){
	if (BIT_IS_SET(PINF,5) || BIT_IS_SET(PINF, 6)){	
		gameOver = 0;
		gameStart = 0;
		lives = 3;
		oflow_counter = 0;
		time = 0; min = 0;
		boolKey = 0;

		while(BIT_IS_SET(PINF,5) || BIT_IS_SET(PINF, 6));

		usb_serial_set_control(usb_serial_get_control());
		clear_screen();
	}
}

int sprite_collision(Sprite* collide, Sprite* object){
	int coll_left = collide->x;
	int coll_right = collide->x + collide->width;
	int coll_bott = collide->y + collide->height;
	int coll_top = collide->y;
	int obj_left = object->x;
	int obj_right = object->x + object->width;
	int obj_bott = object->y + object->height;
	int obj_top = object->y;

	if (coll_left <= obj_right && coll_right >= obj_left){
		if (coll_top <= obj_bott && coll_bott >= obj_top){
			return 1;
		}
	} 
	return 0;	
}

int border_wall_collision_movement(Sprite* hero){
	if (sprite_collision(hero,&left ) ||
		sprite_collision(hero,&right) ||
		sprite_collision(hero,&bottom) ||
		sprite_collision(hero, &top)){return 1;}

	if (sprite_collision(hero,&tower))return 1;

	if ((sprite_collision(hero, &walls_down[0])) || 
   (sprite_collision(hero, &walls_down[1])) ||
   (sprite_collision(hero, &walls_down[2])) ||
   (sprite_collision(hero, &walls_across[0])) ||
   (sprite_collision(hero, &walls_across[1])) ||
   (sprite_collision(hero, &walls_across[2]))) {return 1;}

   return 0;
}

int sprite_drop_collision(Sprite* sprited){
	if ((sprite_collision(sprited, &walls_down[0]) && sprited != &walls_down[0]) || 
	   (sprite_collision(sprited, &walls_down[1]) && sprited != &walls_down[1]) ||
	   (sprite_collision(sprited, &walls_down[2]) && sprited != &walls_down[2]) ||
	   (sprite_collision(sprited, &walls_across[0]) && sprited != &walls_across[0]) ||
	   (sprite_collision(sprited, &walls_across[1]) && sprited != &walls_across[1]) ||
	   (sprite_collision(sprited, &walls_across[2]) && sprited != &walls_across[2]) ||
	   (sprite_collision(sprited, &shield) && sprited != &shield) ||
	   (sprite_collision(sprited, &bow) && sprited != &bow) ||
	   (sprite_collision(sprited, &bomb) && sprited != &bomb) ||
	   (sprite_collision(sprited, &enemy[0]) && sprited != &enemy[0]) ||
	   (sprite_collision(sprited, &enemy[1]) && sprited != &enemy[1]) ||
	   (sprite_collision(sprited, &enemy[2]) && sprited != &enemy[2]) ||
	   (sprite_collision(sprited, &enemy[3]) && sprited != &enemy[3]) ||
	   (sprite_collision(sprited, &enemy[4]) && sprited != &enemy[4]) ||
	   (sprite_collision(sprited, &treasure[0]) && sprited != &treasure[0]) ||
	   (sprite_collision(sprited, &treasure[1]) && sprited != &treasure[1]) ||
	   (sprite_collision(sprited, &treasure[2]) && sprited != &treasure[2]) ||
	   (sprite_collision(sprited, &treasure[3]) && sprited != &treasure[3]) ||
	   (sprite_collision(sprited, &treasure[4]) && sprited != &treasure[4]) ||
	   (sprite_collision(sprited, &door) && sprited != &door)) {
		return 1;
	} else {return 0;}
}

void move_player(void){
	player.is_visible = 1;

	//object movement is inversed to show correct player movement;
	if (flr == 0){
		//UP
		if (BIT_IS_SET(PIND, 1)){
			if (top.y + top.height <= 2 && player.y + round(PLAYERHEIGHT / 2) + 1 <= LCD_Y / 2){
				tower.y++;
				key.y++;
				left.y++;
				right.y++;
				top.y++;
				bottom.y++;
				door.y++;
				enemy[0].y++;
			} else {
				if (!border_wall_collision_movement(&player)){player.y--;} 
				else{player.y += 2;}
			}			
		} else 
		//DOWN
		if (BIT_IS_SET(PINB, 7)){
			if (bottom.y >= LCD_Y - 2 && player.y + round(PLAYERHEIGHT / 2)  + 1 >= LCD_Y / 2){
				tower.y--;
				key.y--;
				left.y--;
				right.y--;
				top.y--;
				bottom.y--;
				door.y--;
				enemy[0].y--;
			} else{
				if (!border_wall_collision_movement(&player)){player.y++;} 
				else {player.y -= 2;}
			}
		} else
		//LEFT
		if (BIT_IS_SET(PINB, 1)){	
			if (left.x + left.width <= 2 && player.x + round(PLAYERWIDTH / 2) + 1 <= LCD_X / 2){
				tower.x++;
				key.x++;
				left.x++;
				right.x++;
				top.x++;
				bottom.x++;
				door.x++;
				enemy[0].x++;
			} else {
				if (!border_wall_collision_movement(&player)){player.x--;} 
				else {player.x += 2;}
			}
		} else
		//RIGHT
		if (BIT_IS_SET(PIND, 0)){
			if (right.x + right.width >= LCD_X - 2 && player.x + round(PLAYERWIDTH / 2) + 1 >= LCD_X / 2){
				tower.x--;
				key.x--;
				left.x--;
				right.x--;
				top.x--;
				bottom.x--;
				door.x--;
				enemy[0].x--;
			} else {
				if (!border_wall_collision_movement(&player)){player.x++;} 
				else {player.x += 2;}
			}
		}
	} 
	
	if (flr >= 1){
		//UP
		if (BIT_IS_SET(PIND, 1)){
			if (top.y + top.height <= 2 && player.y + round(PLAYERHEIGHT / 2) + 1 <= LCD_Y / 2
				){
				left.y++;
				right.y++;
				top.y++;
				bottom.y++;
				door.y++;
				key.y++;
				bomb.y++;
				shield.y++;
				bow.y++;
				for (int i = 0; i < num_treasures; i++){treasure[i].y++;}
				for (int i = 0; i < num_enemies; i++){enemy[i].y++;}
				for (int i = 0; i < NUM_WALLS_ACROSS; i++){walls_across[i].y++;}
				for (int i = 0; i < NUM_WALLS_DOWN; i++){walls_down[i].y++;}
			} else {
				if (!border_wall_collision_movement(&player)){player.y--;} 
				else{player.y += 2;}
			}
			
		} else 
		//DOWN
		if (BIT_IS_SET(PINB, 7)){
			if (bottom.y + bottom.height > LCD_Y && player.y + round(PLAYERHEIGHT / 2) + 1 >= LCD_Y / 2){
				left.y--;
				right.y--;
				top.y--;
				bottom.y--;
				door.y--;
				key.y--;
				bomb.y--;
				shield.y--;
				bow.y--;
				for (int i = 0; i < num_treasures; i++){treasure[i].y--;}
				for (int i = 0; i < num_enemies; i++){enemy[i].y--;}
				for (int i = 0; i < NUM_WALLS_ACROSS; i++){walls_across[i].y--;}
				for (int i = 0; i < NUM_WALLS_DOWN; i++){walls_down[i].y--;}
			} else{
				if (!border_wall_collision_movement(&player)){player.y++;} 
				else {player.y -= 2;}
			}
		} else
		//LEFT
		if (BIT_IS_SET(PINB, 1)){	
			if (left.x < 0 && player.x + round(PLAYERWIDTH / 2) + 1 <= LCD_X / 2){
				left.x++;
				right.x++;
				top.x++;
				bottom.x++;
				door.x++;
				key.x++;
				bomb.x++;
				shield.x++;
				bow.x++;
				for (int i = 0; i < num_treasures; i++){treasure[i].x++;}
				for (int i = 0; i < num_enemies; i++){enemy[i].x++;}				
				for (int i = 0; i < NUM_WALLS_ACROSS; i++){walls_across[i].x++;}
				for (int i = 0; i < NUM_WALLS_DOWN; i++){walls_down[i].x++;}
			} else {
				if (!border_wall_collision_movement(&player)){player.x--;} 
				else {player.x += 2;}
			}
			
		} else
		//RIGHT
		if (BIT_IS_SET(PIND, 0)){
			if (right.x + right.width > LCD_X && player.x + round(PLAYERWIDTH / 2) + 1 >= LCD_X / 2){
				left.x--;
				right.x--;
				top.x--;
				bottom.x--;
				key.x--;
				door.x--;
				bomb.x--;
				shield.x--;
				bow.x--;
				for (int i = 0; i < num_treasures; i++){treasure[i].x--;}
				for (int i = 0; i < num_enemies; i++){enemy[i].x--;}
				for (int i = 0; i < NUM_WALLS_ACROSS; i++){walls_across[i].x--;}
				for (int i = 0; i < NUM_WALLS_DOWN; i++){walls_down[i].x--;}
			} else {
				if (!border_wall_collision_movement(&player)){player.x++;} 
				else {player.x -= 2;}
			}
		}	
	}

	if (player.is_visible == 1){
		sprite_draw(&player);
	}
}

void enemy_crawl(void){
	if (num_enemies == 0) num_enemies = 1;

	for (int i = 0; i < num_enemies; i++){
		if (sprite_collision(&player, &enemy[i]) && shield_check == 0){
			boolKey = 0;
			do {
				player.x = rand_between(0,LCD_X - PLAYERWIDTH - PLAYERWIDTH / 2);
				player.y = rand_between(0,LCD_Y - PLAYERHEIGHT - PLAYERHEIGHT / 2);
			}while(sprite_drop_collision(&player));
			lives--;
		} else if (sprite_collision(&player, &enemy[i]) && shield_check == 1){
			shield_check = 0;
			shield.x = -1000;

			enemy[i].x = -1000;
		}

		if (((enemy[i].y > LCD_Y && enemy[i].y + enemy[i].height - 1 < 0) || 
			(enemy[i].x < LCD_X && enemy[i].x + enemy[i].width - 1 > 0)) && 
			!border_wall_collision_movement(&enemy[i])){
			if (enemy[i].x < player.x)enemy[i].x += 0.1;
			if (enemy[i].x > player.x)enemy[i].x -= 0.1;
			if (enemy[i].y < player.y)enemy[i].y += 0.1;
			if (enemy[i].y > player.y)enemy[i].y -= 0.1;		
		}
	}
}


/*-----------------------------------------------------------------------------------------------------*\

											CREATE ALL SPRITES
										   AND LEVEL GENERATION

\*-----------------------------------------------------------------------------------------------------*/

int rand_between(int mini, int max){
	return rand() % (max + 1 - mini) + mini;
}

void create_scroll_border(void){
	sprite_init(&left,-21 - VERWIDTH,-12 - HORHEIGHT,VERWIDTH,VERHEIGHT,verBitmaps);
	sprite_init(&right,126 - VERWIDTH,-12 - HORHEIGHT,VERWIDTH,VERHEIGHT,verBitmaps);
	sprite_init(&top,-21,-12 - HORHEIGHT,180, HORHEIGHT,horBitmaps);
	sprite_init(&bottom,-21,64 + HORHEIGHT,180,HORHEIGHT,horBitmaps);
}

void reset_scroll_border(void){
	left.x = -21 - VERWIDTH; 		left.y =-12 -HORHEIGHT;
	right.x = 126 - VERWIDTH;		right.y = -12 -HORHEIGHT;
	top.x = -21;					top.y = -12-HORHEIGHT;
	bottom.x = -21;					bottom.y = 64 + HORHEIGHT;
}

void draw_scroll_border(void){
	sprite_draw(&left);
	sprite_draw(&right);
	sprite_draw(&top);
	sprite_draw(&bottom);
}

void sprites_init(void){
	create_scroll_border();

	//PLAYER INIT
	sprite_init(&player, round((LCD_X - PLAYERWIDTH) / 2), round((LCD_Y - PLAYERHEIGHT) / 2), PLAYERWIDTH, PLAYERHEIGHT, playerBitmaps);

	//RANDOM GENERATION OF SPRITES (e.g Monsters, Treasure)
	for (int i = 0; i < MAX_ENEMY_TREASURE; i++){sprite_init(&enemy[i], -1000, 0, ENEMYWIDTH, ENEMYHEIGHT, enemyBitmaps);}
	for (int i = 0; i < MAX_ENEMY_TREASURE; i++){sprite_init(&treasure[i],-1000, 0,TREASUREWIDTH, TREASUREHEIGHT, treasureBitmaps);}

	//GROUND AND LEVEL 1 SPRITES

	sprite_init(&tower, -1000, 0, TOWERWIDTH, TOWERHEIGHT, towerBitmaps);
	sprite_init(&door, -1000, 0, DOORWIDTH, DOORHEIGHT, doorBitmaps);
	sprite_init(&key, -1000, 0, KEYWIDTH, KEYHEIGHT, keyBitmaps);

	//WALLS PAST LEVEL 1
	//WALLS GOING HORIZONTALLY
	sprite_init(&walls_across[0],-1000, 0 ,ACROSSWIDTH,HORHEIGHT,horBitmaps);
	sprite_init(&walls_across[1],-1000, 0 ,ACROSSWIDTH,HORHEIGHT,horBitmaps);
	sprite_init(&walls_across[2],-1000, 0 ,ACROSSWIDTH,HORHEIGHT,horBitmaps);

	//WALLS GOING VERTICALLY
	sprite_init(&walls_down[0],-1000, 0 ,VERWIDTH,DOWNHEIGHT,verBitmaps);
	sprite_init(&walls_down[1],-1000, 0 ,VERWIDTH,DOWNHEIGHT,verBitmaps);
	sprite_init(&walls_down[2],-1000, 0 ,VERWIDTH,DOWNHEIGHT,verBitmaps);

	//DOOR INIT
	for (int i = 0; i < 4; i++){sprite_init(&doors[i], -1000, 0, DOORWIDTH, DOORHEIGHT, doorBitmaps);}

	//DEFENCE ITEMS INIT
	sprite_init(&shield, -1000, 0, SHIELDWIDTH, SHIELDHEIGHT, shieldBitmaps);
	sprite_init(&bomb, -1000, 0, BOMBWIDTH, BOMBHEIGHT, bombBitmaps);
	sprite_init(&bow, -1000, 0, BOWWIDTH, BOWHEIGHT, bowBitmaps);

	//DOOR POSSIBLE LOCATIONS
	doors[0].x = left.x + VERWIDTH + 2; 		doors[0].y = top.y + HORHEIGHT + 2;
	doors[1].x = left.x + VERWIDTH + 2;			doors[1].y = bottom.y - DOORHEIGHT - 2;
	doors[2].x = right.x - DOORWIDTH - 1;		doors[2].y = top.y + HORHEIGHT + 2;
	doors[3].x = right.x - DOORWIDTH - 1;		doors[3].y = bottom.y - DOORHEIGHT - 2;
}

void place_sprites(int lvl){
	reset_scroll_border();
	if (lvl == 0){
		tower.x = 5.5; 					tower.y = -15;
		door.x = (TOWERWIDTH + 1) / 2; 	door.y = TOWERHEIGHT - DOORHEIGHT - 15;
		enemy[0].x = 90;				enemy[0].y = -3;
		key.x = -9;						key.y = -5;
	}else {random_level_generator(); tower.x = -1000;}
}

void draw_level(int lvl){

	if (lvl == 0){
		sprite_draw(&tower);
		sprite_draw(&door);
		sprite_draw(&enemy[0]);
		sprite_draw(&key);
	} else {random_level_drawer();}
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

void random_level_generator(void){

	int max_left = left.x + left.width + 2 + DOORWIDTH + 2,
		max_right = right.x - 2 - DOORWIDTH - 2,
		max_bottom = bottom.y - 2 - DOORHEIGHT - 2;

	int rng;

	rng = rand_between(0,4);

	walls_down[0].x = rand_between(max_left, max_right);

	walls_down[0].y = top.y + top.height; 
	if (walls_down[0].x <= max_left + ACROSSWIDTH){
		walls_across[0].x = walls_down[0].x + walls_down[0].width;
	} else if (walls_down[0].x >= max_right - ACROSSWIDTH) {
		walls_across[0].x = walls_down[0].x - walls_across[0].width + 2;
	} else {
		walls_across[0].x = walls_down[0].x + walls_down[0].width;
	}
	walls_across[0].y = walls_down[0].y + walls_down[0].height - (walls_down[0].width * 2);

	walls_down[1].x = rand_between(max_left, max_right);
	walls_down[1].y = bottom.y - walls_down[0].height;
	if (walls_down[1].x <= max_left + ACROSSWIDTH){
		walls_across[1].x = walls_down[1].x + walls_down[1].width;
	} else if (walls_down[1].x >= max_right - ACROSSWIDTH) {
		walls_across[1].x = walls_down[1].x - walls_across[1].width + 2;
	} else {
		walls_across[1].x = walls_down[1].x - walls_across[1].width + 2;
	}

	walls_across[1].y = rand_between(bottom.y - 76*.35 + walls_across[1].height, max_bottom);

	if (walls_across[0].x >= LCD_X / 2){
		walls_down[2].x = max_left;
	} else {
		walls_down[2].x = max_right - walls_down[2].width;
	}
	walls_down[2].y = top.y + top.height;

	walls_across[2].width = 126 * .25;
	if (walls_across[1].x >= LCD_X / 2){
		walls_across[2].x = left.x + left.width;
	} else {
		walls_across[2].x = right.x - walls_across[2].width + 2;
	}
	walls_across[2].y = max_bottom - walls_across[2].height;

	door = doors[rand_between(0,3)];
	
	do {
		key.x = rand_between(-18, 102 - KEYWIDTH);
		key.y = rand_between(-11, 59 - KEYHEIGHT);
	} while(sprite_drop_collision(&key));

	bow_likelihood = rand_between(0,10);
	bomb_likelihood = rand_between(0,10);
	shield_likelihood = rand_between(0,10);

	if (bow_likelihood <= 3){
		do {
			bow.x = rand_between(left.x + left.width, right.x);
			bow.y = rand_between(top.y + top.height, bottom.y - BOWHEIGHT);
		} while (sprite_drop_collision(&bow));
	}

	if (bomb_likelihood <= 3){
		do {
			bomb.x = rand_between(left.x + left.width, right.x);
			bomb.y = rand_between(top.y + top.height, bottom.y - BOMBHEIGHT);
		} while (sprite_drop_collision(&bomb));
	}
	if (shield_likelihood <= 3){
		do {
			shield.x = rand_between(left.x + left.width, right.x);
			shield.y = rand_between(top.y + top.height, bottom.y - SHIELDHEIGHT);
		} while (sprite_drop_collision(&shield));
	}

	num_enemies = rand_between(1,5);
	num_treasures = rand_between(0,5);

	for (int i = 0; i < num_enemies;i++){
		do{
			enemy[i].x = rand_between(-21,106);
			enemy[i].y = rand_between(-12,65);
		} while (sprite_drop_collision(&enemy[i]));
	}

	for (int i = 0; i < num_treasures;i++){
		do{
			treasure[i].x = rand_between(-21,105);
			treasure[i].y = rand_between(-12,65);
		} while (sprite_drop_collision(&treasure[i]));
	}
}

void random_level_drawer(void){

	sprite_draw(&door);
	sprite_draw(&key);

	for (int i = 0; i < NUM_WALLS_DOWN; i++){sprite_draw(&walls_down[i]);}
	for (int i = 0; i < NUM_WALLS_ACROSS; i++){sprite_draw(&walls_across[i]);}

	for (int i = 0; i < num_treasures;i++){sprite_draw(&treasure[i]);}

	for (int i = 0; i < num_enemies;i++){sprite_draw(&enemy[i]);}

	if (bow_likelihood <= 3){
		sprite_draw(&bow);
	}
	if (bomb_likelihood <= 3){
		sprite_draw(&bomb);
	}
	if (shield_likelihood <= 3){
		sprite_draw(&shield);
	}
}

/*-----------------------------------------------------------------------------------------------------*\

										SCREENS - PAUSE, START, LOAD
										 
\*-----------------------------------------------------------------------------------------------------*/

void show_load_screen(void){
	sprintf(score_msg, "Score: %0d", score);
	sprintf(flr_msg, "Floor: %d", flr);
	sprintf(liv_msg, "Lives: %d", lives);
	clear_screen();
	draw_string(19, 3, "LOADING ...", FG_COLOUR);
	draw_string(20, LCD_Y / 3 + 2, flr_msg, FG_COLOUR);
	draw_string(20,LCD_Y / 3 * 2 + 2, score_msg, FG_COLOUR);
	show_screen();
	_delay_ms(2000);
}

void draw_double(int x, int y, double seconds, double minutes, colour_t colour){
    snprintf(buffer, sizeof(buffer), "Time: %02.0f:%02.0f", minutes, seconds);
    draw_string(x, y, buffer, colour);
}

int pause_status(double min, double sec){

	if (BIT_IS_SET(PINB, 0)){
		/*sprintf(score_msg, "Score: %i", score);
		sprintf(flr_msg, "Floor: %i", flr);
		sprintf(liv_msg, "Lives: %i", lives);*/

		clear_screen();

		draw_in_border();

		draw_string(15,2, "P A U S E D", FG_COLOUR);
		draw_string(15, LCD_Y / 5 + 2, flr_msg, FG_COLOUR);
		draw_double(15,LCD_Y / 5 * 2 + 2, min, sec, FG_COLOUR);
		draw_string(15,LCD_Y / 5 * 3 + 2, score_msg, FG_COLOUR);
		draw_string(15,LCD_Y / 5 * 4 + 2, liv_msg, FG_COLOUR);

		player.is_visible = 0;

		show_screen();
		return 1;
	}

	return 0;
}

void start_screen(void){
	int countPos = 28, cDown = 11, cDownOut = 3;

	char *name = "Sean O'Connell";
	char *studentNum = "n10000569";
	char *readyCount = "Ready?";

	clear_screen();

	draw_in_border();

	draw_string(17,2, "ANSI-TOWER", FG_COLOUR);
	draw_string(8,LCD_Y / 4 + 2, name, FG_COLOUR);
	draw_string(19,LCD_Y / 4 * 2 + 2, studentNum, FG_COLOUR);
	draw_string(countPos,LCD_Y / 4 * 3 + 2, readyCount, FG_COLOUR);

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

	show_screen();
}

void game_over(void){

	sprintf(score_msg, "Score: %i", score);
	sprintf(flr_msg, "Floor: %i", flr);

	clear_screen();

	draw_string(19, 3, "GAME OVER", FG_COLOUR);
	draw_string(20, LCD_Y / 5 + 2, score_msg, FG_COLOUR);
	draw_string(20,LCD_Y / 4 * 2 + 2, flr_msg, FG_COLOUR);
	draw_string(22,LCD_Y / 4 * 3 + 1, "Restart", BG_COLOUR);

	LCD_CMD(lcd_set_display_mode, lcd_display_inverse);
	show_screen();
	clear_screen();
}


/*//ADC
void aim_bot( void ) {
    int right_adc = adc_read(1);
 
    int line_end_x = player.x + (player.width / 2);
    int line_end_y = player.y + (player.height / 2);
    int line_length = 10;
    int start_x = line_end_x + (line_length * cos((float)right_adc / (1023 / 12.7)));
    int start_y = line_end_y + (line_length * sin((float)right_adc / (1023 / 12.7)));
    draw_line (start_x,start_y,line_end_x, line_end_y, FG_COLOUR);
}*/