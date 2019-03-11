#include "powerMonitor.h"


#include "common_defines.h"
#include "clock.h"
#include <array>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "pacl_utils.h"


int powerMonitor::powerModelStatus = 0;
int powerMonitor::threadActive = 0;
int powerMonitorFeatures::collectStatus = 0;
int powerMonitorFeatures::collectingSamples = 0;
float powerMonitorFeatures::initialVoltageLevel = 1.1;



// Global variables
int monitorSignal = 0;
int prevMonitorSignal = 0;
int loopIterations = 0;

float powerBudget0;
float powerBudget;
float threshold0;
int   pwrdModelHPS_1_1;


FILE* powerMonitorFeatures::measuredPowerFile = fopen("measuredPower", "w");

FILE* powerMonitorFeatures::predictedPowerFile = fopen("predictedPower", "w");
FILE* powerMonitorFeatures::predictedNewPowerFile = fopen("predictedPowerNew", "w");
FILE* powerMonitorFeatures::predictedNew1PowerFile = fopen("predictedPowerNew1", "w");
FILE* powerMonitorFeatures::powerModelFile = fopen("powerModel", "w");


FILE* powerMonitorFeatures::powerBudgetFile = fopen("powerBudget", "w");
FILE* powerMonitorFeatures::averagePowerFile = fopen("averagePower", "w");
FILE* powerMonitorFeatures::HPS_1_5_File = fopen("HPS_1_5", "w");
FILE* powerMonitorFeatures::HPS_2_5_File = fopen("HPS_2_5", "w");
FILE* powerMonitorFeatures::HPS_3_3_File = fopen("HPS_3_3", "w");




powerMonitor::powerMonitor(){
    //updateCounter = 0;
}



void* start_Monitor(void * arg) {

  powerMonitorFeatures *ptrMonitor = (powerMonitorFeatures*) arg;
  ptrMonitor-> realTimeMonitoring();
  printf("\nInstantiating powerMonitor thread \n");
  return NULL;

}

void powerMonitor::printPowerBudgetRangeHPS(){
    
    cout << "\n HPS_MAX Power = " << monitorTask.pwrSample.HPS_PMAX;
    cout << "\n HPS_MIN Power = " << monitorTask.pwrSample.HPS_PMIN;

}

// Returns min and max values of HPS power samples
float powerMonitor::HPS_PMIN() {return monitorTask.pwrSample.HPS_PMIN;}
float powerMonitor::HPS_PMAX() {return monitorTask.pwrSample.HPS_PMAX;}
float powerMonitor::HPS_AVERAGE_1_1() {return monitorTask.pwrSample.AVR_HPS_1_1V;}


void powerMonitor::startMonitorThread(){

    if(threadActive == 0){
    
        threadCheck = pthread_create(&monitorThread, NULL, &start_Monitor, &monitorTask);
       
        if (threadCheck != 0)   printf("Can't create thread: %s\n", strerror(threadCheck));
  	    else threadActive = 1;

    }else {

        // To indicate about a new loop iteration on a host application
        if (monitorSignal <1 ) monitorSignal++;
        else if(monitorSignal >= 1) monitorSignal--;


      
        //updateCounter++;

    }

     //cout <<"\n Join thread!!" << endl;
	//pthread_join(monitorThread, NULL);
}


void powerMonitor::updatePowerBudget(float pwrBudget, int userThreshold){

   

    //powerBudgetUpdateInterval = iterations;

    //if(updateCounter == powerBudgetUpdateInterval){
        //pthread_mutex_lock(&mutex1);
        powerBudget0 = pwrBudget;
        powerBudget = pwrBudget;

        if(userThreshold >= 0 && userThreshold <=100) 
            threshold0 = (POW_THR_MAX * userThreshold) / 100;
        //else if (th)
        else cout << "\n Power budget threshold out of range! (0-100)";
        
        //pthread_mutex_unlock(&mutex1);
        //updateCounter = 0;
        //cout << "\n Power Budget0 = " << powerBudget0;

   // }

   



}


powerMonitor::~powerMonitor(){
    
}


powerMonitorFeatures::powerMonitorFeatures(){

    
    prevMonitorSignal = monitorSignal;
    collectedSamples = 0;
    applied = 0;
    buildStatusHPS_1_1 = 0;

    cout << "power monitor features object" << endl;

}

