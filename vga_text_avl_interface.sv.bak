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
logic [9:0] DrawX, DrawY;
logic [10:0] sprite_addr;
logic [7:0] sprite_data;
logic [6:0] col, row, char;
logic [11:0] pos; 
logic [11:0] vram_addr;
logic [2:0] pixel, fgd_n, bgd_n;
logic [31:0] PALETTE_REG [8];
logic [31:0] PALETTE_READ_DATA, READ_DATA, RAM_OUT;
logic [3:0] fgd_idx, bgd_idx;
logic [3:0] fgd_r, fgd_g, fgd_b, bgd_r, bgd_g, bgd_b;

//Declare submodules..e.g. VGA controller, ROMS, etc
	vga_controller vga (.Clk(CLK), .Reset(RESET), .*);
	font_rom font (.addr(sprite_addr), .data(sprite_data));
	ram ram0 (.address_a(AVL_ADDR), .address_b(vram_addr), 
			.byteena_a(AVL_BYTE_EN), .clock(CLK),
			.data_a(AVL_WRITEDATA), .data_b(),
			.rden_a(AVL_CS & AVL_READ & ~AVL_ADDR[11]), .rden_b(1'b1),
			.wren_a(AVL_CS & AVL_WRITE & ~AVL_ADDR[11]), .wren_b(1'b0), 
			.q_a(READ_DATA), .q_b(RAM_OUT));
   
// Read and write from AVL interface to register block, note that READ waitstate = 1, so this should be in always_ff
always_ff @(posedge CLK) begin
	if (RESET)
	begin
		integer i;
		for (i = 0; i < 8; i = i + 1) begin
			PALETTE_REG[i] <= 32'h000;
		end
	end
	else if (AVL_CS)
	begin
		if (AVL_WRITE & AVL_ADDR[11])
		begin
			case (AVL_BYTE_EN)
				4'b1111:
					PALETTE_REG[AVL_ADDR[2:0]] <= AVL_WRITEDATA;
				4'b1100:
					PALETTE_REG[AVL_ADDR[2:0]][31:16] <= AVL_WRITEDATA[31:16];
				4'b0011:
					PALETTE_REG[AVL_ADDR[2:0]][15:0] <= AVL_WRITEDATA[15:0];
				4'b1000:
					PALETTE_REG[AVL_ADDR[2:0]][31:24] <= AVL_WRITEDATA[31:24];
				4'b0100:
					PALETTE_REG[AVL_ADDR[2:0]][23:16] <= AVL_WRITEDATA[23:16];
				4'b0010:
					PALETTE_REG[AVL_ADDR[2:0]][15:8] <= AVL_WRITEDATA[15:8];
				4'b0001:
					PALETTE_REG[AVL_ADDR[2:0]][7:0] <= AVL_WRITEDATA[7:0];
				default:
					PALETTE_REG[AVL_ADDR[2:0]] <= PALETTE_REG[AVL_ADDR[2:0]];
			endcase
		end
		else if (AVL_READ & AVL_ADDR[11])
			PALETTE_READ_DATA <= PALETTE_REG[AVL_ADDR[2:0]];
	end
end

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
	col = DrawX/8;
	row = DrawY/16;
	pos = (row*80) + col;
	vram_addr = pos/2;
	case (col[0])
		1'b0:
		begin
			char = RAM_OUT[14:8];
			invert = RAM_OUT[15];
			fgd_idx  = RAM_OUT[7:4];
			bgd_idx = RAM_OUT[3:0];
		end
		1'b1:
		begin
			char = RAM_OUT[30:24];
			invert = RAM_OUT[31];
			fgd_idx = RAM_OUT[23:20];
			bgd_idx = RAM_OUT[19:16];
		end
	endcase
	sprite_addr = {char, DrawY[3:0]};
end

assign pixel = sprite_data[7-DrawX[2:0]];

always_comb
begin
	fgd_iv = fgd_idx[0];
	bgd_iv = bgd_idx[0];
	fgd_n = fgd_idx >> 1;
	bgd_n = bgd_idx >> 1;
	
	if (fgd_iv) 
	begin
		fgd_r = PALETTE_REG[fgd_n][24:21];
		fgd_g = PALETTE_REG[fgd_n][20:17];
		fgd_b = PALETTE_REG[fgd_n][16:13];
	end
	else 
	begin
		fgd_r = PALETTE_REG[fgd_n][12:9];
		fgd_g = PALETTE_REG[fgd_n][8:5];
		fgd_b = PALETTE_REG[fgd_n][4:1];
	end
	
	if (bgd_iv) 
	begin
		bgd_r = PALETTE_REG[bgd_n][24:21];
		bgd_g = PALETTE_REG[bgd_n][20:17];
		bgd_b = PALETTE_REG[bgd_n][16:13];
	end
	else 
	begin
		bgd_r = PALETTE_REG[bgd_n][12:9];
		bgd_g = PALETTE_REG[bgd_n][8:5];
		bgd_b = PALETTE_REG[bgd_n][4:1];
	end
end

always_ff @(posedge pixel_clk)
begin
	if (blank)
	begin
		if (invert ^ pixel)
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
	end
	else
	begin
		red <= 4'h0;
		green <= 4'h0;
		blue <= 4'h0;
	end
end

endmodule	
