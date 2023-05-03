module end_palette (
	input logic [3:0] index,
	output logic [3:0] red, green, blue
);

localparam [0:15][11:0] palette = {
	{4'hA, 4'h9, 4'h9},
	{4'h0, 4'h0, 4'h0},
	{4'hF, 4'hF, 4'hF},
	{4'h9, 4'h9, 4'hA},
	{4'hB, 4'hB, 4'hA},
	{4'h6, 4'h6, 4'h6},
	{4'h9, 4'hA, 4'h9},
	{4'h9, 4'h9, 4'hA},
	{4'h9, 4'hA, 4'hB},
	{4'h2, 4'h2, 4'h2},
	{4'hA, 4'hB, 4'hA},
	{4'hA, 4'h9, 4'hA},
	{4'hB, 4'h9, 4'h9},
	{4'h9, 4'h9, 4'hA},
	{4'hA, 4'hA, 4'h9},
	{4'hE, 4'hE, 4'hE}
};

assign {red, green, blue} = palette[index];

endmodule