void powerMonitorFeatures::collectDataHPS_1_1( int iterNum){
  

        if (applied == 0) {

            pwrSample.SUM_HPS_1_1V = 0;

            // Added for peripheral power
            pwrSample.SUM_HPS_1_5V = 0;
            pwrSample.SUM_HPS_2_5V = 0;
            pwrSample.SUM_HPS_3_3V = 0;

            switch(iterNum){

                    // Apply settings to find PV_MAX
                case 1:         
                    setVoltage.apply_HPS(1.1); // Apply 1.1V 
                    set_hps_freq(0);           // Default (925) MHz
                    
                    // Added for peripheral power
                    setVoltage.apply_HPS_MAX_Peripheral();
                    break;
                   // Apply settings to find PV_MIN
                case 2:         
                    setVoltage.apply_HPS(1.0); // Apply 1.0V   
                    set_hps_freq(0);           // Default (925) MHz 
                    break;
                    // Apply settings to find PF_MIN
                case 3:         
                    setVoltage.apply_HPS(1.1);  // Apply 1.1V
                    set_hps_freq(23);           // Apply 100 MHz 

                    // Added for peripheral power
                    setVoltage.apply_HPS_MIN_Peripheral();
                    break;
                case 4:         
                    setVoltage.apply_HPS(1.0);  // Apply 1.0V
                    set_hps_freq(23);           // Apply 100 MHz
                    break;
            }
        }

     

        // Measure power on HPS 1.1V rail

        pwrSample.HPS_1_1V = readPower(HPS_1_1, HPS_MONITOR, 1);

        // Added for peripheral power
       
        pwrSample.HPS_1_5V = readPower(HPS_1_5, HPS_MONITOR, 1);
        pwrSample.HPS_2_5V = readPower(HPS_2_5, HPS_MONITOR, 1);
        pwrSample.HPS_3_3V = readPower(HPS_3_3, HPS_MONITOR, 1);

       

        // Sum samples
        pwrSample.SUM_HPS_1_1V += pwrSample.HPS_1_1V;

        // Added for peripheral power
        pwrSample.SUM_HPS_1_5V += pwrSample.HPS_1_5V;
        pwrSample.SUM_HPS_2_5V += pwrSample.HPS_2_5V;
        pwrSample.SUM_HPS_3_3V += pwrSample.HPS_3_3V;

        cout << "\n Real-time HPS 1.1V, Power = " << pwrSample.HPS_1_1V;

        // Added for peripheral power
        cout << "\n Real-time HPS 1.5V, Power = " << pwrSample.HPS_1_5V; 
        cout << "\n Real-time HPS 2.5V, Power = " << pwrSample.HPS_2_5V; 
        cout << "\n Real-time HPS 3.3V, Power = " << pwrSample.HPS_3_3V << endl; 

        collectingSamples = 1;
        collectedSamples++;
        applied = 1;


}

void powerMonitorFeatures::estimateAveragePower(){

    

        pwrSample.AVR_HPS_1_1V = pwrSample.SUM_HPS_1_1V / collectedSamples;
        
        // Added for peripheral power
        pwrSample.AVR_HPS_1_5V = pwrSample.SUM_HPS_1_5V / collectedSamples;
        pwrSample.AVR_HPS_2_5V = pwrSample.SUM_HPS_2_5V / collectedSamples;
        pwrSample.AVR_HPS_3_3V = pwrSample.SUM_HPS_3_3V / collectedSamples;

        //cout << "\n Sum = "<< pwrSample.SUM_HPS_1_1V;
        cout <<"\n HPS 1.1 Average Power = " << pwrSample.AVR_HPS_1_1V;

        // Added for peripheral power
        cout <<"\n HPS 1.5 Average Power = " << pwrSample.AVR_HPS_1_5V;
        cout <<"\n HPS 2.5 Average Power = " << pwrSample.AVR_HPS_2_5V;
        cout <<"\n HPS 3.3 Average Power = " << pwrSample.AVR_HPS_3_3V;
        
        cout << "\n Collected Samples = " << collectedSamples;

        pwrSample.SUM_HPS_1_1V = 0;

        // Added for peripheral power
        pwrSample.SUM_HPS_1_5V = 0;
        pwrSample.SUM_HPS_2_5V = 0;
        pwrSample.SUM_HPS_3_3V = 0;


        collectedSamples = 0;
    

}

void powerMonitorFeatures::setPowerBudget(){

    //pthread_mutex_lock(&mutex1);   
    
    powerBudget = powerBudget0;
    cout <<"\n Power budget was set: " << powerBudget;

    //pthread_mutex_unlock(&mutex1);


}

void powerMonitorFeatures::recordAveragePower(){


     pwrSample.DATA_HPS_1_1[loopIterations-1] = pwrSample.AVR_HPS_1_1V;


    if (loopIterations == 2){

        pwrSample.HPS_1_5V_MAX = pwrSample.AVR_HPS_1_5V;
        pwrSample.HPS_2_5V_MAX = pwrSample.AVR_HPS_2_5V;
        pwrSample.HPS_3_3V_MAX = pwrSample.AVR_HPS_3_3V;
        fprintf(powerModelFile,"\n-------------Measured Peripheral Power Data----------\n\n");
        fprintf(powerModelFile,"1_5V_MAX: %f \n", pwrSample.HPS_1_5V_MAX);
        fprintf(powerModelFile,"2_5V_MAX: %f \n", pwrSample.HPS_2_5V_MAX);
        fprintf(powerModelFile,"3_3V_MAX: %f \n\n", pwrSample.HPS_3_3V_MAX);

    }else if (loopIterations == 4){

        pwrSample.HPS_1_5V_MIN = pwrSample.AVR_HPS_1_5V;    
        pwrSample.HPS_2_5V_MIN = pwrSample.AVR_HPS_2_5V;
        pwrSample.HPS_3_3V_MIN = pwrSample.AVR_HPS_3_3V;

        fprintf(powerModelFile,"1_5V_MIN: %f \n", pwrSample.HPS_1_5V_MIN);
        fprintf(powerModelFile,"2_5V_MIN: %f \n", pwrSample.HPS_2_5V_MIN);
        fprintf(powerModelFile,"3_3V_MIN: %f \n", pwrSample.HPS_3_3V_MIN);
    }


}


