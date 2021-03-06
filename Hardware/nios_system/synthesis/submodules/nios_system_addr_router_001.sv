// (C) 2001-2012 Altera Corporation. All rights reserved.
// Your use of Altera Corporation's design tools, logic functions and other 
// software and tools, and its AMPP partner logic functions, and any output 
// files any of the foregoing (including device programming or simulation 
// files), and any associated documentation or information are expressly subject 
// to the terms and conditions of the Altera Program License Subscription 
// Agreement, Altera MegaCore Function License Agreement, or other applicable 
// license agreement, including, without limitation, that your use is for the 
// sole purpose of programming logic devices manufactured by Altera and sold by 
// Altera or its authorized distributors.  Please refer to the applicable 
// agreement for further details.


// $Id: //acds/rel/12.0sp2/ip/merlin/altera_merlin_router/altera_merlin_router.sv.terp#1 $
// $Revision: #1 $
// $Date: 2012/06/21 $
// $Author: swbranch $

// -------------------------------------------------------
// Merlin Router
//
// Asserts the appropriate one-hot encoded channel based on 
// either (a) the address or (b) the dest id. The DECODER_TYPE
// parameter controls this behaviour. 0 means address decoder,
// 1 means dest id decoder.
//
// In the case of (a), it also sets the destination id.
// -------------------------------------------------------

