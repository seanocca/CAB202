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
void serial_outputs(uint8_t scored, uint8_t c_floor, uint8_t lives_left, double time);

void run_time(void);

int sprite_collision(Sprite* collide, Sprite* object);
int wall_collision_movement(Sprite* hero);
void check_lvl_hits(void);
void key_follow(void);

void move_player(void);
void enemy_crawl(void);

void restart_game(void);

void init_teensy(void);

void random_level_generator(int levels);
void random_level_drawer(int levels);

void create_scroll_border(void);
void draw_scroll_border(void);
void draw_in_border(void);

void sprites_init(void);
void place_sprites(int level);

void draw_level(int lvl);

void get_screen_numbers(int scr, int floor, int live);
void show_load_screen(void);

void draw_double(uint8_t x, uint8_t y, double seconds, double minutes, colour_t colour);

int pause_status(double min, double sec);
void start_screen(void);
void game_over(int lives, int flr, int score);

uint8_t rand_number(void);
uint8_t rand_between(int min, int max);

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

Sprite enemy[MAX_ENEMY_TREASURE];
Sprite treasure[MAX_ENEMY_TREASURE];

Sprite walls_down[NUM_WALLS_DOWN];
Sprite walls_across[NUM_WALLS_ACROSS];

Sprite doors[4];

uint8_t num_enemies, num_treasures;

uint8_t pausing = 0, gameOver = 0, gameStart = 0,
	received = 0, flr, score, lives, boolKey = 0,
	loading = 0;

double time = 0, min = 0;
 
volatile int oflow_counter = 0, gameTime = 0;
 
char buffer[81];

char *keyCollect = "[%d] Player found the Key";


char *flr_msg = "Floor: %d";
char *tim_msg = "Time: %02d:%02d";
char *score_msg = "Score: %d";
char *liv_msg = "Lives: %d";

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

	//SETUP 8 BIT TIMER
	TCCR0A = 0;
	TCCR0B = 4;
	TIMSK0 = 1;
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

void serial_outputs(uint8_t scored, uint8_t c_floor, uint8_t lives_left, double time){

	double pl_x = player.x, pl_y = player.y;

	pl_x -= 4;
	pl_y -= 4;

	char *output = "[%i] Current Score: %i 	Current Floor: %i 	Lives Left: %i";
	char *outTime = "TIME: %f";

	sprintf(outTime, "TIME: %i ", (int)time);
	sprintf(output,"Score: %i | Floor: %i | Lives: %i | Pos: (%0.2f, %0.2f)", scored, c_floor, lives_left, pl_x, pl_y);

	usb_serial_send(outTime);
	usb_serial_send(output);

	if (gameOver == 1){
		gameStart = 0;
	}

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

ISR(TIMER0_OVF_vect){
	gameTime++;
	if ((int)gameTime % 60 == 0 && gameStart == 1){
		serial_outputs(score, flr, lives, get_serial_time());
	}
}

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
	srand(time);

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

	boolKey = 0; 
	pausing = 0; 

	flr = 0;
	score = 0;

	sprites_init();

	place_sprites(flr);
}