void powerMonitorFeatures::realTimeMonitoring(){

     //setVoltage.apply_HPS_MIN_Peripheral();

    while(1){

        

        // Condition to indicate the start of a new loop on a host application 

        if (prevMonitorSignal != monitorSignal){

            prevMonitorSignal = monitorSignal;
       
            if (collectStatus == 0 && collectingSamples == 1){

                if (loopIterations == MEASURE_ITERATIONS){ // set default settings once collection has finished
                    
                    cout<< "\n*********************** Profiling has finished! ********************";
                    setVoltage.apply_HPS(1.1);
                    set_hps_freq(0);
                    setVoltage.apply_HPS_MAX_Peripheral();
                    collectStatus = 1;
                } 

                estimateAveragePower();  // Estimate av. power before next loop
                recordAveragePower();

                //pwrSample.DATA_HPS_1_1[loopIterations-1] = pwrSample.AVR_HPS_1_1V;
                //cout << "\n HPS MIN/MAX = " << pwrSample.DATA_HPS_1_1[loopIterations-1];

                collectingSamples = 0;
               // collectedSamples = 0;
                applied = 0;

            }else if (collectStatus == 1 && collectingSamples == 0){

                // To test model by applying various VF settings
                
                cout << "\n -----------In test mode (Power Constrain)---------- \n"; 


                printf("\n Predicted HPS 1.1 Power = %f", pwrSample.PREDICTED_HPS_1_1);

                // Added for peripheral power
                pwrSample.PREDICTED_HPS_1_5 = predictPowerHPS_1_5(1.36);
                printf("\n Predicted HPS 1.5 Power = %f", pwrSample.PREDICTED_HPS_1_5);
                pwrSample.PREDICTED_HPS_2_5 = predictPowerHPS_2_5(2.39);
                printf("\n Predicted HPS 2.5 Power = %f", pwrSample.PREDICTED_HPS_2_5);
                pwrSample.PREDICTED_HPS_3_3 = predictPowerHPS_3_3(3.15);
                printf("\n Predicted HPS 3.3 Power = %f", pwrSample.PREDICTED_HPS_3_3);
                
                // (First sample for predicted power will have no value) 
                estimateAveragePower();

                meetPowerConstrain(powerBudget);


                //cout << "\n PMIN = " << pwrSample.HPS_PMIN;
               // cout << "\n PMAX = " << pwrSample.HPS_PMAX;
                      
       
            }
            
            loopIterations++;
            cout<< "\n\n Loop iterations = " << loopIterations << "\n";

        }else {  // Condition while the loop in the host application is running
   
           
            // ****************** Profiling Condition *******************************************************

            /*cout << "\n Collect Status = " << collectStatus;
            cout << "\n Loop iterations = " << loopIterations;
            cout << "\n Build  Status HPS = " << buildStatusHPS_1_1;
            */

            if(collectStatus == 0 && loopIterations > 0 ){

                collectDataHPS_1_1(loopIterations);

             // ********************** Estimate Power model once data collection has finshed *****************
            }else if(collectStatus == 1 && loopIterations >= (MEASURE_ITERATIONS+1) && buildStatusHPS_1_1 == 0 ){ 

                
                // Added for peripheral power
                buildModelHPS_1_1_Peripheral();
                
                buildModelHPS_1_1();


             // ************Condition to simply run power monitoring, e.g. measure and display power readings          
            }else{

                // Measure HPS 1.1 rail power
                pwrSample.HPS_1_1V = readPower(HPS_1_1, HPS_MONITOR, 1);

                // Added for peripheral power
                pwrSample.HPS_1_5V = readPower(HPS_1_5, HPS_MONITOR, 1);
                pwrSample.HPS_2_5V = readPower(HPS_2_5, HPS_MONITOR, 1);
                pwrSample.HPS_3_3V = readPower(HPS_3_3, HPS_MONITOR, 1);

                // To save measured data and predicted power, while testing
                if (collectStatus == 1 && collectingSamples == 0){

                    pwrSample.SUM_HPS_1_1V += pwrSample.HPS_1_1V;

                    // For finding average power after profiling has finished
                    collectedSamples++;

                     

                    fprintf(predictedPowerFile, "%f\n", pwrSample.PREDICTED_HPS_1_1);
                    fprintf(predictedNewPowerFile, "%f\n", pwrSample.PREDICTED_HPS_1_1_NEW);
                    fprintf(predictedNew1PowerFile, "%f\n", pwrSample.PREDICTED_HPS_1_1_NEW_1);

                    fprintf(measuredPowerFile, "%f\n", pwrSample.HPS_1_1V);
                    fprintf(powerBudgetFile, "%f\n", powerBudget);
                    fprintf(averagePowerFile, "%f\n", pwrSample.AVR_HPS_1_1V);

                    // Added for peripheral power
                    fprintf(HPS_1_5_File, "%f\n", pwrSample.HPS_1_5V);
                    fprintf(HPS_2_5_File, "%f\n", pwrSample.HPS_2_5V);  
                    fprintf(HPS_3_3_File, "%f\n", pwrSample.HPS_3_3V);
                }

                /*
                cout << "\n Real-time HPS 1.1V, Power = " << pwrSample.HPS_1_1V; 
                cout << "\n Real-time HPS 1.5V, Power = " << pwrSample.HPS_1_5V; 
                cout << "\n Real-time HPS 2.5V, Power = " << pwrSample.HPS_2_5V; 
                cout << "\n Real-time HPS 3.3V, Power = " << pwrSample.HPS_3_3V << endl; 
                */
            }

            // Add sampling delay
            delay(SAMPLING_DELAY);
    
        }
    }
}


