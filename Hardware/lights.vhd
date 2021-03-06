LIBRARY ieee;
USE ieee.std_logic_1164.all;
USE ieee.std_logic_arith.all;
USE ieee.std_logic_unsigned.all;

ENTITY lights IS
	PORT (
		SW : IN STD_LOGIC_VECTOR(7 DOWNTO 0);
		KEY : IN STD_LOGIC_VECTOR(3 DOWNTO 0);
		CLOCK_50 : IN STD_LOGIC;
		LEDR : OUT STD_LOGIC_VECTOR(7 DOWNTO 0);
		DRAM_CLK, DRAM_CKE : OUT STD_LOGIC;
		DRAM_ADDR : OUT STD_LOGIC_VECTOR(11 DOWNTO 0);
		DRAM_BA_0, DRAM_BA_1 : BUFFER STD_LOGIC;
		DRAM_CS_N, DRAM_CAS_N, DRAM_RAS_N, DRAM_WE_N : OUT STD_LOGIC;
		DRAM_DQ : INOUT STD_LOGIC_VECTOR(15 DOWNTO 0);
		DRAM_UDQM, DRAM_LDQM : BUFFER STD_LOGIC;
		LCD_DATA : INOUT STD_LOGIC_VECTOR(7 DOWNTO 0);
		LCD_ON, LCD_BLON, LCD_EN, LCD_RS, LCD_RW : OUT STD_LOGIC;
		SD_DAT, SD_DAT3, SD_CMD : INOUT STD_LOGIC;
		SD_CLK : OUT STD_LOGIC;
		SRAM_DQ : INOUT STD_LOGIC_VECTOR(15 DOWNTO 0);
		SRAM_ADDR : OUT STD_LOGIC_VECTOR(17 DOWNTO 0);
		SRAM_LB_N, SRAM_UB_N, SRAM_CE_N, SRAM_OE_N, SRAM_WE_N : OUT STD_LOGIC;
		VGA_CLK, VGA_BLANK, VGA_HS, VGA_VS, VGA_SYNC : OUT STD_LOGIC;
		VGA_R, VGA_G, VGA_B : OUT STD_LOGIC_VECTOR(9 downto 0);
		PS2_CLK, PS2_DAT : INOUT STD_LOGIC
	);
END lights;

ARCHITECTURE Structure OF lights IS
	
	COMPONENT nios_system
	PORT (
		clk_clk : IN STD_LOGIC;
		reset_reset_n : IN STD_LOGIC;
		leds_export : OUT STD_LOGIC_VECTOR(7 DOWNTO 0);
		switches_export : IN STD_LOGIC_VECTOR(7 DOWNTO 0);
		sdram_wire_addr : OUT STD_LOGIC_VECTOR(11 DOWNTO 0);
		sdram_wire_ba : BUFFER STD_LOGIC_VECTOR(1 DOWNTO 0);
		sdram_wire_cas_n : OUT STD_LOGIC;
		sdram_wire_cke : OUT STD_LOGIC;
		sdram_wire_cs_n : OUT STD_LOGIC;
		sdram_wire_dq : INOUT STD_LOGIC_VECTOR(15 DOWNTO 0);
		sdram_wire_dqm : BUFFER STD_LOGIC_VECTOR(1 DOWNTO 0);
		sdram_wire_ras_n : OUT STD_LOGIC;
		sdram_wire_we_n : OUT STD_LOGIC;
		sdram_clk_clk   : OUT STD_LOGIC;
		lcd_data_DATA    : INOUT std_logic_vector(7 downto 0)  := (others => 'X'); 
		lcd_data_ON      : OUT   std_logic;                                       
		lcd_data_BLON    : OUT   std_logic;                                        
		lcd_data_EN      : OUT   std_logic;                                        
		lcd_data_RS      : OUT   std_logic;                                        
		lcd_data_RW      : OUT   std_logic;
		keys_export      : IN    std_logic_vector(3 downto 0)  := (others => 'X');
		sdcard_interface_b_SD_cmd   : inout std_logic                     := 'X';             -- b_SD_cmd
		sdcard_interface_b_SD_dat   : inout std_logic                     := 'X';             -- b_SD_dat
		sdcard_interface_b_SD_dat3  : inout std_logic                     := 'X';             -- b_SD_dat3
		sdcard_interface_o_SD_clock : out   std_logic;                                        -- o_SD_clock
		pixel_buffer_DQ             : inout std_logic_vector(15 downto 0) := (others => 'X'); -- DQ
		pixel_buffer_ADDR           : out   std_logic_vector(17 downto 0);                    -- ADDR
		pixel_buffer_LB_N           : out   std_logic;                                        -- LB_N
		pixel_buffer_UB_N           : out   std_logic;                                        -- UB_N
		pixel_buffer_CE_N           : out   std_logic;                                        -- CE_N
		pixel_buffer_OE_N           : out   std_logic;                                        -- OE_N
		pixel_buffer_WE_N           : out   std_logic;                                        -- WE_N
		vga_controller_CLK          : out   std_logic;                                        -- CLK
		vga_controller_HS           : out   std_logic;                                        -- HS
		vga_controller_VS           : out   std_logic;                                        -- VS
		vga_controller_BLANK        : out   std_logic;                                        -- BLANK
		vga_controller_SYNC         : out   std_logic;                                        -- SYNC
		vga_controller_R            : out   std_logic_vector(9 downto 0);                     -- R
		vga_controller_G            : out   std_logic_vector(9 downto 0);                     -- G
		vga_controller_B            : out   std_logic_vector(9 downto 0);                     -- B
		ps2_CLK                     : inout std_logic                     := 'X';             -- CLK
      ps2_DAT                     : inout std_logic                     := 'X'              -- DAT
);

