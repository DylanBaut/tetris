module bg1_1_rom (
	input logic clock,
	input logic [16:0] address,
	output logic [4:0] q
);

logic [4:0] memory [0:83999] /* synthesis ram_init_file = "./bg1_1/bg1_1.mif" */;

always_ff @ (posedge clock) begin
	q <= memory[address];
end

endmodule