void powerMonitorFeatures::buildModelHPS_1_1(){



 // Function finds coefficients for HPS 1_1 Power equation:
 
 // Voltage scaling equation:       Pv = alfa1 * Voltage + beta1, equation for voltage scaling
 // Frequency scaling equation:     Pf = alfa0 * Voltage^2 * (Frequency - FMAX) +beta0
 // Total power equation            P = alfa0 * Voltage^2 * Frequency + beta0 + C
 // Savings due to voltage scaling: C = Pv - Pf
 // Final equation:                 P = alfa0 * (Voltage^2) * (Frequency - FMAX) + (alfa1 * Voltage + beta1)

 // DATA_HPS_1_1 index meaning:  0 - PV_MAX, 1 - PV_MIN, 2 - PF_MIN,  PV_MAX = PF_MAX
 // MODEL_HPS_1_1 index meaning: 0 - alfa0, 1 - alfa1, 2 - beta1
 // PF_MAX = PV_MAX

 // HPS_PMIN and HPS_PMAX corresdpond to minimum and maximum power samples 
 // and can be used as power budget constrain range

    pwrSample.HPS_PMAX = pwrSample.DATA_HPS_1_1[0];
    pwrSample.HPS_PMIN = pwrSample.DATA_HPS_1_1[3]; 


////////////////////////////////////////////////////////////////

    // new alfan0 = (PV_MIN-PMIN)/(FMAX-FMIN)
    pwrSample.MODEL_HPS_1_1[3] = (pwrSample.DATA_HPS_1_1[1] - pwrSample.DATA_HPS_1_1[3]) / (FMAX - FMIN);
    // betan0
    pwrSample.MODEL_HPS_1_1[4] = pwrSample.DATA_HPS_1_1[1] - (pwrSample.MODEL_HPS_1_1[3]*FMAX);

    // alfan1 = (PMAX - PF_MIN)/(FMAX-FMIN)
    pwrSample.MODEL_HPS_1_1[5] = (pwrSample.DATA_HPS_1_1[0] - pwrSample.DATA_HPS_1_1[2])/(FMAX - FMIN);
    // betan1 = PMAX - alfan1*FMAX
    pwrSample.MODEL_HPS_1_1[6] = pwrSample.DATA_HPS_1_1[0] - (pwrSample.MODEL_HPS_1_1[5]*FMAX);

    fprintf(powerModelFile,"PMAX1: %f \n", pwrSample.DATA_HPS_1_1[0]);
    fprintf(powerModelFile,"PMAX0: %f \n", pwrSample.DATA_HPS_1_1[1]);
    fprintf(powerModelFile,"PMIN1: %f \n", pwrSample.DATA_HPS_1_1[2]);
    fprintf(powerModelFile,"PMIN0: %f \n", pwrSample.DATA_HPS_1_1[3]);

    fprintf(powerModelFile, "\nalfa0\t\talfa1\t\tbeta0\t\tbeta1 \n");
    fprintf(powerModelFile, "%f \t%f\t%f\t%f \n", pwrSample.MODEL_HPS_1_1[3], pwrSample.MODEL_HPS_1_1[5], pwrSample.MODEL_HPS_1_1[4], pwrSample.MODEL_HPS_1_1[6]);
    fclose(powerModelFile);

////////////////////////////////////////////////////////////////

// ****************************Coefficients for Pv equation************************************
// alfa1 = (PV_MAX - PV_MIN) / (VMAX - VMIN)
    pwrSample.MODEL_HPS_1_1[1] = (pwrSample.DATA_HPS_1_1[0] - pwrSample.DATA_HPS_1_1[1]) / (VMAX-VMIN);  
// beta1 = PV_MAX - alfa1 * VMAX;
    pwrSample.MODEL_HPS_1_1[2] = pwrSample.DATA_HPS_1_1[0] - (pwrSample.MODEL_HPS_1_1[1] * VMAX);
// ****************************Coefficients for Pf equation************************************
// alfa0 = (((PV_MAX - PF_MIN) / (FMAX - FMIN))) / (VMAX*VMAX)
    pwrSample.MODEL_HPS_1_1[0] = (((pwrSample.DATA_HPS_1_1[0] - pwrSample.DATA_HPS_1_1[2]) / (FMAX - FMIN))) / (VMAX*VMAX);

    cout << "\n\n*********************************HPS 1.1 Model ***********************************\n";
    cout << "\n P_MAX = " << pwrSample.DATA_HPS_1_1[0] << " W";
    cout << "\n PV_MIN = " << pwrSample.DATA_HPS_1_1[1] << " W";
    cout << "\n PF_MIN = " << pwrSample.DATA_HPS_1_1[2] << " W\n";
    cout << "\n P_MIN = " << pwrSample.DATA_HPS_1_1[3] << " W\n";
    cout << "\n Pv equation =  " << pwrSample.MODEL_HPS_1_1[1] << " * Voltage + " << pwrSample.MODEL_HPS_1_1[2];
    cout << "\n Pf equation =  " << pwrSample.MODEL_HPS_1_1[0] << " * Voltage^2 * (Frequency - " << FMAX << ")";
    cout << "\n P total =  " <<  pwrSample.MODEL_HPS_1_1[0] << " * Voltage^2 * (Frequency - " << FMAX << ") + (" << pwrSample.MODEL_HPS_1_1[1] << "* Voltage + " << pwrSample.MODEL_HPS_1_1[2] << ")";
    
    cout << "\n min slope = " << (pwrSample.DATA_HPS_1_1[2] - pwrSample.DATA_HPS_1_1[3])/ (VMAX-VMIN);
    cout << "\n max slope = " << (pwrSample.DATA_HPS_1_1[0] - pwrSample.DATA_HPS_1_1[1])/ (VMAX-VMIN);
    cout << "\n predicted max slope = " << predictSlopeHPS(freqSettings::freqList[0]);
    cout << "\n predicted min slope = " << predictSlopeHPS(freqSettings::freqList[23]);
    cout << "\n predicted x slope = " << predictSlopeHPS(freqSettings::freqList[20]);
    
    cout << "\n\n***********************************************************************************";

    // Fill Look up table for PF, when V = 1.1V. It will be used for initial frequency setting prediction for a given power budget
    for (int i = 0; i < FREQLIST ; i++){

        pwrSample.HPS_1_1_LUT[i] = predictPowerHPS_1_1(freqSettings::freqList[i], 1.1);

    }



    buildStatusHPS_1_1 = 1;  // To indicate that model was built successfully
    pwrdModelHPS_1_1 = buildStatusHPS_1_1;
    powerMonitor::powerModelStatus = pwrdModelHPS_1_1;

}