END COMPONENT;
	
	SIGNAL DQM : STD_LOGIC_VECTOR(1 DOWNTO 0);
	SIGNAL BA : STD_LOGIC_VECTOR(1 DOWNTO 0);
	
BEGIN
		DRAM_BA_0 <= BA(0);
		DRAM_BA_1 <= BA(1);
		DRAM_UDQM <= DQM(1);
		DRAM_LDQM <= DQM(0);
		
		NiosII: nios_system
		PORT MAP (
			clk_clk => CLOCK_50,
			reset_reset_n => KEY(0) or KEY(1),
			leds_export => LEDR,
			switches_export => SW,
			keys_export		=> KEY,
			sdram_wire_addr => DRAM_ADDR,
			sdram_wire_ba => BA,
			sdram_wire_cas_n => DRAM_CAS_N,
			sdram_clk_clk => DRAM_CLK,
			sdram_wire_cke => DRAM_CKE,
			sdram_wire_cs_n => DRAM_CS_N,
			sdram_wire_dq => DRAM_DQ,
			sdram_wire_dqm => DQM,
			sdram_wire_ras_n => DRAM_RAS_N,
			sdram_wire_we_n => DRAM_WE_N,
			lcd_data_DATA    => LCD_DATA,    
			lcd_data_ON      => LCD_ON,      
			lcd_data_BLON    => LCD_BLON,   
			lcd_data_EN      => LCD_EN,      
			lcd_data_RS      => LCD_RS,      
			lcd_data_RW      => LCD_RW,
			sdcard_interface_b_SD_cmd   => SD_CMD,
			sdcard_interface_b_SD_dat   => SD_DAT,
			sdcard_interface_b_SD_dat3  => SD_DAT3,
			sdcard_interface_o_SD_clock => SD_CLK,
			pixel_buffer_DQ             => SRAM_DQ,             --     pixel_buffer.DQ
			pixel_buffer_ADDR           => SRAM_ADDR,           --                 .ADDR
			pixel_buffer_LB_N           => SRAM_LB_N,           --                 .LB_N
			pixel_buffer_UB_N           => SRAM_UB_N,           --                 .UB_N
			pixel_buffer_CE_N           => SRAM_CE_N,           --                 .CE_N
			pixel_buffer_OE_N           => SRAM_OE_N,           --                 .OE_N
			pixel_buffer_WE_N           => SRAM_WE_N,           --                 .WE_N
			vga_controller_CLK          => VGA_CLK,          	 --   vga_controller.CLK
			vga_controller_HS           => VGA_HS,           	 --                 .HS
			vga_controller_VS           => VGA_VS,           	 --                 .VS
			vga_controller_BLANK        => VGA_BLANK,        	 --                 .BLANK
			vga_controller_SYNC         => VGA_SYNC,         	 --                 .SYNC
			vga_controller_R            => VGA_R,            	 --                 .R
			vga_controller_G            => VGA_G,            	 --                 .G
			vga_controller_B            => VGA_B, 					 --                 .B
			ps2_CLK                     => ps2_CLK,
			ps2_DAT							 => ps2_DAT
		);
		
END Structure;