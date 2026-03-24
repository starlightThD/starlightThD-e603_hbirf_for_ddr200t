/*
 *  Copyright (c) 2018-2025 Nuclei System Technology, Inc.       
 *  All rights reserved.                                                
 */
`include "global.v"

module simple_gpio(
  input               clk,
  input               rst_n,
  
  // ICB总线接口
  input               i_icb_cmd_valid,
  output              i_icb_cmd_ready,
  input  [31:0]       i_icb_cmd_addr,
  input               i_icb_cmd_read,
  input  [31:0]       i_icb_cmd_wdata,
  
  output              i_icb_rsp_valid,
  input               i_icb_rsp_ready,
  output [31:0]       i_icb_rsp_rdata,
  
  // GPIO端口
  input  [7:0]        sw_i,      // 8位开关输入
  output [7:0]        led_o      // 8位LED输出
);

  // 寄存器定义
  reg [7:0]           led_reg;    // LED输出寄存器
  
  // 地址解码
  wire                addr_is_led = (i_icb_cmd_addr[11:2] == 10'h0);    // 0x10010000
  wire                addr_is_sw  = (i_icb_cmd_addr[11:2] == 10'h1);    // 0x10010004
  
  // 简单的ICB接口实现 - 总是准备好接收命令
  assign i_icb_cmd_ready = 1'b1;
  
  // 响应总是有效（组合逻辑）
  reg i_icb_rsp_valid_reg;
  reg [31:0] i_icb_rsp_rdata_reg;
  
  assign i_icb_rsp_valid = i_icb_rsp_valid_reg;
  assign i_icb_rsp_rdata = i_icb_rsp_rdata_reg;
  
  // LED输出
  assign led_o = led_reg;
  
  // 命令处理
  always @(posedge clk or negedge rst_n) begin
    if (~rst_n) begin
      led_reg <= 8'h00;
      i_icb_rsp_valid_reg <= 1'b0;
      i_icb_rsp_rdata_reg <= 32'h0;
    end else begin
      // 默认值
      i_icb_rsp_valid_reg <= 1'b0;
      
      // 当有命令到达时处理
      if (i_icb_cmd_valid) begin
        // 立即产生响应
        i_icb_rsp_valid_reg <= 1'b1;
        
        if (i_icb_cmd_read) begin
          // 读操作
          if (addr_is_led) begin
            i_icb_rsp_rdata_reg <= {24'h0, led_reg};  // 读LED寄存器
          end else if (addr_is_sw) begin
            // 读SW输入 - 返回实际的开关状态
            i_icb_rsp_rdata_reg <= {24'h0, sw_i};     // 读取SW输入状态
          end else begin
            i_icb_rsp_rdata_reg <= 32'h0;             // 未定义地址返回0
          end
        end else begin
          // 写操作
          if (addr_is_led) begin
            led_reg <= i_icb_cmd_wdata[7:0];  // 只使用低8位
          end
          // 写SW地址是只读的，忽略写操作
          i_icb_rsp_rdata_reg <= 32'h0;       // 写操作响应数据为0
        end
      end
      
      // 如果响应被接受，清除有效标志
      if (i_icb_rsp_valid_reg && i_icb_rsp_ready) begin
        i_icb_rsp_valid_reg <= 1'b0;
      end
    end
  end
  
  // 错误处理（无错误）
  // assign i_icb_rsp_err = 1'b0;

endmodule