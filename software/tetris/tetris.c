#include "tetris.h"

int currPiece;
int rotnum;
int colnum;

int piece[7][4] = {
			{0x4444, 0xF000, 0x4444, 0xF000},
            {0xE200, 0x44C0, 0x8E00, 0x6440},
            {0x2E00, 0x4460, 0xE800, 0xC440},
			{0xCC00, 0xCC00, 0xCC00, 0xCC00},
            {0x4620, 0x6C00, 0x8C40, 0x6C00},
            {0x4640, 0xE400, 0x4C40, 0x4E00},
            {0x2640, 0xC600, 0x4C80, 0xC600}
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
		vga_ctrl->color_block[i] = 0;
	}
	for (i = 0; i<4; i++) {
		int val = block >> bit_shift;
		val = val & 0x000F;
		val = val << 3;
		vga_ctrl->VRAM[i] = vga_ctrl->VRAM[i] | val;
		vga_ctrl->currBlock[i] = vga_ctrl->currBlock[i] | val;
		bit_shift-=4;
	}
	updateColorBlock();
	updateColor();
	/*
	for (i = 0; i<20; i++) {
		printf("%x", vga_ctrl->VRAM[i]);
	}
	*/
}

void rotate() {
	int i;
	bool moveable = true;
	for (i = 0; i<20; i++) {
		vga_ctrl->VRAM[i] = vga_ctrl->VRAM[i] ^ vga_ctrl->currBlock[i];
		vga_ctrl->rotBlock[i] = 0;
	}
	int oldrot = rotnum;
	rotnum = (rotnum + 1) % 4;
	int bit_shift = 12;
	int block = piece[currPiece][rotnum];
	int top = 0;
	for (i = 0; i<20; i++) {
		if (vga_ctrl->currBlock[i]) {
			top = i;
			break;
		}
	}

	// printf("%d", top);

	if (colnum>6) {
		if (colnum==9) moveable = false;
		else if (colnum==7) {
			int shift = 12;
			int j;
			for (j = 0; j<4; j++) {
				int val = block >> shift;
				val = val & 0x000F;
				if (val%2){
					moveable = false;
					break;
				}
				shift-=4;
			}
		}
		else if (colnum==8) {
			int shift = 12;
			int j;
			for (j = 0; j<4; j++) {
				int val = block >> shift;
				val = val & 0x000F;
				int tmp = val >> 1;
				if (val%2 || tmp%2){
					moveable = false;
					break;
				}
				shift-=4;
			}
		}
	}

	for (i = top; i<top+4; i++) {
		int val = block >> bit_shift;
		val = val & 0x000F;
		if (colnum<=6) val = val << (6 - colnum);
		else if (colnum==7) val = val >> 1;
		else if (colnum==8) val = val >> 2;
		vga_ctrl->rotBlock[i] = vga_ctrl->rotBlock[i] | val;
		bit_shift-=4;
		if (i>19) moveable = false;
	}

	/*
	for (i = 0; i<6; i++) {
		printf("%x\n", vga_ctrl->rotBlock[i]);
	}
	*/

	for (i = top; i<top+4; i++) {
		int line = vga_ctrl->rotBlock[i];
		int tmp = vga_ctrl->VRAM[i] & line;
		if (tmp) {
			moveable = false;
			break;
		}
	}

	/*
	for (i = 0; i<6; i++) {
		printf("%x\n", vga_ctrl->rotBlock[i]);
	}
	*/

	if (moveable) {
		for (i = 0; i<20; i++) {
			vga_ctrl->currBlock[i] = vga_ctrl->rotBlock[i];
			vga_ctrl->VRAM[i] = vga_ctrl->VRAM[i] | vga_ctrl->currBlock[i];
		}
		updateColorBlock();
		updateColor();
	} else {
		for (i = 0; i<20; i++) {
			vga_ctrl->VRAM[i] = vga_ctrl->VRAM[i] | vga_ctrl->currBlock[i];
		}
		rotnum = oldrot;
	}

	/*
	for (i = 0; i<4; i++) {
		printf("%x ", vga_ctrl->currBlock[i]);
		printf("%x\n", vga_ctrl->rotBlock[i]);
	}
	*/

	/*
	printf("VRAM Curr Block Rot Block\n");
	for (i = 0; i<20; i++) {
		printf("%x %x %x\n", vga_ctrl->VRAM[i], vga_ctrl->currBlock[i], vga_ctrl->rotBlock[i]);
	}
	*/

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
	bool full = false;
	int i;
	for (i = 19; i>=0; i--) {
		if (vga_ctrl->VRAM[i]==0x3FF) {
			full = true;
			break;
		}
	}
	if (full) removeLine();
	gen_block();
}

void removeLine() {
	int i;
	int removed = 0;
	for (i = 19; i>=0; i--) {
		while (vga_ctrl->VRAM[i]==0x3FF) {
			helperRemove(i);
			removed++;
		}
	}
	switch (removed) {
	case 1:
		vga_ctrl->score = vga_ctrl->score + 40;
		break;
	case 2:
		vga_ctrl->score = vga_ctrl->score + 100;
		break;
	case 3:
		vga_ctrl->score = vga_ctrl->score + 300;
		break;
	case 4:
		vga_ctrl->score = vga_ctrl->score + 1200;
		break;
	default:
		break;
	}
	updateScore();
}