void powerMonitorFeatures::buildModelHPS_1_1_Peripheral(){

    // alfa_1_5
    pwrSample.MODEL_HPS_PERIPHERAL_1_5[0] = (pwrSample.HPS_1_5V_MAX - pwrSample.HPS_1_5V_MIN)/(VMAX_1_5-VMIN_1_5);
    // beta_1_5
    pwrSample.MODEL_HPS_PERIPHERAL_1_5[1] = pwrSample.HPS_1_5V_MAX - (pwrSample.MODEL_HPS_PERIPHERAL_1_5[0] * VMAX_1_5);

    // alfa_2_5
    pwrSample.MODEL_HPS_PERIPHERAL_2_5[0] = (pwrSample.HPS_2_5V_MAX - pwrSample.HPS_2_5V_MIN)/(VMAX_2_5-VMIN_2_5);
    // beta_2_5
    pwrSample.MODEL_HPS_PERIPHERAL_2_5[1] = pwrSample.HPS_2_5V_MAX - (pwrSample.MODEL_HPS_PERIPHERAL_2_5[0] * VMAX_2_5);
     
     // alfa_3_3
    pwrSample.MODEL_HPS_PERIPHERAL_3_3[0] = (pwrSample.HPS_3_3V_MAX - pwrSample.HPS_3_3V_MIN)/(VMAX_3_3-VMIN_3_3);
    // beta_3_3
    pwrSample.MODEL_HPS_PERIPHERAL_3_3[1] = pwrSample.HPS_3_3V_MAX - (pwrSample.MODEL_HPS_PERIPHERAL_3_3[0] * VMAX_3_3);

    fprintf(powerModelFile,"\n----------------Peripheral Power Models-------------\n");
    fprintf(powerModelFile,"\nParameter: Alfa_n \t Beta_n \n\n");
    fprintf(powerModelFile,"HPS 1_5V : %f \t %f\n", pwrSample.MODEL_HPS_PERIPHERAL_1_5[0], pwrSample.MODEL_HPS_PERIPHERAL_1_5[1]);
    fprintf(powerModelFile,"HPS 2_5V : %f \t %f\n", pwrSample.MODEL_HPS_PERIPHERAL_2_5[0], pwrSample.MODEL_HPS_PERIPHERAL_2_5[1]);
    fprintf(powerModelFile,"HPS 3_3V : %f \t %f\n", pwrSample.MODEL_HPS_PERIPHERAL_3_3[0], pwrSample.MODEL_HPS_PERIPHERAL_3_3[1]);
    fprintf(powerModelFile,"------------------HPS Core Power Model----------------\n");


}



