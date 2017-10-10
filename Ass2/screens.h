#include <avr/io.h> 
#include <avr/interrupt.h>
#include <util/delay.h>
#include <lcd.h>
#include "lcd_model.h"

int boolKey = 0, loading = 0, cDownOut = 3,
cDown = 11, gameStart, gameOver;

char *load_score_msg = "Score: %d";
char *load_flr_msg = "Floor: %d";

char *flr_msg = "Floor: %d";
char *tim_msg = "Time: %02d:%02d";
char *score_msg = "Score: %d";
char *liv_msg = "Lives: %d";

char buffer[20];

void get_load_screen(int scr, int floor){
	sprintf(load_score_msg, "Score: %u", scr);
	sprintf(load_flr_msg, "Floor: %u", flr);
}

void show_load_screen(void){
	clear_screen();
	draw_string(19, 3, "LOADING ...", FG_COLOUR);
	draw_string(20, LCD_Y / 3 + 2, load_flr_msg, FG_COLOUR);
	draw_string(20,LCD_Y / 3 * 2 + 2, load_score_msg, FG_COLOUR);
	show_screen();

	_delay_ms(1000);
	boolKey = 0;

	loading = 0;
}

void draw_double(uint8_t x, uint8_t y, double seconds, double minutes, colour_t colour) {
    snprintf(buffer, sizeof(buffer), "Time: %02.0f:%02.0f", minutes, seconds);
    draw_string(x, y, buffer, colour);
}

int pause_status(int flr, int score, int lives, double min, double sec){
	if (BIT_IS_SET(PINB, 0)){

		sprintf(flr_msg, "Floor: %d", flr);
		sprintf(score_msg, "Score: %d", score);
		sprintf(liv_msg, "Lives: %d", lives);

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