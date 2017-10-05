#include <macros.h>
#include <sprite.c>
#include <sprite.h>


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