`timescale 1 ns / 1 ns

module nios_system_addr_router_001_default_decode
  #(
     parameter DEFAULT_CHANNEL = 2,
               DEFAULT_DESTID = 2 
   )
  (output [96 - 92 : 0] default_destination_id,
   output [17-1 : 0] default_src_channel
  );

  assign default_destination_id = 
    DEFAULT_DESTID[96 - 92 : 0];
  generate begin : default_decode
    if (DEFAULT_CHANNEL == -1)
      assign default_src_channel = '0;
    else
      assign default_src_channel = 17'b1 << DEFAULT_CHANNEL;
  end endgenerate

endmodule


module nios_system_addr_router_001
(
    // -------------------
    // Clock & Reset
    // -------------------
    input clk,
    input reset,

    // -------------------
    // Command Sink (Input)
    // -------------------
    input                       sink_valid,
    input  [107-1 : 0]    sink_data,
    input                       sink_startofpacket,
    input                       sink_endofpacket,
    output                      sink_ready,

    // -------------------
    // Command Source (Output)
    // -------------------
    output                          src_valid,
    output reg [107-1    : 0] src_data,
    output reg [17-1 : 0] src_channel,
    output                          src_startofpacket,
    output                          src_endofpacket,
    input                           src_ready
);

    // -------------------------------------------------------
    // Local parameters and variables
    // -------------------------------------------------------
    localparam PKT_ADDR_H = 67;
    localparam PKT_ADDR_L = 36;
    localparam PKT_DEST_ID_H = 96;
    localparam PKT_DEST_ID_L = 92;
    localparam ST_DATA_W = 107;
    localparam ST_CHANNEL_W = 17;
    localparam DECODER_TYPE = 0;

    localparam PKT_TRANS_WRITE = 70;
    localparam PKT_TRANS_READ  = 71;

    localparam PKT_ADDR_W = PKT_ADDR_H-PKT_ADDR_L + 1;
    localparam PKT_DEST_ID_W = PKT_DEST_ID_H-PKT_DEST_ID_L + 1;




    // -------------------------------------------------------
    // Figure out the number of bits to mask off for each slave span
    // during address decoding
    // -------------------------------------------------------
    localparam PAD0 = log2ceil(32'h1000 - 32'h0);
    localparam PAD1 = log2ceil(32'h2000 - 32'h1800);
    localparam PAD2 = log2ceil(32'h2010 - 32'h2000);
    localparam PAD3 = log2ceil(32'h2020 - 32'h2010);
    localparam PAD4 = log2ceil(32'h2030 - 32'h2020);
    localparam PAD5 = log2ceil(32'h2040 - 32'h2030);
    localparam PAD6 = log2ceil(32'h2060 - 32'h2040);
    localparam PAD7 = log2ceil(32'h2080 - 32'h2060);
    localparam PAD8 = log2ceil(32'h20a0 - 32'h2080);
    localparam PAD9 = log2ceil(32'h20a8 - 32'h20a0);
    localparam PAD10 = log2ceil(32'h20b0 - 32'h20a8);
    localparam PAD11 = log2ceil(32'h20b8 - 32'h20b0);
    localparam PAD12 = log2ceil(32'h20ba - 32'h20b8);
    localparam PAD13 = log2ceil(32'h2800 - 32'h2400);
    localparam PAD14 = log2ceil(32'h6000 - 32'h4000);
    localparam PAD15 = log2ceil(32'h100000 - 32'h80000);
    localparam PAD16 = log2ceil(32'h1800000 - 32'h1000000);

    // -------------------------------------------------------
    // Work out which address bits are significant based on the
    // address range of the slaves. If the required width is too
    // large or too small, we use the address field width instead.
    // -------------------------------------------------------
    localparam ADDR_RANGE = 32'h1800000;
    localparam RANGE_ADDR_WIDTH = log2ceil(ADDR_RANGE);
    localparam OPTIMIZED_ADDR_H = (RANGE_ADDR_WIDTH > PKT_ADDR_W) ||
                                  (RANGE_ADDR_WIDTH == 0) ?
                                        PKT_ADDR_H :
                                        PKT_ADDR_L + RANGE_ADDR_WIDTH - 1;
    localparam RG = RANGE_ADDR_WIDTH-1;

      wire [PKT_ADDR_W-1 : 0] address = sink_data[OPTIMIZED_ADDR_H : PKT_ADDR_L];

    // -------------------------------------------------------
    // Pass almost everything through, untouched
    // -------------------------------------------------------
    assign sink_ready        = src_ready;
    assign src_valid         = sink_valid;
    assign src_startofpacket = sink_startofpacket;
    assign src_endofpacket   = sink_endofpacket;

    wire [PKT_DEST_ID_W-1:0] default_destid;
    wire [17-1 : 0] default_src_channel;




    nios_system_addr_router_001_default_decode the_default_decode(
      .default_destination_id (default_destid),
      .default_src_channel (default_src_channel)
    );

    always @* begin
        src_data    = sink_data;
        src_channel = default_src_channel;

        src_data[PKT_DEST_ID_H:PKT_DEST_ID_L] = default_destid;
        // --------------------------------------------------
        // Address Decoder
        // Sets the channel and destination ID based on the address
        // --------------------------------------------------

        // ( 0x0 .. 0x1000 )
        if ( {address[RG:PAD0],{PAD0{1'b0}}} == 'h0 ) begin
            src_channel = 17'b00000000000000010;
            src_data[PKT_DEST_ID_H:PKT_DEST_ID_L] = 1;
        end

        // ( 0x1800 .. 0x2000 )
        if ( {address[RG:PAD1],{PAD1{1'b0}}} == 'h1800 ) begin
            src_channel = 17'b00000000000000001;
            src_data[PKT_DEST_ID_H:PKT_DEST_ID_L] = 0;
        end

        // ( 0x2000 .. 0x2010 )
        if ( {address[RG:PAD2],{PAD2{1'b0}}} == 'h2000 ) begin
            src_channel = 17'b00000000000001000;
            src_data[PKT_DEST_ID_H:PKT_DEST_ID_L] = 3;
        end

        // ( 0x2010 .. 0x2020 )
        if ( {address[RG:PAD3],{PAD3{1'b0}}} == 'h2010 ) begin
            src_channel = 17'b00000000000100000;
            src_data[PKT_DEST_ID_H:PKT_DEST_ID_L] = 5;
        end

        // ( 0x2020 .. 0x2030 )
        if ( {address[RG:PAD4],{PAD4{1'b0}}} == 'h2020 ) begin
            src_channel = 17'b00000000010000000;
            src_data[PKT_DEST_ID_H:PKT_DEST_ID_L] = 7;
        end

        // ( 0x2030 .. 0x2040 )
        if ( {address[RG:PAD5],{PAD5{1'b0}}} == 'h2030 ) begin
            src_channel = 17'b00010000000000000;
            src_data[PKT_DEST_ID_H:PKT_DEST_ID_L] = 13;
        end

        // ( 0x2040 .. 0x2060 )
        if ( {address[RG:PAD6],{PAD6{1'b0}}} == 'h2040 ) begin
            src_channel = 17'b00000000100000000;
            src_data[PKT_DEST_ID_H:PKT_DEST_ID_L] = 8;
        end

        // ( 0x2060 .. 0x2080 )
        if ( {address[RG:PAD7],{PAD7{1'b0}}} == 'h2060 ) begin
            src_channel = 17'b00000001000000000;
            src_data[PKT_DEST_ID_H:PKT_DEST_ID_L] = 9;
        end

        // ( 0x2080 .. 0x20a0 )
        if ( {address[RG:PAD8],{PAD8{1'b0}}} == 'h2080 ) begin
            src_channel = 17'b00000010000000000;
            src_data[PKT_DEST_ID_H:PKT_DEST_ID_L] = 10;
        end

        // ( 0x20a0 .. 0x20a8 )
        if ( {address[RG:PAD9],{PAD9{1'b0}}} == 'h20a0 ) begin
            src_channel = 17'b00000000000010000;
            src_data[PKT_DEST_ID_H:PKT_DEST_ID_L] = 4;
        end

        // ( 0x20a8 .. 0x20b0 )
        if ( {address[RG:PAD10],{PAD10{1'b0}}} == 'h20a8 ) begin
            src_channel = 17'b00100000000000000;
            src_data[PKT_DEST_ID_H:PKT_DEST_ID_L] = 14;
        end

        // ( 0x20b0 .. 0x20b8 )
        if ( {address[RG:PAD11],{PAD11{1'b0}}} == 'h20b0 ) begin
            src_channel = 17'b10000000000000000;
            src_data[PKT_DEST_ID_H:PKT_DEST_ID_L] = 16;
        end

        // ( 0x20b8 .. 0x20ba )
        if ( {address[RG:PAD12],{PAD12{1'b0}}} == 'h20b8 ) begin
            src_channel = 17'b00000000001000000;
            src_data[PKT_DEST_ID_H:PKT_DEST_ID_L] = 6;
        end

        // ( 0x2400 .. 0x2800 )
        if ( {address[RG:PAD13],{PAD13{1'b0}}} == 'h2400 ) begin
            src_channel = 17'b00000100000000000;
            src_data[PKT_DEST_ID_H:PKT_DEST_ID_L] = 11;
        end

        // ( 0x4000 .. 0x6000 )
        if ( {address[RG:PAD14],{PAD14{1'b0}}} == 'h4000 ) begin
            src_channel = 17'b01000000000000000;
            src_data[PKT_DEST_ID_H:PKT_DEST_ID_L] = 15;
        end

        // ( 0x80000 .. 0x100000 )
        if ( {address[RG:PAD15],{PAD15{1'b0}}} == 'h80000 ) begin
            src_channel = 17'b00001000000000000;
            src_data[PKT_DEST_ID_H:PKT_DEST_ID_L] = 12;
        end

        // ( 0x1000000 .. 0x1800000 )
        if ( {address[RG:PAD16],{PAD16{1'b0}}} == 'h1000000 ) begin
            src_channel = 17'b00000000000000100;
            src_data[PKT_DEST_ID_H:PKT_DEST_ID_L] = 2;
        end
    end

    // --------------------------------------------------
    // Ceil(log2()) function
    // --------------------------------------------------
    function integer log2ceil;
        input reg[63:0] val;
        reg [63:0] i;

        begin
            i = 1;
            log2ceil = 0;

            while (i < val) begin
                log2ceil = log2ceil + 1;
                i = i << 1;
            end
        end
    endfunction

endmodule