int powerMonitorFeatures::findMinErrorIndex(float newPowBudget, float LUT[]){

    // Function returns index which corresponds to frequency value with lowest error

    float minError = fabs(LUT[0]- newPowBudget);   // At F = 925 MHz, V = 1.1V
    float pError = 0;  

    for(int j =0; j < FREQLIST-1; j++){  // Starting from the highest frequency

        pError =  fabs(LUT[j] - newPowBudget);
 
        if (pError <= minError && pError >= 0){  
            minError = pError;     
        }else  return j;
   
 
    }   
}


float powerMonitorFeatures::predictPowerHPS_1_1(int fVal, float vLevel){

 // Function estimates power based on provided frequency and voltage settings
 // P = alfa0 * (Voltage^2) * (Frequency - FMAX) + (alfa1 * Voltage + beta1)

 return pwrSample.MODEL_HPS_1_1[0]*(vLevel*vLevel)*(fVal-FMAX) + (pwrSample.MODEL_HPS_1_1[1]*vLevel + pwrSample.MODEL_HPS_1_1[2]);
 

}

float powerMonitorFeatures::predictPowerHPS_1_5(float vLevel){
    
 return pwrSample.MODEL_HPS_PERIPHERAL_1_5[0]*vLevel + pwrSample.MODEL_HPS_PERIPHERAL_1_5[1];
}

float powerMonitorFeatures::predictPowerHPS_2_5(float vLevel){
    
 return pwrSample.MODEL_HPS_PERIPHERAL_2_5[0]*vLevel + pwrSample.MODEL_HPS_PERIPHERAL_2_5[1];
}

float powerMonitorFeatures::predictPowerHPS_3_3(float vLevel){
    
 return pwrSample.MODEL_HPS_PERIPHERAL_3_3[0]*vLevel + pwrSample.MODEL_HPS_PERIPHERAL_3_3[1];
}




float powerMonitorFeatures::predictPowerHPS_1_1_NEW(int fVal, float vLevel){


 // Function estimates power based on provided frequency and voltage settings
 // P = alfa0 * (Voltage^2) * (Frequency - FMAX) + (alfa1 * Voltage + beta1)

  float PFS_MAX, PFS_MIN, alfan2, betan2, predict;

      // P_FS_MAX
    PFS_MAX =  pwrSample.MODEL_HPS_1_1[5] * (fVal) + pwrSample.MODEL_HPS_1_1[6];

     // P_FS_MIN
    PFS_MIN =  pwrSample.MODEL_HPS_1_1[3] * (fVal) + pwrSample.MODEL_HPS_1_1[4];  

    alfan2 = (PFS_MAX - PFS_MIN)/(VMAX-VMIN);
    betan2 = PFS_MAX - (alfan2 * VMAX);

    predict = alfan2*(vLevel) + betan2;


 return predict; 
 

}

float powerMonitorFeatures::predictPowerHPS_1_1_NEW_1(int fVal, float vLevel){


    float  alfan3, betan3, PFS_MAX, predict;
  
    PFS_MAX =  pwrSample.MODEL_HPS_1_1[5] * (fVal) + pwrSample.MODEL_HPS_1_1[6];
    alfan3 = predictSlopeHPS(fVal);

    // alfan3
    //pwrSample.MODEL_HPS_1_1[7] = alfan3;

    
    // betan3
    betan3 = PFS_MAX - (alfan3 * VMAX);

    //pwrSample.MODEL_HPS_1_1[8] = betan3;
   

    predict = alfan3*(vLevel) + betan3;


 return predict; 
 

}

float powerMonitorFeatures::predictSlopeHPS(int fVal){

    float  alfan3, betan3, predictSlope, predict;
    float maxSlope, minSlope;

    maxSlope = (pwrSample.DATA_HPS_1_1[0] - pwrSample.DATA_HPS_1_1[1])/ (VMAX-VMIN);
    minSlope = (pwrSample.DATA_HPS_1_1[2] - pwrSample.DATA_HPS_1_1[3])/ (VMAX-VMIN);

    alfan3 = (maxSlope - minSlope) / (FMAX - FMIN);
    betan3 = maxSlope - alfan3*FMAX;

    return alfan3*(fVal) + betan3;

}



