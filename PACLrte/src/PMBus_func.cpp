#include <stdio.h> 
#include <iostream>

#include "../inc/pacl.h"

#include <random>

using std::cout;
using std::endl;

unsigned int Modes::VS_MODE = NO_VS; // Default mode, no voltage scaling

float voltSettings::voltList[VOLTLIST] = {1.1, 1.1, 1.09, 1.08, 1.07, 1.06, 1.05, 1.04, 1.03, 1.02, 1.01, 1.00};


float readPower(unsigned char channel, unsigned char device, bool usage){

	float power, voltage, current;

	if(1){	// For FPGA
		int pmbus;
   		pmbus = i2cOpen(I2CPORT);
		voltage = readVout(pmbus, device, channel);
		current = readIout(pmbus, device, channel);
		power = voltage * current;
		i2cClose(pmbus);
	}else power = (rand() / (float)RAND_MAX * 1) + 1; // For emulator
	
	//printf("Voltage: %f V, current: %f A, power: %f W\n", voltage, current, power);
	
	return power;
}	


float check_Vin(unsigned char channel, unsigned device, bool mode){
	
	float voltage;
	
	if(mode){	// For FPGA
		int pmbus;
		pmbus = i2cOpen(I2CPORT);
		voltage = readVin(pmbus, device, channel);
		i2cClose(pmbus);
	}else voltage = generateConstrain(9.2, 12); // For emulator
	
	printf("PACL::check_Vin: %f V\n", voltage +1);  //The real input voltage is 1V higher

	return voltage;

}

void  writeVout(unsigned char channel, unsigned char device,  float voltage){  // Function to change individual voltage rails
	
	int pmbus;
	pmbus = i2cOpen(I2CPORT);
	writeVoutCommand(pmbus, device, channel, voltage);
	i2cClose(pmbus);


}


voltageScaling::voltageScaling(void) {    // Constructor, opens I2C PORT
  
   cout << "Voltage Scaling Object is being created" << endl;
}

voltageScaling::~voltageScaling(void) {    // Destructor, closes I2C PORT
 
   cout << "Voltage Scaling Object is being deleted" << endl;
}

void voltageScaling::apply_HPS(float level){

 	pmbus = i2cOpen(I2CPORT);

 	writeVout(HPS_1_1, HPS_MONITOR, level);

	i2cClose(pmbus);
	
}

void voltageScaling::apply_HPS_MIN_Peripheral(){

 	pmbus = i2cOpen(I2CPORT);

	writeVout(HPS_1_5, HPS_MONITOR, 1.36);
	writeVout(HPS_2_5, HPS_MONITOR, 2.28);
	writeVout(HPS_3_3, HPS_MONITOR, 3.0);

	i2cClose(pmbus);
	
}

void voltageScaling::apply_HPS_MAX_Peripheral(){

 	pmbus = i2cOpen(I2CPORT);

	writeVout(HPS_1_5, HPS_MONITOR, 1.5);
	writeVout(HPS_2_5, HPS_MONITOR, 2.5);
	writeVout(HPS_3_3, HPS_MONITOR, 3.3);


	i2cClose(pmbus);
	
}




void voltageScaling::apply_NO_VS() {  // Reset Voltage Rails to default values

    Modes::VS_MODE = NO_VS;

	 pmbus = i2cOpen(I2CPORT);

	writeVout(FPGA_1_1, FPGA_MONITOR, 1.1);
	writeVout(FPGA_1_5, FPGA_MONITOR, 1.5);
	writeVout(FPGA_2_5, FPGA_MONITOR, 2.5);

	writeVout(HPS_1_1, HPS_MONITOR, 1.1);
	writeVout(HPS_1_5, HPS_MONITOR, 1.5);
	writeVout(HPS_2_5, HPS_MONITOR, 2.5);
	writeVout(HPS_3_3, HPS_MONITOR, 3.3);

	  i2cClose(pmbus);

}

void voltageScaling::apply_FULL_VS() {  // Apply VS to FPGA and HPS 

	Modes::VS_MODE = FULL_VS;


	 pmbus = i2cOpen(I2CPORT);

	writeVout(FPGA_1_1, FPGA_MONITOR, 0.96);
	writeVout(FPGA_1_5, FPGA_MONITOR, 1.36);
	writeVout(FPGA_2_5, FPGA_MONITOR, 2.28);

	writeVout(HPS_1_1, HPS_MONITOR, 0.96);
	writeVout(HPS_1_5, HPS_MONITOR, 1.36);
	writeVout(HPS_2_5, HPS_MONITOR, 2.28);
	writeVout(HPS_3_3, HPS_MONITOR, 3.0);

	i2cClose(pmbus);


}
 
void voltageScaling::apply_FPGA_ONLY() {     // Apply VS to FPGA only

	Modes::VS_MODE = FPGA_VS;
	 pmbus = i2cOpen(I2CPORT);

	writeVout(FPGA_1_1, FPGA_MONITOR, 0.96);
	writeVout(FPGA_1_5, FPGA_MONITOR, 1.36);
	writeVout(FPGA_2_5, FPGA_MONITOR, 2.28);

	writeVout(HPS_1_1, HPS_MONITOR, 1.1);
	writeVout(HPS_1_5, HPS_MONITOR, 1.5);
	writeVout(HPS_2_5, HPS_MONITOR, 2.5);
	writeVout(HPS_3_3, HPS_MONITOR, 3.3);

	i2cClose(pmbus);

}

void voltageScaling::apply_HPS_ONLY() {   // Apply VS to HPS only
    
	Modes::VS_MODE = HPS_VS;
		 pmbus = i2cOpen(I2CPORT);

	writeVout(FPGA_1_1, FPGA_MONITOR, 1.1);
	writeVout(FPGA_1_5, FPGA_MONITOR, 1.5);
	writeVout(FPGA_2_5, FPGA_MONITOR, 2.5);;

	writeVout(HPS_1_1, HPS_MONITOR, 0.96);
	writeVout(HPS_1_5, HPS_MONITOR, 1.36);
	writeVout(HPS_2_5, HPS_MONITOR, 2.28);
	writeVout(HPS_3_3, HPS_MONITOR, 3.0);

	i2cClose(pmbus);


}








