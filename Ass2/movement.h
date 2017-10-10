#include <macros.h>
#include "spritecreate.h"

int lives = 3, flr;

void move_player(void){
	player.is_visible = 1;

	//object movement is inversed to show correct player movement;
	if (flr == 0){
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
				} else{
					player.y++;
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
				} else {
					player.y--;
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
				} else {
					player.x++;
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
				enemy.y++;
				key.y++;
				//treasure.y++;
			} else {
				if (wall_collision_movement(player) == 0){
					player.y--;
				} else{
					player.y++;
				}
			}
			
		} else 
		//DOWN
		if (BIT_IS_SET(PINB, 7)){
			if (bottom.y > LCD_Y - 2 && player.y + round(PLAYERHEIGHT / 2)  + 1 == LCD_Y / 2){
				left.y--;
				right.y--;
				top.y--;
				bottom.y--;
				door.y--;
				enemy.y--;
				key.y--;
				//treasure.y--;
			} else{
				if (wall_collision_movement(player) == 0){
					player.y++;
				} else {
					player.y--;
				}
			}
		} else
		//LEFT
		if (BIT_IS_SET(PINB, 1)){	
			if (left.x + left.width  < 2 && player.x + round(PLAYERWIDTH / 2) + 1 == LCD_X / 2){
				left.x++;
				right.x++;
				top.x++;
				bottom.x++;
				door.x++;
				enemy.x++;
				key.x++;
				//treasure.x++;
			} else {
				if (wall_collision_movement(player) == 0){
					player.x--;
				} else {
					player.x++;
				}
			}
			
		} else
		//RIGHT
		if (BIT_IS_SET(PIND, 0)){
			if (right.x + right.width > LCD_X - 2 && player.x + round(PLAYERWIDTH / 2) + 1 == LCD_X / 2){
				left.x--;
				right.x--;
				top.x--;
				bottom.x--;
				key.x--;
				door.x--;
				enemy.x--;
				//treasure.x--;
			} else {
				if (wall_collision_movement(player) == 0){
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
				wall_1.y++;
				wall_2.y++;
				wall_3.y++;
				wall_4.y++;
				//wall_5.y++;
				//wall_6.y++;
				door.y++;
				enemy.y++;
				key.y++;
				//treasure.y++;
			} else {
				if (wall_collision_movement(player) == 0){
					player.y--;
				} else{
					player.y++;
				}
			}
			
		} else 
		//DOWN
		if (BIT_IS_SET(PINB, 7)){
			if (bottom.y > LCD_Y - 2 && player.y + round(PLAYERHEIGHT / 2)  + 1 == LCD_Y / 2){
				left.y--;
				right.y--;
				top.y--;
				bottom.y--;
				wall_1.y--;
				wall_2.y--;
				wall_3.y--;
				wall_4.y--;
				//wall_5.y--;
				//wall_6.y--;
				door.y--;
				enemy.y--;
				key.y--;
				//treasure.y--;
			} else{
				if (wall_collision_movement(player) == 0){
					player.y++;
				} else {
					player.y--;
				}
			}
		} else
		//LEFT
		if (BIT_IS_SET(PINB, 1)){	
			if (left.x + left.width  < 2 && player.x + round(PLAYERWIDTH / 2) + 1 == LCD_X / 2){
				left.x++;
				right.x++;
				top.x++;
				bottom.x++;
				wall_1.x++;
				wall_2.x++;
				wall_3.x++;
				wall_4.x++;
				//wall_5.x++;
				//wall_6.x++;
				door.x++;
				enemy.x++;
				key.x++;
				//treasure.x++;
			} else {
				if (wall_collision_movement(player) == 0){
					player.x--;
				} else {
					player.x++;
				}
			}
			
		} else
		//RIGHT
		if (BIT_IS_SET(PIND, 0)){
			if (right.x + right.width > LCD_X - 2 && player.x + round(PLAYERWIDTH / 2) + 1 == LCD_X / 2){
				left.x--;
				right.x--;
				top.x--;
				bottom.x--;
				wall_1.x--;
				wall_2.x--;
				wall_3.x--;
				wall_4.x--;
				//wall_5.x--;
				//wall_6.x--;
				key.x--;
				door.x--;
				enemy.x--;
				//treasure.x--;
			} else {
				if (wall_collision_movement(player) == 0){
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

	if (sprite_collision(player, enemy) == 1){
		lives--;

		sprite_init(&player, round((LCD_X - PLAYERWIDTH) / 2), round((LCD_Y - PLAYERHEIGHT) / 2), PLAYERWIDTH, PLAYERHEIGHT, playerBitmaps);
	}

	if (enemy.x <= 83 && enemy.x >= 0){
		if (enemy.y <= 47 && enemy.y >= 0){
			if (enemy.x < player.x){
				enemy.x += 0.1;
			} 
			if (enemy.x > player.x){
				enemy.x -= 0.1;
			}
			if (enemy.y < player.y){
				enemy.y += 0.1;
			}
			if (enemy.y > player.y){
				enemy.y -= 0.1;
			}
		}
	}
}