void powerMonitorFeatures::applyVFScaling(float randomVoltage, int randomFrequencyIndex){

    /*
    
    int randomVoltageIndex = RandomInt(0, 11);
    int randomFrequencyIndex = RandomInt(0, 23);

    int randomFrequency = freqSettings::freqList[randomFrequencyIndex];
    float randomVoltage = voltSettings::voltList[randomVoltageIndex];

   */


  

    /// New prediction test

    pwrSample.PREDICTED_HPS_1_1_NEW = predictPowerHPS_1_1_NEW(freqSettings::freqList[randomFrequencyIndex], randomVoltage);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////


    /// New prediction test with slope prediction

    pwrSample.PREDICTED_HPS_1_1_NEW_1 = predictPowerHPS_1_1_NEW_1(freqSettings::freqList[randomFrequencyIndex], randomVoltage);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////



    // Make a prediction using HPS 1.1V Model
    
    pwrSample.PREDICTED_HPS_1_1 = predictPowerHPS_1_1(freqSettings::freqList[randomFrequencyIndex], randomVoltage);



    
    cout << "\n\n******************* Running Test on HPS 1.1 Model  *********************\n";
    cout << "\n V-F settings, voltage = " << randomVoltage << " V, frequency =  " << freqSettings::freqList[randomFrequencyIndex] << " MHz";
    printf("\n Predicted HPS 1.1 Power = %f", pwrSample.PREDICTED_HPS_1_1);
    printf("\n New Predicted HPS 1.1 Power = %f", pwrSample.PREDICTED_HPS_1_1_NEW);
    printf("\n New (incl. slope pred.) Predicted HPS 1.1 Power = %f", pwrSample.PREDICTED_HPS_1_1_NEW_1);





    cout << "\n\n***********************************************************************";
    
     

                double start_time = getCurrentTime();

       
    // ************************Apply test settings***********************
    setVoltage.apply_HPS(randomVoltage); // Input is a float value of voltage level

                double elapsed_time = getCurrentTime() - start_time;
                printf("\n Elapsed time: %0.5f seconds", elapsed_time);
    set_hps_freq(randomFrequencyIndex);  // Input is an index of frequency array

    // ****************Print predicted value using applied settings *****


}



void powerMonitorFeatures::estimateEnergy_HPS_1_1(float* fpsData){

    for(int j = 0; j < FREQLIST; j++){

        pwrSample.HPS_1_1_ENERGY[j] = (1/fpsData[j]) * pwrSample.HPS_1_1_LUT[j];
        cout << "\n estimated energy = " << pwrSample.HPS_1_1_ENERGY[j];

    }


}

int powerMonitorFeatures::findMinEnergy_HPS_1_1(){

    float minEnergyPoint;

    int minIndex = 0;
    minEnergyPoint = pwrSample.HPS_1_1_ENERGY[0];

    for(int j = 1; j < FREQLIST; j++){

        if(pwrSample.HPS_1_1_ENERGY[j] < minEnergyPoint){
            minEnergyPoint = pwrSample.HPS_1_1_ENERGY[j];
            minIndex = j;
        }
    }
    cout << "\n Index = " << minIndex <<" Energy = "<< pwrSample.HPS_1_1_ENERGY[minIndex];
    return minIndex;

}

/*float powerMonitorFeatures::checkEnergy_1_1(){


}*/



