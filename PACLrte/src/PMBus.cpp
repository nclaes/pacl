/*
 *
 * PMBus.h
 *
*/

/* -------- Includes -------- */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "../inc/i2c-dev.h"
#include "../inc/pacl.h"

/* -------- Connection -------- */
int i2cOpen(char i2cPort[]) {

	int iic_fd;
	
	iic_fd = open(i2cPort, O_RDWR);
	if (iic_fd < 0) {
		printf("ERROR: Unable to open i2c port for PMBus access: %d\n", iic_fd);
		exit(1);
	}

	return iic_fd;
}

void i2cClose(int iic_fd) {
	close(iic_fd);
}

/* -------- Raw Comms -------- */

// Read a command from the PMBus controller
unsigned int readCommand(int iic_fd, unsigned char device, unsigned char channel, unsigned char command) {

	int status;
	
	if (ioctl(iic_fd, I2C_SLAVE, device) < 0) {
		printf("ERROR: Unable to set I2C slave address 0x%02X\n", device);
		exit(1);
	}

	status = i2c_smbus_write_byte_data(iic_fd, CMD_PAGE, channel);
	if (status < 0) {
		printf("ERROR: Unable to write page address to I2C slave at 0x%02X: %d\n", device, status);
		exit(1);
	}

	status = i2c_smbus_read_word_data(iic_fd, command);
	if(status < 0) {
		printf("ERROR: Unable to read command (%d) from I2C slave at 0x%02X: %d\n", command, device, status);
		exit(1);
	}
	
	return status;
}

// Write a command to the PMBus controller
void writeCommand(int iic_fd, unsigned char device, unsigned char channel, unsigned char command, unsigned int value) {

	int status;
	
	if (ioctl(iic_fd, I2C_SLAVE, device) < 0) {
		printf("ERROR: Unable to set I2C slave address 0x%02X\n", device);
		exit(1);
	}

	status = i2c_smbus_write_byte_data(iic_fd, CMD_PAGE, channel);
	if (status < 0) {
		printf("ERROR: Unable to write page address to I2C slave at 0x%02X: %d\n", device, status);
		exit(1);
	}

	status = i2c_smbus_write_word_data(iic_fd, command, value);
	//fprintf(stderr, "Writing: %08X\n", value);
	if(status < 0) {
		printf("ERROR: Unable to write command (%d) to I2C slave at 0x%02X: %d\n", command, device, status);
		exit(1);
	}
}


/* -------- Data Formats -------- */

// Convert linear11 format to float
float linear11ToFloat(unsigned short value) {

	// Extract (with sign extension) the exponent
	signed char exponent = value >> 11;
	if (exponent & 0x10) exponent -= 0x20;

	// Extract and sign extend the mantissa
	signed short mantissa = value & 0x7FF;
	if (mantissa & 0x400) mantissa -= 0x800;

	// Return the float computed from mantissa*2^exponent
	return (exponent < 0 ? ((float)mantissa) / (1 << -exponent) 
				: ((float)mantissa) * (1 << exponent));
}

// Convert linear16 to float (exponent is 2^-13)
float linear16ToFloat(unsigned short value) {
	return ((float)value / (1 << -L16EXP));
}

// Convert float to linear16 (exponent is 2^-12)
unsigned short floatToLinear16(float value) {
	return (unsigned short)(value * (1 << -L16EXP));
}

/* -------- Commands -------- */

float readVoutUVWarnLimit(int iic_fd, unsigned char device, unsigned char channel) {
	
	unsigned short response;
	float voltage;
	
	response = readCommand(iic_fd, device, channel, CMD_VOUT_UV_WARN_LIMIT);
	voltage = linear16ToFloat(response);
	
	return voltage;
}

float readVoutUVFaultLimit(int iic_fd, unsigned char device, unsigned char channel) {
	
	unsigned short response;
	float voltage;
	
	response = readCommand(iic_fd, device, channel, CMD_VOUT_UV_FAULT_LIMIT);
	voltage = linear16ToFloat(response);
	
	return voltage;
}

float readVoutOVWarnLimit(int iic_fd, unsigned char device, unsigned char channel) {
	
	unsigned short response;
	float voltage;
	
	response = readCommand(iic_fd, device, channel, CMD_VOUT_OV_WARN_LIMIT);
	voltage = linear16ToFloat(response);
	
	return voltage;
}

