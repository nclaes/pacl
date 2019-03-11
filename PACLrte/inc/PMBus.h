/*
 *
 * PMBus.h
 *
 * Library for interfacing with PMBus hardware on the Xilinx ZC702
 *
 */
#ifndef PMBUS_H
#define PMBUS_H



/* -------- Connection -------- */
int i2cOpen(char i2cPort[]);

void i2cClose(int iic_fd);

/* -------- Raw Comms -------- */

// Read a command from the PMBus controller
unsigned int readCommand(int iic_fd, unsigned char device, unsigned char channel, unsigned char command);

// Write a command to the PMBus controller
void writeCommand(int iic_fd, unsigned char device, unsigned char channel, unsigned char command, unsigned int value);


/* -------- Data Formats -------- */

// Convert linear11 format to float
float linear11ToFloat(unsigned short value);

// Convert linear16 to float (exponent is 2^-13)
float linear16ToFloat(unsigned short value);

// Convert float to linear16 (exponent is 2^-12)
unsigned short floatToLinear16(float value);

/* -------- Commands -------- */

float readVoutUVWarnLimit(int iic_fd, unsigned char device, unsigned char channel);

float readVoutUVFaultLimit(int iic_fd, unsigned char device, unsigned char channel);

float readVoutOVWarnLimit(int iic_fd, unsigned char device, unsigned char channel);

float readVoutOVFaultLimit(int iic_fd, unsigned char device, unsigned char channel);


// Write output voltage (CMD_VOUT_COMMAND)
void writeVoutCommand(int iic_fd, unsigned char device, unsigned char channel, float value);

// Read max voltage (CMD_VOUT_MAX)
float readVoutMax(int iic_fd, unsigned char device, unsigned char channel);

// Write max voltage (CMD_VOUT_MAX)
void  writeVoutMax(int iic_fd, unsigned char device, unsigned char channel, float value);


/*
// Read input voltage (CMD_READ_VIN)
float readVin(int iic_fd, unsigned char device, unsigned char channel);
*/

float readVin(int iic_fd, unsigned char device, unsigned char channel);

float readVIN_ON(int iic_fd, unsigned char device, unsigned char channel);


// Read output voltage (CMD_READ_VOUT)
float readVout(int iic_fd, unsigned char device, unsigned char channel);

// Read output current (CMD_READ_IOUT)
float readIout(int iic_fd, unsigned char device, unsigned char channel);

///////////////////////////////////////////////////////////////////////////////////////////////////////////
float readTemp(int iic_fd, unsigned char device, unsigned char channel);

//////////////////////////////////////////////////////////////////////////////////////////////////////////

// Read voltage offset (VOUT_CAL_OFFSET) - 2s complement signed
float readCalOffset(int iic_fd, unsigned char device, unsigned char channel);






#endif
