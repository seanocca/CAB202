#include <stdio.h>
#include <stdlib.h>
#include "char.h"

Sprite player;
Sprite tower;
Sprite key;
Sprite enemy;
Sprite door;
Sprite left;
Sprite right;
Sprite top;
Sprite bottom;
Sprite wall_1;
Sprite wall_2;
Sprite wall_3;
Sprite wall_4;
Sprite wall_5;
Sprite wall_6;

int random_number = 0;

int rand_number(void);
void random_level_generator(int levels);
void random_level_drawer(int levels);

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

void create_floor(int lvl){

	create_scroll_border();
	sprite_init(&player, round((LCD_X - PLAYERWIDTH) / 2), round((LCD_Y - PLAYERHEIGHT) / 2), PLAYERWIDTH, PLAYERHEIGHT, playerBitmaps);

	if (lvl == 0){
		sprite_init(&tower, 5.5, -12, TOWERWIDTH, TOWERHEIGHT, towerBitmaps);
		sprite_init(&door, (TOWERWIDTH + 1) / 2, TOWERHEIGHT - DOORHEIGHT - 12, DOORWIDTH, DOORHEIGHT, doorBitmaps);
		sprite_init(&enemy, 90, -3, ENEMYWIDTH, ENEMYHEIGHT, enemyBitmaps);
		sprite_init(&key, -6, -3, KEYWIDTH, KEYHEIGHT, keyBitmaps);
	}
	if (lvl == 1){
		random_level_generator(1);
	}
	if (lvl >= 2){
		random_level_generator(2);
	}
}

void draw_level(int lvl){

	if (lvl == 0){
		sprite_draw(&tower);
		sprite_draw(&door);
		sprite_draw(&enemy);
		sprite_draw(&key);
	}
	if (lvl == 1){
		random_level_drawer(1);		
	}
	if (lvl >= 2){
		random_level_drawer(2);
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

int rand_number(void){
	random_number = (rand() % 10) + 1;
	return random_number;
}

int rand_between(int min, int max){
	random_number = (rand() % max + 1 - min) + min;
	return random_number;
}

void random_level_generator(int levels){
	if (levels == 1){
		int door_x, door_y, key_x, key_y; //, tre_x, tre_y, enemy_x, enemy_y;
		door_x = rand_between(-10,103);
		door_y = rand_between(-19,53);
		key_x = rand_between(-10,105);
		key_y = rand_between(-19,57);

		sprite_init(&door, door_x, door_y,DOORWIDTH,DOORHEIGHT, doorBitmaps);
		sprite_init(&key, key_x, key_y, KEYWIDTH,KEYHEIGHT, keyBitmaps);
	}
	if (levels == 2){
		sprite_init(&wall_1,-19,31,44,HORHEIGHT,horBitmaps);
		sprite_init(&wall_2,-19,49,44,HORHEIGHT,horBitmaps);
		sprite_init(&wall_3,38,31,44,HORHEIGHT,horBitmaps);
		sprite_init(&wall_4,25,43,VERWIDTH,27,verBitmaps);
		sprite_init(&door, -10,62 - DOORWIDTH,DOORWIDTH,DOORHEIGHT,doorBitmaps);
	}
}

void random_level_drawer(int levels){

	if (levels == 1){
		sprite_draw(&door);
		sprite_draw(&key);
		sprite_draw(&enemy);
	}
	if (levels >= 2){
		sprite_draw(&wall_1);
		sprite_draw(&wall_2);
		sprite_draw(&wall_3);
		sprite_draw(&wall_4);
		//sprite_draw(&wall_5);
		//sprite_draw(&wall_6);
		sprite_draw(&door);
		sprite_draw(&enemy);
	}
}