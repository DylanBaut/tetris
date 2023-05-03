/************************************************************************
Avalon-MM Interface VGA Text mode display

Register Map:
0x000-0x0257 : VRAM, 80x30 (2400 byte, 600 word) raster order (first column then row)
0x258        : control register

VRAM Format:
X->
[ 31  30-24][ 23  22-16][ 15  14-8 ][ 7    6-0 ]
[IV3][CODE3][IV2][CODE2][IV1][CODE1][IV0][CODE0]

IVn = Draw inverse glyph
CODEn = Glyph code from IBM codepage 437

Control Register Format:
[[31-25][24-21][20-17][16-13][ 12-9][ 8-5 ][ 4-1 ][   0    ] 
[[RSVD ][FGD_R][FGD_G][FGD_B][BKG_R][BKG_G][BKG_B][RESERVED]

VSYNC signal = bit which flips on every Vsync (time for new frame), used to synchronize software
BKG_R/G/B = Background color, flipped with foreground when IVn bit is set
FGD_R/G/B = Foreground color, flipped with background when Inv bit is set

************************************************************************/
//`define NUM_REGS 601 //80*30 characters / 4 characters per register
//`define CTRL_REG 600 //index of control register

module vga_text_avl_interface (
	// Avalon Clock Input, note this clock is also used for VGA, so this must be 50Mhz
	// We can put a clock divider here in the future to make this IP more generalizable
	input logic CLK,
	
	// Avalon Reset Input
	input logic RESET,
	
	// Avalon-MM Slave Signals
	input  logic AVL_READ,					// Avalon-MM Read
	input  logic AVL_WRITE,					// Avalon-MM Write
	input  logic AVL_CS,					// Avalon-MM Chip Select
	input  logic [3:0] AVL_BYTE_EN,			// Avalon-MM Byte Enable
	input  logic [11:0] AVL_ADDR,			// Avalon-MM Address
	input  logic [31:0] AVL_WRITEDATA,		// Avalon-MM Write Data
	output logic [31:0] AVL_READDATA,		// Avalon-MM Read Data
	
	// Exported Conduit (mapped to VGA port - make sure you export in Platform Designer)
	output logic [3:0]  red, green, blue,	// VGA color channels (mapped to output pins in top-level)
	output logic hs, vs						// VGA HS/VS
);

//logic [31:0] LOCAL_REG       [`NUM_REGS]; // Registers
//put other local variables here
logic pixel_clk, blank, sync, invert, fgd_iv, bgd_iv, wren;
logic [2:0] idx;
logic [9:0] DrawX, DrawY;
logic [10:0] sprite_addr;
logic block;
logic [7:0] sprite_data;
logic [6:0] col, row, char;
logic [11:0] pos; 
logic [11:0] vram_addr;
logic [2:0] pixel, fgd_n, bgd_n;
logic [31:0] PALETTE_REG [7];
logic [31:0] block_colors;
logic [31:0] PALETTE_READ_DATA, READ_DATA, RAM_OUT;
logic [3:0] fgd_idx, bgd_idx;
logic [3:0] fgd_r, fgd_g, fgd_b, bgd_r, bgd_g, bgd_b;