void powerMonitorFeatures::meetPowerConstrain(float pBudget){


    cout << "\n Threshold0 = " << threshold0;

    // To check if a new requirement matches the previous constrain
    
    if(pBudget == lastPowerBudget) 
        cout << "\n Power budget requirement hasn't been modified!";

    else{

    cout << "\n MPC Power budget = " << pBudget;

//  applyVFScaling(newVoltageLevel, newFreqIndex);  

    newVoltageLevel = initialVoltageLevel;
    // Make initial frequency prediction 
    newFreqIndex = findMinErrorIndex(pBudget, pwrSample.HPS_1_1_LUT);
    cout << "\n New freq indx = " << findMinErrorIndex(0.315, pwrSample.HPS_1_1_LUT);

    // Estimate an error between power constrain and predicted power
    
    float predictionError = pBudget - predictPowerHPS_1_1(freqSettings::freqList[newFreqIndex], initialVoltageLevel);
    cout << "\n Prediction error = " << predictionError;
   
    int checkState = 0;

    while(predictionError < threshold0 ){
    
    checkState = 1;

        if (newVoltageLevel > 1.0){  // Apply Voltage Scaling first

            newVoltageLevel -= 0.01;

        }else{  // Adjust initial frequency value prediction            
            
            ++newFreqIndex;

            if (newFreqIndex >= FREQLIST - 1){
                newFreqIndex = FREQLIST - 1;
                cout << "\n Power constrain cannot be met!";
                break;
            }
            cout << "\n Inital Frequency value was re-adjusted!";
            //cout << "\n Selected new frequency = " << freqSettings::freqList[newFreqIndex];
        }

        predictionError =  pBudget - predictPowerHPS_1_1(freqSettings::freqList[newFreqIndex], newVoltageLevel);
    }


       cout << "\n Selected frequency (MHz) = " << freqSettings::freqList[newFreqIndex];
       cout << "\n Selected voltage level = " << newVoltageLevel << endl;


    // Call function to apply new settings
    
     if(checkState == 0 && pBudget < 0.45)  //HPS_PMAX()+HPS_PMIN())/2)
     {
         //newFreqIndex =   newFreqIndex = FREQLIST - 1;
          newFreqIndex = findMinErrorIndex(pBudget, pwrSample.HPS_1_1_LUT);
         //newVoltageLevel = initialVoltageLevel;
     }

      //clock_t start, end;

    //start = clock();
    
    applyVFScaling(newVoltageLevel, newFreqIndex); 

    //end = clock();

    //double duration_sec = double(end-start)/CLOCKS_PER_SEC;
    //cout << "\n\n" << "Freq delay =" <<  duration_sec << "\n\n";
   

    }

    lastPowerBudget = pBudget;

}

/*
void powerMonitorFeatures::findSettings(float powerConstrain){

     // Frequency scaling equation:     Pf = alfa0 * Voltage^2 * (Frequency - FMAX)
    
    float estFreq;
    float initalPred;
    float predError;

    float stepSize = 0.01;

    newVoltageLevel = initialVoltageLevel;
    powerBudget = powerConstrain;


   // for(int j = 0; j < 24; j++){

        estFreq = (powerBudget + pwrSample.MODEL_HPS_1_1[0]*(1.1)*(1.1)*925) / (pwrSample.MODEL_HPS_1_1[0] *1.1*1.1); 
       // cout << "\n \n f = " << estFreq << endl;
        //cout << "\nPower budget = " << powerBudget << endl;
        //cout << pwrSample.MODEL_HPS_1_1[0];
       // cout << predictPowerHPS_1_1(estFreq, 1.1);
    //}




        float error = 0;// = powerBudget - predictedPower(freqSettings::freqList[j], 1.1);

       // while(error >= 0){

        float prevError = fabs(powerBudget - predictPowerHPS_1_1(freqSettings::freqList[0], 1.1));

        //cout << "Initial error" << prevError << endl;

        for(int j = 1; j < 24; j++){

            error =  fabs(powerBudget - predictPowerHPS_1_1(freqSettings::freqList[j], initialVoltageLevel));

            //cout << "\n Error = " << error; 
            //cout << "\n Previous Error = " << prevError; 

            if (error > prevError){

                initalPred = predictPowerHPS_1_1(freqSettings::freqList[j], initialVoltageLevel);
                predError =  powerBudget - initalPred;
                newFreqIndex = j;

                cout << "\n\n******************* Running Power Constrain Test *********************\n";
                cout << "\n Power budget = " << powerBudget << " W";
                cout << "\n Inital Prediction (no VS) = " << initalPred << " W";
                cout << "\n Prediction error = " << predError;
                
                if(predError < POWER_THRESHOLD0){

                    do{
                    
                     newVoltageLevel -= stepSize;
                     predError =  powerBudget - predictPowerHPS_1_1(freqSettings::freqList[j], newVoltageLevel);

                     if(newVoltageLevel <= 1.0) {
                        newVoltageLevel = 1.0;
                        
                        do{
                            ++newFreqIndex;
                            if (newFreqIndex >= FREQLIST - 1){
                                newFreqIndex = FREQLIST - 1;
                                cout << "\n Power constrain cannot be met!";
                                break;
                            }

                        }while( (powerBudget - predictPowerHPS_1_1(freqSettings::freqList[newFreqIndex], newVoltageLevel)) < POWER_THRESHOLD1);
                        
                        
                        cout << "\n Inital Frequency value was re-adjusted!";
                        cout << "\n New frequency index = " << newFreqIndex;
                        //cout << "\n Selected new frequency = " << freqSettings::freqList[j-1];
                        break;
                     }

                    }while(predError < POWER_THRESHOLD1);
                      
                  cout << "\n Prediction (with VS) = " << predictPowerHPS_1_1(freqSettings::freqList[newFreqIndex], newVoltageLevel) << " W";
                    


                }
                cout << "\n Selected frequency (MHz) = " << freqSettings::freqList[newFreqIndex];
                cout << "\n Selected voltage level = " << newVoltageLevel << endl;

                break;

            }

           
            prevError = error;   

        }
    // }
}

*/

powerMonitorFeatures::~powerMonitorFeatures(){

    
}