void check_lvl_hits(void){
	if (lives <= 0){
		gameOver = 1;
	}
	if (sprite_collision(&player, &key) == 1){
		boolKey = 1;
	}

	if (sprite_collision(&player, &door) == 1 && boolKey == 1){
		flr++;
		score += 100;
		boolKey = 0;
		get_screen_numbers(score, flr, lives);
		place_sprites(flr);
		loading = 1;
	}

	if (flr == 0){
		if ((sprite_collision(&player,&door) == 1 && boolKey == 0) || (sprite_collision(&player,&tower))) {
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

int main(void){	
	setup();

	while(gameStart == 0){
		start_screen();
	}

	while (gameOver == 0 && gameStart == 1) {
		run_time();
		if (loading == 0){
			if (pause_status(time, min) == 0){
				process();
			} else {
				get_screen_numbers(score, flr, lives);
				pause_status(time, min);
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
		oflow_counter = 0;
		time = 0; min = 0;
		while(BIT_IS_SET(PINF,5) || BIT_IS_SET(PINF, 6));
		usb_serial_set_control(usb_serial_get_control());
		main();
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

int wall_collision_movement(Sprite* hero){
	if (hero->x <= 3){
		hero->x++;
		return 1;
	}
	if (hero->x + hero->width >= LCD_X - 3){
		hero->x--;
		return 1;
	} 
	if(hero->y <= 3){
		hero->y++;
		return 1;
	} 
	if(hero->y + hero->height >= LCD_Y - 3){
		hero->y--;
		return 1;
	} 
	return 0;
}

void move_player(void){
	player.is_visible = 1;

	//object movement is inversed to show correct player moaccvement;
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
				if (wall_collision_movement(&player) == 0){
					player.y--;
				} else{
					player.y++;
				}
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
				if (wall_collision_movement(&player) == 0){
					player.y++;
				} else {
					player.y--;
				}
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
				if (wall_collision_movement(&player) == 0){
					player.x--;
				} else {
					player.x++;
				}
			}
			
		} else
		//RIGHT
		if (BIT_IS_SET(PIND, 0)){
			if (right.x + right.width > LCD_X && player.x + round(PLAYERWIDTH / 2) + 1 >= LCD_X / 2){
				tower.x--;
				key.x--;
				left.x--;
				right.x--;
				top.x--;
				bottom.x--;
				door.x--;
				enemy[0].x--;
			} else {
				if (wall_collision_movement(&player) == 0){
					player.x++;
				} else {
					player.x--;
				}
			}
		}
	} 
	if (flr == 1){
		//UP
		if (BIT_IS_SET(PIND, 1)){
			if (top.y + top.height <= 2 && player.y + round(PLAYERHEIGHT / 2) + 1 == LCD_Y / 2){
				left.y++;
				right.y++;
				top.y++;
				bottom.y++;
				door.y++;
				enemy[0].y++;
				key.y++;
				treasure[0].y++;
			} else {
				if (wall_collision_movement(&player) == 0){
					player.y--;
				} else{
					player.y++;
				}
			}
			
		} else 
		//DOWN
		if (BIT_IS_SET(PINB, 7)){
			if (bottom.y >= LCD_Y - 2 && player.y + round(PLAYERHEIGHT / 2)  + 1 == LCD_Y / 2){
				left.y--;
				right.y--;
				top.y--;
				bottom.y--;
				door.y--;
				enemy[0].y--;
				key.y--;
				treasure[0].y--;
			} else{
				if (wall_collision_movement(&player) == 0){
					player.y++;
				} else {
					player.y--;
				}
			}
		} else
		//LEFT
		if (BIT_IS_SET(PINB, 1)){	
			if (left.x + left.width <= 2 && player.x + round(PLAYERWIDTH / 2) + 1 == LCD_X / 2){
				left.x++;
				right.x++;
				top.x++;
				bottom.x++;
				door.x++;
				enemy[0].x++;
				key.x++;
				treasure[0].x++;
			} else {
				if (wall_collision_movement(&player) == 0){
					player.x--;
				} else {
					player.x++;
				}
			}
			
		} else
		//RIGHT
		if (BIT_IS_SET(PIND, 0)){
			if (right.x + right.width > LCD_X && player.x + round(PLAYERWIDTH / 2) + 1 == LCD_X / 2){
				left.x--;
				right.x--;
				top.x--;
				bottom.x--;
				key.x--;
				door.x--;
				enemy[0].x--;
				treasure[0].x--;
			} else {
				if (wall_collision_movement(&player) == 0){
					player.x++;
				} else {
					player.x--;
				}
			}
		}
	}
	if (flr >= 2){
		//UP
		if (BIT_IS_SET(PIND, 1)){
			if (top.y + top.height <= 2 && player.y + round(PLAYERHEIGHT / 2) + 1 == LCD_Y / 2){
				left.y++;
				right.y++;
				top.y++;
				bottom.y++;
				door.y++;
				enemy[0].y++;
				key.y++;
				//treasure.y++;
				for (int i = 0; i < NUM_WALLS_ACROSS; i++){
					walls_across[i].y++;
				}
				for (int i = 0; i< NUM_WALLS_DOWN; i++){
					walls_down[i].y++;
				}
			} else {
				if (wall_collision_movement(&player) == 0){
					player.y--;
				} else{
					player.y++;
				}
			}
			
		} else 
		//DOWN
		if (BIT_IS_SET(PINB, 7)){
			if (bottom.y >= LCD_Y - 2 && player.y + round(PLAYERHEIGHT / 2)  + 1 == LCD_Y / 2){
				left.y--;
				right.y--;
				top.y--;
				bottom.y--;
				door.y--;
				enemy[0].y--;
				key.y--;
				//treasure.y--;
				for (int i = 0; i < NUM_WALLS_ACROSS; i++){
					walls_across[i].y--;
				}
				for (int i = 0; i< NUM_WALLS_DOWN; i++){
					walls_down[i].y--;
				}
			} else{
				if (wall_collision_movement(&player) == 0){
					player.y++;
				} else {
					player.y--;
				}
			}
		} else
		//LEFT
		if (BIT_IS_SET(PINB, 1)){	
			if (left.x + left.width <= 2 && player.x + round(PLAYERWIDTH / 2) + 1 == LCD_X / 2){
				left.x++;
				right.x++;
				top.x++;
				bottom.x++;
				door.x++;
				enemy[0].x++;
				key.x++;
				//treasure.x++;
				for (int i = 0; i < NUM_WALLS_ACROSS; i++){
					walls_across[i].x++;
				}
				for (int i = 0; i< NUM_WALLS_DOWN; i++){
					walls_down[i].x++;
				}
			} else {
				if (wall_collision_movement(&player) == 0){
					player.x--;
				} else {
					player.x++;
				}
			}
			
		} else
		//RIGHT
		if (BIT_IS_SET(PIND, 0)){
			if (right.x + right.width > LCD_X  && player.x + round(PLAYERWIDTH / 2) + 1 == LCD_X / 2){
				left.x--;
				right.x--;
				top.x--;
				bottom.x--;
				key.x--;
				door.x--;
				enemy[0].x--;
				//treasure.x--;
				for (int i = 0; i < NUM_WALLS_ACROSS; i++){
					walls_across[i].x--;
				}
				for (int i = 0; i< NUM_WALLS_DOWN; i++){
					walls_down[i].x--;
				}
			} else {
				if (wall_collision_movement(&player) == 0){
					player.x++;
				} else {
					player.x--;
				}
			}
		}	
	}

	if (player.is_visible == 1){
		sprite_draw(&player);
	}
}

void enemy_crawl(void){

	for (int i = 0; i < MAX_ENEMY_TREASURE; i++){
		if (sprite_collision(&player, &enemy[0]) == 1){
			lives--;

			do {
				player.x = rand_between(0,84 - ENEMYWIDTH);
				player.y = rand_between(0,48 - ENEMYHEIGHT);
			}while(sprite_collision(&player,&tower) || sprite_collision(&player,&door) || 
				  sprite_collision(&player,&left) || sprite_collision(&player,&right) ||
				  sprite_collision(&player,&top) || sprite_collision(&player,&bottom));

		}
	}

	if (enemy[0].x + 1 >= 0 && enemy[0].x + enemy[0].width <= LCD_X){
		if (enemy[0].y = 1 >= 0 && enemy[0].y + enemy[0].height <= LCD_Y){
			if (enemy[0].x < player.x){
				enemy[0].x += 0.1;
			} 
			if (enemy[0].x > player.x){
				enemy[0].x -= 0.1;
			}
			if (enemy[0].y < player.y){
				enemy[0].y += 0.1;
			}
			if (enemy[0].y > player.y){
				enemy[0].y -= 0.1;
			}
		}
	}
}


/*-----------------------------------------------------------------------------------------------------*\

											CREATE ALL SPRITES
										   AND LEVEL GENERATION

\*-----------------------------------------------------------------------------------------------------*/

uint8_t rand_number(void){
	return (rand() % 10) + 1;
}

uint8_t rand_between(int min, int max){
	return (rand() % max + 1 - min) + min;
}

void create_scroll_border(void){
	sprite_init(&left,-21 - VERWIDTH,-12 - HORHEIGHT,VERWIDTH,VERHEIGHT,verBitmaps);
	sprite_init(&right,105 - VERWIDTH,-12 - HORHEIGHT,VERWIDTH,VERHEIGHT,verBitmaps);
	sprite_init(&top,-21,-12 - HORHEIGHT,HORWIDTH, HORHEIGHT,horBitmaps);
	sprite_init(&bottom,-21,76 + HORHEIGHT,HORWIDTH,HORHEIGHT,horBitmaps);
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
	for (int i = 0; i < MAX_ENEMY_TREASURE; i++){		
		sprite_init(&enemy[i], -1000, 0, ENEMYWIDTH, ENEMYHEIGHT, enemyBitmaps);	
	}
	for (int i = 0; i < MAX_ENEMY_TREASURE; i++){
		sprite_init(&treasure[i],-1000, 0,TREASUREWIDTH, TREASUREHEIGHT, treasureBitmaps);
	}

	//GROUND AND LEVEL 1 SPRITES
//	sprite_init(&tower, 5.5, -12, TOWERWIDTH, TOWERHEIGHT, towerBitmaps);
//	sprite_init(&door, (TOWERWIDTH + 1) / 2, TOWERHEIGHT - DOORHEIGHT - 12, DOORWIDTH, DOORHEIGHT, doorBitmaps);
//	sprite_init(&enemy[0], 90, -3, ENEMYWIDTH, ENEMYHEIGHT, enemyBitmaps);
//	sprite_init(&key, -6, -3, KEYWIDTH, KEYHEIGHT, keyBitmaps);

	sprite_init(&tower, -1000, 0, TOWERWIDTH, TOWERHEIGHT, towerBitmaps);
	sprite_init(&door, -1000, 0, DOORWIDTH, DOORHEIGHT, doorBitmaps);
	sprite_init(&key, -1000, 0, KEYWIDTH, KEYHEIGHT, keyBitmaps);

	//WALLS PAST LEVEL 1
	//WALLS GOING HORIZONTALLY
	for(int i = 0; i < NUM_WALLS_ACROSS;i++){
		sprite_init(&walls_across[i],-1000, 0 ,44,HORHEIGHT,horBitmaps);
	}
	//WALLS GOING VERTICALLY
	for (int i = 0; i <NUM_WALLS_DOWN; i++){
		sprite_init(&walls_down[i],-1000, 0 ,VERHEIGHT,27,verBitmaps);
	}

	//DOOR INIT
	for (int i = 0; i < 4; i++){
		sprite_init(&doors[i], -1000, 0, DOORWIDTH, DOORHEIGHT, doorBitmaps);
	}

	//DOOR POSSIBLE LOCATIONS

	doors[0].x = -16; 				doors[0].y = -10;
	doors[1].x = -16; 				doors[1].y = 60 - DOORHEIGHT;
	doors[2].x = 102 - DOORWIDTH;	doors[2].y = -10;
	doors[3].x = 102 - DOORWIDTH;	doors[3].y = 60 - DOORHEIGHT;
}

void place_sprites(int level){
	sprites_init();

	if (level == 0){
		tower.x = 5.5; 					tower.y = -12;
		door.x = (TOWERWIDTH + 1) / 2; 	door.y = TOWERHEIGHT - DOORHEIGHT - 12;
		enemy[0].x = 90;				enemy[0].y = -3;
		key.x = -6;						key.y = -3;
	}
	if (level == 1){
		random_level_generator(1);
	}
	if (level >= 2){
		random_level_generator(2);
	}
}

void draw_level(int lvl){

	if (lvl == 0){
		sprite_draw(&tower);
		sprite_draw(&door);
		sprite_draw(&enemy[0]);
		sprite_draw(&key);
	}
	if (lvl == 1){
		random_level_drawer(1);		
	}
	if (lvl >= 2){
		random_level_drawer(1);
	}
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

void random_level_generator(int levels){
	if (levels == 1){
		door = doors[rand_between(0,4) - 1];
		key.x = rand_between(-18, 102 - KEYWIDTH);
		key.y = rand_between(-11, 59 - KEYHEIGHT);
		enemy[0].x = rand_between(-18, 102 - ENEMYWIDTH);
		enemy[0].y = rand_between(-11, 59 - ENEMYHEIGHT);
		treasure[0].x = rand_between(-18, 102 - TREASUREWIDTH);
		treasure[0].y = rand_between(-11, 59 - ENEMYHEIGHT);

	}
	if (levels == 2){
		door = doors[rand_between(0,4) - 1];
	}
}

void random_level_drawer(int levels){

	if (levels == 1){
		sprite_draw(&door);
		sprite_draw(&key);
		sprite_draw(&enemy[0]);
		sprite_draw(&treasure[0]);
	}
	if (levels >= 2){
		for (int i = 0; i < NUM_WALLS_DOWN; i++){
			sprite_draw(&walls_down[i]);
		}
		for (int i = 0; i < NUM_WALLS_ACROSS; i++){
			sprite_draw(&walls_across[i]);
		}
		
		sprite_draw(&door);
		sprite_draw(&enemy[0]);
	}
}


/*-----------------------------------------------------------------------------------------------------*\

										SCREENS - PAUSE, START, LOAD
										 
\*-----------------------------------------------------------------------------------------------------*/

void get_screen_numbers(int scr, int floor, int live){
	sprintf(score_msg, "Score: %d ", scr);
	sprintf(flr_msg, "Floor: %d ", floor);
	sprintf(liv_msg, "Lives: %d", live);
}

void show_load_screen(void){
	clear_screen();
	draw_string(19, 3, "LOADING ...", FG_COLOUR);
	draw_string(20, LCD_Y / 3 + 2, flr_msg, FG_COLOUR);
	draw_string(20,LCD_Y / 3 * 2 + 2, score_msg, FG_COLOUR);
	show_screen();

	_delay_ms(1000);
	boolKey = 0;

	loading = 0;
}

void draw_double(uint8_t x, uint8_t y, double seconds, double minutes, colour_t colour){
    snprintf(buffer, sizeof(buffer), "Time: %02.0f:%02.0f", minutes, seconds);
    draw_string(x, y, buffer, colour);
}

int pause_status(double min, double sec){
	if (BIT_IS_SET(PINB, 0)){

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

void game_over(int lives, int flr, int score){

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

}