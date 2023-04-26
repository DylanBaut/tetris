#include "tetris.h"

int currLine;
int currPiece;
int colnum;
int rotnum;

int piece[7][4] = {
			{0x4444, 0x0F00, 0x2222, 0x00F0},
            {0xE200, 0x44C0, 0x8E00, 0x6440},
            {0x2E00, 0x4460, 0x0E80, 0xC440},
			{0xCC00, 0xCC00, 0xCC00, 0xCC00},
            {0x4620, 0x06C0, 0x8C40, 0x6C00},
            {0x4640, 0x0E40, 0x4C40, 0x4E00},
            {0x2640, 0x0C60, 0x4C80, 0xC600}
			};

void gen_block() {
	srand(time(0));
	int idx = rand() % 7;
	currPiece = idx;
	rotnum = 0;
	colnum = 4;
	int block = piece[idx][0];
	int i;
	int bit_shift = 12;
	for (i = 0; i<20; i++) {
		vga_ctrl->currBlock[i] = 0;
	}
	for (i = 0; i<4; i++) {
		int val = block >> bit_shift;
		val = val & 0x000F;
		val = val << 3;
		vga_ctrl->VRAM[i] = vga_ctrl->VRAM[i] | val;
		vga_ctrl->currBlock[i] = vga_ctrl->currBlock[i] | val;
		bit_shift-=4;
	}
	currLine = 0;
	/*
	for (i = 0; i<20; i++) {
		printf("%x", vga_ctrl->VRAM[i]);
	}
	*/
}

void rotate() {
	int i;
	bool moveable = true;
	if (currLine<0 || currLine>16) {
		return;
	}
	for (i = 0; i<20; i++) {
		vga_ctrl->VRAM[i] = vga_ctrl->VRAM[i] ^ vga_ctrl->currBlock[i];
		vga_ctrl->rotBlock[i] = 0;
	}
	int oldrot = rotnum;
	rotnum = (rotnum + 1) % 4;
	int bit_shift = 12;
	int block = piece[currPiece][rotnum];
	for (i = 0; i<4; i++) {
		int val = block >> bit_shift;
		val = val & 0x000F;
		val = val << 3;
		vga_ctrl->rotBlock[i] = vga_ctrl->rotBlock[i] | val;
		bit_shift-=4;
	}

	for (i = currLine; i<4; i++) {
		int line = vga_ctrl->rotBlock[i];
		int tmp = vga_ctrl->VRAM[i] & line;
		if (colnum==0 || colnum>16) {
			moveable = false;
			break;
		}
		if (tmp) {
			moveable = false;
			break;
		}
	}
	if (moveable) {
		for (i = currLine; i<4; i++) {
			vga_ctrl->currBlock[i] = vga_ctrl->rotBlock[i];
			vga_ctrl->VRAM[i] = vga_ctrl->VRAM[i] | vga_ctrl->currBlock[i];
		}
	} else {
		for (i = currLine; i<4; i++) {
			vga_ctrl->VRAM[i] = vga_ctrl->VRAM[i] | vga_ctrl->currBlock[i];
		}
		rotnum = oldrot;
	}
}

void move(int key) {
	switch (key) {
	case 0x04:
		moveLeft();
		break;
	case 0x07:
		moveRight();
		break;
	case 0x16:
		moveDown();
		break;
	case 0x1A:
		rotate();
		break;
	default:
		break;
	}
}

void checkFull() {

}

void removeLine() {

}

void moveLeft() {
	int i;
	bool moveable = true;
	if (currLine<0 || currLine>16) {
		return;
	}
	for (i = 0; i<20; i++) {
		vga_ctrl->VRAM[i] = vga_ctrl->VRAM[i] ^ vga_ctrl->currBlock[i];
	}

	for (i = 0; i<19; i++) {
		int line = vga_ctrl->currBlock[i];
		int check = line >> 9;
		if (check%2) {
			moveable = false;
			break;
		}
		line = line << 1;
		int tmp = vga_ctrl->VRAM[i] & line;
		if (tmp) {
			moveable = false;
			break;
		}
	}
	if (moveable) {
		for (i = 0; i<19; i++) {
			vga_ctrl->currBlock[i] = vga_ctrl->currBlock[i] << 1;
			vga_ctrl->VRAM[i] = vga_ctrl->VRAM[i] | vga_ctrl->currBlock[i];
			colnum--;
		}
	} else {
		for (i = 0; i<19; i++) {
			vga_ctrl->VRAM[i] = vga_ctrl->VRAM[i] | vga_ctrl->currBlock[i];
		}
	}
	for (i = 0; i<4; i++) {
		printf("%x", vga_ctrl->VRAM[i]);
		printf("%x\n", vga_ctrl->currBlock[i]);
	}

}

void moveRight() {
	int i;
	bool moveable = true;
	if (currLine<0 || currLine>16) {
		return;
	}
	for (i = 0; i<20; i++) {
		vga_ctrl->VRAM[i] = vga_ctrl->VRAM[i] ^ vga_ctrl->currBlock[i];
	}

	for (i = 0; i<19; i++) {
		int line = vga_ctrl->currBlock[i];
		if (line%2) {
			moveable = false;
			break;
		}
		line = line >> 1;
		int tmp = vga_ctrl->VRAM[i] & line;
		if (tmp) {
			moveable = false;
			break;
		}
	}
	if (moveable) {
		for (i = 0; i<19; i++) {
			vga_ctrl->currBlock[i] = vga_ctrl->currBlock[i] >> 1;
			vga_ctrl->VRAM[i] = vga_ctrl->VRAM[i] | vga_ctrl->currBlock[i];
			colnum++;
		}
	} else {
		for (i = 0; i<19; i++) {
			vga_ctrl->VRAM[i] = vga_ctrl->VRAM[i] | vga_ctrl->currBlock[i];
		}
	}
}

void moveDown() {
	int i;
	bool moveable = true;
	if (currLine<0 || currLine>16) {
		return;
	}
	for (i = 0; i<20; i++) {
		vga_ctrl->VRAM[i] = vga_ctrl->VRAM[i] ^ vga_ctrl->currBlock[i];
	}
	int botLine;
	for (i = 19; i>=0; i--) {
		if (vga_ctrl->currBlock[i]) {
			botLine = i;
			break;
		}
	}
	if (botLine==19) {
		moveable = false;
	}

	for (i = 0; i<4; i++) {
		int line = vga_ctrl->currBlock[botLine];
		int tmp = vga_ctrl->VRAM[botLine+1] & line;
		botLine--;
		if (tmp) {
			moveable = false;
			break;
		}
	}
	if (moveable) {
		for (i = 19; i>=0; i--) {
			if (i==0) vga_ctrl->currBlock[i] = 0;
			else vga_ctrl->currBlock[i] = vga_ctrl->currBlock[i-1];
			vga_ctrl->VRAM[i] = vga_ctrl->VRAM[i] | vga_ctrl->currBlock[i];
		}
	} else {
		for (i = 0; i<19; i++) {
			vga_ctrl->VRAM[i] = vga_ctrl->VRAM[i] | vga_ctrl->currBlock[i];
		}
		//checkFull();
	}
	for (i = 0; i<4; i++) {
		printf("%x", vga_ctrl->VRAM[i]);
		printf("%x\n", vga_ctrl->currBlock[i]);
	}
}

void startGame() {
	int i;
	for (i = 0; i<20; i++) {
		vga_ctrl->VRAM[i] = 0x000;
	}
	gen_block();
}

void endGame() {

}
