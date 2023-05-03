#ifndef _TETRIS_H
#define _TETRIS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "usb_kb/GenericTypeDefs.h"
#include <system.h>
#include <alt_types.h>
#include <time.h>

struct TEXT_VGA_STRUCT {
	unsigned int VRAM [20]; //Week 2 - extended VRAM
	unsigned int currBlock [20];
	unsigned int colors [20];
	unsigned int rotBlock [20];
	unsigned int color_block [20];
	//modify this by adding const bytes to skip to palette, or manually compute palette
	unsigned int game_start;
	unsigned int over;
	unsigned char score0;
	unsigned char score1;
	unsigned char score2;
	unsigned char score3;
	unsigned char score4;
	unsigned char score5;
	int score;
};

//you may have to change this line depending on your platform designer
static volatile struct TEXT_VGA_STRUCT* vga_ctrl = VGA_TEXT_MODE_CONTROLLER_0_BASE;

void gen_block();
void rotate();
void move();
void removeLine();
void helperRemove(int row);
void updateColorBlock();
void updateColor();
void moveLeft();
void moveRight();
void moveDown();
void startGame();
void endGame();
void updateScore();
bool checkstart();

#endif