float readVoutOVFaultLimit(int iic_fd, unsigned char device, unsigned char channel) {
	
	unsigned short response;
	float voltage;
	
	response = readCommand(iic_fd, device, channel, CMD_VOUT_OV_FAULT_LIMIT);
	voltage = linear16ToFloat(response);
	
	return voltage;
}


// Write output voltage (CMD_VOUT_COMMAND)
void writeVoutCommand(int iic_fd, unsigned char device, unsigned char channel, float value) {
	
	unsigned short voltage;
	float voutMin, voutMax;

	voltage = floatToLinear16(value);
	writeCommand(iic_fd, device, channel, CMD_VOUT_UV_WARN_LIMIT, voltage);
	
	voutMin = readVoutUVFaultLimit(iic_fd, device, channel);
	if (value < voutMin) {
		printf("WARNING: Requested voltage is below FAULT_LIMIT (%f)\n", voutMin);
		//writeCommand(iic_fd, device, channel, CMD_VOUT_UV_WARN_LIMIT, voltage);
		writeCommand(iic_fd, device, channel, CMD_VOUT_UV_FAULT_LIMIT, voltage);
	}
	
	writeCommand(iic_fd, device, channel, CMD_VOUT_COMMAND, voltage);
}

// Read max voltage (CMD_VOUT_MAX)
float readVoutMax(int iic_fd, unsigned char device, unsigned char channel) {
	
	unsigned short response;
	float voltage;
	
	response = readCommand(iic_fd, device, channel, CMD_VOUT_MAX);
	voltage = linear16ToFloat(response);
	
	return voltage;
}

// Write max voltage (CMD_VOUT_MAX)
void  writeVoutMax(int iic_fd, unsigned char device, unsigned char channel, float value) {

	unsigned short voltage;
	
	voltage = floatToLinear16(value);
	writeCommand(iic_fd, device, channel, CMD_VOUT_MAX, voltage);
}


/*
// Read input voltage (CMD_READ_VIN)
float readVin(int iic_fd, unsigned char device, unsigned char channel) {
	
	unsigned short response;
	float voltage;
	
	response = readCommand(iic_fd, device, channel, CMD_READ_VIN);
	voltage = linear16ToFloat(response);
	
	return voltage;
}
*/

float readVin(int iic_fd, unsigned char device, unsigned char channel) {

	unsigned short response;
	float voltage;
	response = readCommand(iic_fd, device, channel, CMD_READ_VIN);
	voltage = linear11ToFloat(response);

	return voltage;

}

float readVIN_ON(int iic_fd, unsigned char device, unsigned char channel) {

	unsigned short response;
	float voltage;
	response = readCommand(iic_fd, device, channel, CMD_READ_VIN_ON);
	voltage = linear11ToFloat(response);

	return voltage;

}


// Read output voltage (CMD_READ_VOUT)
float readVout(int iic_fd, unsigned char device, unsigned char channel) {
	
	unsigned short response;
	float voltage;
	
	response = readCommand(iic_fd, device, channel, CMD_READ_VOUT);
	voltage = linear16ToFloat(response);

	//fprintf(stderr, "VOUT_COMMAND = %f\n", linear16ToFloat(readCommand(iic_fd, device, channel, CMD_VOUT_COMMAND)));
	
	return voltage;
}

// Read output current (CMD_READ_IOUT)
float readIout(int iic_fd, unsigned char device, unsigned char channel) {
	
	unsigned short response;
	float current;

	// Actually read a voltage (across current sense resistor)
	response = readCommand(iic_fd, device, channel+1, CMD_READ_VOUT);
	// Convert current sense voltage (in mV) and scales to Amps with mOhm rsns value
	if (channel == 2) {
		current = linear11ToFloat(response) / RSNS3;
	} else {
		current = linear11ToFloat(response) / RSNS1;
	}

	return current;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
float readTemp(int iic_fd, unsigned char device, unsigned char channel) {
	
	unsigned short response;
	float temp;

	// Actually read a voltage (across current sense resistor)
	response = readCommand(iic_fd, device, channel, CMD_READ_TEMP);
	temp = linear11ToFloat(response);
	

	return temp;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////

// Read voltage offset (VOUT_CAL_OFFSET) - 2s complement signed
float readCalOffset(int iic_fd, unsigned char device, unsigned char channel) {

	unsigned short response;
	float offset;
	
	response = readCommand(iic_fd, device, channel, VOUT_CAL_OFFSET);
	offset = linear16ToFloat(response);
	
	return offset;
}


