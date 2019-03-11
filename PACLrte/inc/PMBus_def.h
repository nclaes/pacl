

#ifndef DEFINE_H
#define DEFINE_H

/* -------- Defines -------- */
// i2c Port
#define I2CPORT "/dev/i2c-0"

// Default exponent for Linear11
#define L16EXP -13

// PMBus Commands
#define CMD_PAGE 0x00
#define CMD_VOUT_COMMAND 0x21
#define CMD_VOUT_MAX 0x24
#define CMD_VOUT_OV_WARN_LIMIT 0x42
#define CMD_VOUT_OV_FAULT_LIMIT 0x40
#define CMD_VOUT_UV_WARN_LIMIT 0x43
#define CMD_VOUT_UV_FAULT_LIMIT 0x44
#define CMD_READ_TEMP 0x8D
#define CMD_READ_VIN 0x88
#define CMD_READ_VOUT 0x8B
#define CMD_READ_IOUT 0x8C
#define VOUT_CAL_OFFSET 0x23

#define CMD_READ_VIN_ON 0x35

// PMBus Devices
#define HPS_MONITOR 0x5C
#define FPGA_MONITOR 0x5E

// PSU Channels
// Channels on the FPGA_MONITOR device
#define FPGA_2_5 0
#define FPGA_1_5 2
#define FPGA_1_1 4
#define HPS_VIN 6
#define FPGA_TEMP 7
#define FPGA_VIN_ON 7

// Channels on the HPS_MONITOR device
#define HPS_1_1 0
#define HPS_1_5 2
#define HPS_2_5 4
#define HPS_3_3 6


// Current sense
// All channels except 1.5V (for HPS and FPGA) use 0.001 ohm RSNS
#define RSNS1 1
#define RSNS3 3


#endif