void helperRemove(int row) {
	int i;
	vga_ctrl->VRAM[row] = 0;
	vga_ctrl->colors[row] = 0;
	for (i = row; i>0; i--) {
		vga_ctrl->colors[i] = vga_ctrl->colors[i-1];
		vga_ctrl->VRAM[i] = vga_ctrl->VRAM[i-1];
	}
}

void moveLeft() {
	int i;
	bool moveable = true;
	for (i = 0; i<20; i++) {
		vga_ctrl->VRAM[i] = vga_ctrl->VRAM[i] ^ vga_ctrl->currBlock[i];
	}

	for (i = 0; i<20; i++) {
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
		for (i = 0; i<20; i++) {
			vga_ctrl->currBlock[i] = vga_ctrl->currBlock[i] << 1;
			vga_ctrl->VRAM[i] = vga_ctrl->VRAM[i] | vga_ctrl->currBlock[i];
		}
		updateColorBlock();
		updateColor();
		colnum--;
	} else {
		for (i = 0; i<20; i++) {
			vga_ctrl->VRAM[i] = vga_ctrl->VRAM[i] | vga_ctrl->currBlock[i];
		}
	}
	/*
	for (i = 0; i<4; i++) {
		printf("%x", vga_ctrl->VRAM[i]);
		printf("%x\n", vga_ctrl->currBlock[i]);
	}
	*/

}

void moveRight() {
	int i;
	bool moveable = true;
	for (i = 0; i<20; i++) {
		vga_ctrl->VRAM[i] = vga_ctrl->VRAM[i] ^ vga_ctrl->currBlock[i];
	}

	for (i = 0; i<20; i++) {
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
		for (i = 0; i<20; i++) {
			vga_ctrl->currBlock[i] = vga_ctrl->currBlock[i] >> 1;
			vga_ctrl->VRAM[i] = vga_ctrl->VRAM[i] | vga_ctrl->currBlock[i];
		}
		updateColorBlock();
		updateColor();
		colnum++;
	} else {
		for (i = 0; i<20; i++) {
			vga_ctrl->VRAM[i] = vga_ctrl->VRAM[i] | vga_ctrl->currBlock[i];
		}
	}
}

void moveDown() {
	int i;
	bool moveable = true;
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

	// printf("%d\n\n", moveable);

	for (i = 0; i<4; i++) {
		int line = vga_ctrl->currBlock[botLine];
		int tmp = vga_ctrl->VRAM[botLine+1] & line;
		// printf("Row val %d %x\n", botLine, vga_ctrl->VRAM[botLine+1]);
		botLine--;
		if (botLine<0) {
			break;
		}
		if (tmp) {
			moveable = false;
			break;
		}
	}

	// printf("%d\n\n", moveable);

	if (moveable) {
		for (i = 19; i>=0; i--) {
			if (i==0) vga_ctrl->currBlock[i] = 0;
			else vga_ctrl->currBlock[i] = vga_ctrl->currBlock[i-1];
			vga_ctrl->VRAM[i] = vga_ctrl->VRAM[i] | vga_ctrl->currBlock[i];
		}
		updateColorBlock();
		updateColor();
	} else {
		for (i = 0; i<20; i++) {
			vga_ctrl->VRAM[i] = vga_ctrl->VRAM[i] | vga_ctrl->currBlock[i];
		}
		checkFull();
	}

	/*
	printf("VRAM Curr Block\n");
	for (i = 0; i<20; i++) {
		printf("%x %x\n", vga_ctrl->VRAM[i], vga_ctrl->currBlock[i]);
	}
	*/
}

void updateColorBlock() {
	int i;
	for (i = 0; i<20; i++) {
		vga_ctrl->colors[i] = vga_ctrl->colors[i] ^ vga_ctrl->color_block[i];
	}
	for (i=0; i<20; i++) {
	vga_ctrl->color_block[i] = 0;
		if (vga_ctrl->currBlock[i]) {
			int j;
			int val = currPiece;
			int curr = vga_ctrl->currBlock[i];
			for (j = 0; j<10; j++) {
				if (curr%2) {
					vga_ctrl->color_block[i] = vga_ctrl->color_block[i] | val;
				}
				curr = curr >> 1;
				val = val << 3;
			}
		}
	}

}

void updateColor() {
	int i;
	for (i = 0; i<20; i++) {
		vga_ctrl->colors[i] = vga_ctrl->colors[i] | vga_ctrl->color_block[i];
	}

}

void startGame() {
	int i;
	for (i = 0; i<20; i++) {
		vga_ctrl->colors[i] = 0x000;
		vga_ctrl->VRAM[i] = 0x000;
	}
	vga_ctrl->score0 = '0';
	vga_ctrl->score1 = '0';
	vga_ctrl->score2 = '0';
	vga_ctrl->score3 = '0';
	vga_ctrl->score4 = '0';
	vga_ctrl->score5 = '0';
	vga_ctrl->over = '0';
	vga_ctrl->score = 0;
	gen_block();
}

void updateScore() {
	int i;
	int val = vga_ctrl->score;
	int num;
	for (i = 0; i<6; i++) {
		num = val % 10;
		switch (i) {
		case 0:
			vga_ctrl->score5 = num+'0';
			break;
		case 1:
			vga_ctrl->score4 = num+'0';
			break;
		case 2:
			vga_ctrl->score3 = num+'0';
			break;
		case 3:
			vga_ctrl->score2 = num+'0';
			break;
		case 4:
			vga_ctrl->score1 = num+'0';
			break;
		case 5:
			vga_ctrl->score0 = num+'0';
			break;
		default:
			break;
		}
		val = val/10;
	}
}

void endGame() {

}
