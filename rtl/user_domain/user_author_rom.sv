`include "common_cells/registers.svh"

module user_author_rom #(
  parameter obi_pkg::obi_cfg_t           ObiCfg      = obi_pkg::ObiDefaultConfig,
  parameter type                         obi_req_t   = logic,
  parameter type                         obi_rsp_t   = logic
) (
  input  logic clk_i,
  input  logic rst_ni,
  input  obi_req_t obi_req_i,
  output obi_rsp_t obi_rsp_o
);

  logic req_q, req_d;
  `FF(req_q, req_d, '0, clk_i, rst_ni);

  logic we_q, we_d;
  `FF(we_q, we_d, '0, clk_i, rst_ni);

  logic [ObiCfg.IdWidth-1:0] id_q, id_d; // Id of the request, must be same for the response
  `FF(id_q, id_d, '0, clk_i, rst_ni);

  logic [ObiCfg.AddrWidth-1:0] addr_q, addr_d; // Internal address of the word to read
  `FF(addr_q, addr_d, '0, clk_i, rst_ni);

  // Signals used to create the response
  logic [ObiCfg.DataWidth-1:0] rsp_data; // Data field of the obi response
  logic rsp_err; // Error field of the obi response

  // Wire the registers holding the request
  assign req_d = obi_req_i.req;
  assign id_d = obi_req_i.a.aid;
  assign we_d = obi_req_i.a.we;
  assign addr_d = obi_req_i.a.addr;

  // Assign the response data
  logic [2:0] word_addr;
  always_comb begin
    rsp_data = '0;
    rsp_err  = '0;
    word_addr = addr_q[4:2];

    if(req_q) begin
      if(~we_q) begin
        case(word_addr)
          3'h0: rsp_data = 32'h616b_694d;
          3'h1: rsp_data = 32'h4720_6c69;
          3'h2: rsp_data = 32'h6b69_6465;
          3'h3: rsp_data = 32'h5420_2620;
          3'h4: rsp_data = 32'h616d_6f68;
          3'h5: rsp_data = 32'h6548_2073;
          3'h6: rsp_data = 32'h676f_7a72;
          3'h7: rsp_data = 32'h0000_0000;
          default: rsp_data = '0;
        endcase
      end else begin
        rsp_err = '1;
      end
    end
  end

  // Wire the response
  // A channel
  assign obi_rsp_o.gnt = obi_req_i.req;
  // R channel:
  assign obi_rsp_o.rvalid = req_q;
  assign obi_rsp_o.r.rdata = rsp_data;
  assign obi_rsp_o.r.rid = id_q;
  assign obi_rsp_o.r.err = rsp_err;
  assign obi_rsp_o.r.r_optional = '0;

endmodule