//Declare submodules..e.g. VGA controller, ROMS, etc
	vga_controller vga (.Clk(CLK), .Reset(RESET), .*);
	font_rom font (.addr(sprite_addr), .data(sprite_data));
	ram ram0 (.address_a(AVL_ADDR), .address_b(vram_addr), 
			.byteena_a(AVL_BYTE_EN), .clock(CLK),
			.data_a(AVL_WRITEDATA), .data_b(),
			.rden_a((AVL_CS & AVL_READ & ~AVL_ADDR[11])), .rden_b(1'b1),
			.wren_a(AVL_CS & AVL_WRITE & ~AVL_ADDR[11]), .wren_b(1'b0), 
			.q_a(READ_DATA), .q_b(RAM_OUT));
			
assign PALETTE_REG[0] = 32'h055f;
assign PALETTE_REG[1] = 32'h000a;
assign PALETTE_REG[2] = 32'h0a50;
assign PALETTE_REG[3] = 32'h0ff5;
assign PALETTE_REG[4] = 32'h05f5;
assign PALETTE_REG[5] = 32'h0a0a;
assign PALETTE_REG[6] = 32'h0a00;

always_comb
begin
	if(AVL_READ & ~AVL_ADDR[11] & AVL_CS)
			AVL_READDATA = READ_DATA;
	else if (AVL_READ & AVL_ADDR[11] & AVL_CS)
			AVL_READDATA = PALETTE_READ_DATA;
	else
			AVL_READDATA = 32'hX;
end


//handle drawing (may either be combinational or sequential - or both).
always_comb
begin
	if (DrawX<=440 && DrawX>=200)
	begin
		col = (DrawX-200)/24;
		row = DrawY/24;
		vram_addr = row;
		block = RAM_OUT[9-col];
		sprite_addr = 0;
		char = 0;
		idx = (block_colors >> 3*(9-col)) & 4'b0111;
	end
	else if (DrawX==199)
	begin
		col = 0;
		row = DrawY/24;
		vram_addr = row + 40;
		block = 0;
		sprite_addr = 0;
		char = 0;
		idx = 0;
	end
	else
	begin
		if (DrawY<=15)
		begin
			if (DrawX>=0 && DrawX<=7)
			begin
				vram_addr = 102;
				char = RAM_OUT;
			end
			else if (DrawX>=8 && DrawX<=15)
			begin
				vram_addr = 102;
				char = RAM_OUT >> 8;
			end
			else if (DrawX>=16 && DrawX<=23)
			begin
				vram_addr = 102;
				char = RAM_OUT >> 16;
			end
			else if (DrawX>=24 && DrawX<=31)
			begin
				vram_addr = 102;
				char = RAM_OUT >> 24;
			end
			else if (DrawX>=32 && DrawX<=39)
			begin
				vram_addr = 103;
				char = RAM_OUT;
			end
			else if (DrawX>=40 && DrawX<=47)
			begin
				vram_addr = 103;
				char = RAM_OUT >> 8;
			end
			else
			begin
				vram_addr = 104;
				char = 0;
			end
		end
		else
		begin
			vram_addr = 0;
			char = 0;
		end
		col = 0;
		row = 0;
		block = 0;
		idx = 0;
		sprite_addr = {char, DrawY[3:0]};
	end
end

assign pixel = sprite_data[7-DrawX[2:0]];

always_ff @(posedge pixel_clk)
begin
	if (DrawX==199)
		block_colors <= RAM_OUT;
end

always_ff @(posedge pixel_clk)
begin
	if (blank)
	begin
		if (DrawX>464)
		begin
			red <= 4'h0;
			green <= 4'h0;
			blue <= 4'h0;
		end
		else if (DrawX<176 && DrawY<=15)
		begin
			if (pixel)
			begin
				red <= 4'hf;
				green <= 4'hf;
				blue <= 4'hf;
			end
			else
			begin
				red <= 4'h0;
				green <= 4'h0;
				blue <= 4'h0;
			end
		end
		else if ((DrawX>=176 && DrawX<200) || (DrawX>=440 && DrawX<=464)) 
		begin
			red <= 4'hf;
			green <= 4'hf;
			blue <= 4'hf;
		end
		else if (block)
		begin
			red <= PALETTE_REG[idx] >> 8;
			green <= (PALETTE_REG[idx] >> 4) & 4'hf;
			blue <= PALETTE_REG[idx] & 4'hf;
		end
		else 
		begin
			red <= 4'h0;
			green <= 4'h0;
			blue <= 4'h0;
		end
		/*
		else if (invert ^ pixel)
		begin
			red <= fgd_r;
			green <= fgd_g;
			blue <=  fgd_b;
		end
		else
		begin
			red <= bgd_r;
			green <= bgd_g;
			blue <= bgd_b;
		end
		*/
	end
	else
	begin
		red <= 4'h0;
		green <= 4'h0;
		blue <= 4'h0;
	end
end

endmodule	
