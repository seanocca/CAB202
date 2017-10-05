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

void create_floor(int lvl){

	create_scroll_border();

	if (lvl == 0){
		sprite_init(&player, round((LCD_X - PLAYERWIDTH) / 2), round((LCD_Y - PLAYERHEIGHT) / 2), PLAYERWIDTH, PLAYERHEIGHT, playerBitmaps);
		sprite_init(&tower, 5.5, -12, TOWERWIDTH, TOWERHEIGHT, towerBitmaps);
		sprite_init(&door, (TOWERWIDTH + 1) / 2, TOWERHEIGHT - DOORHEIGHT - 12, DOORWIDTH, DOORHEIGHT, doorBitmaps);
		sprite_init(&enemy, 90, -3, ENEMYWIDTH, ENEMYHEIGHT, enemyBitmaps);
		sprite_init(&key, -6, -3, KEYWIDTH, KEYHEIGHT, keyBitmaps);
	}
	if (lvl == 1){
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