#include <stdio.h>
#include <stdlib.h>

int sprite_collision(Sprite collide, Sprite object){
	int coll_left = collide.x;
	int coll_right = collide.x + collide.width;
	int coll_bott = collide.y + collide.height;
	int coll_top = collide.y;
	int obj_left = object.x;
	int obj_right = object.x + object.width;
	int obj_bott = object.y + object.height;
	int obj_top = object.y;

	if (coll_left <= obj_right && coll_right >= obj_left){
		if (coll_top <= obj_bott && coll_bott >= obj_top){
			return 1;
		} 
	} 
	return 0;	
}

int wall_collision_movement(Sprite hero){
	if (hero.x <= 3){
		hero.x++;
		return 1;
	}
	if (hero.x + hero.width >= LCD_X - 3){
		hero.x--;
		return 1;
	} 
	if(hero.y <= 3){
		hero.y++;
		return 1;
	} 
	if(hero.y + hero.height >= LCD_Y - 3){
		hero.y--;
		return 1;
	} 
	return 0;
}
