
#include <stdio.h>

#include "func.h"
#include "clock.h"

#include <time.h>

#include <math.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h> 

#define NUMER_MASK  0xFFFF0007
#define DENOM_MASK  0xFFC0FFFF
#define MPUCLK_MASK 0xFFFFFE00
#define LOCK_MASK   0x00000001

#define SET_BIT(val, bitIndex) val |= (1 << bitIndex)
#define CLEAR_BIT(val, bitIndex) val &= ~(1 << bitIndex)

// Clock manager address
#define CLKMGR_BASE         0xFFD04000
#define CLKMGR_LENGTH       0x000001FF

#define PLL_BYPASS          0xFFD04004
#define PLL_LOCK_STATUS     0xFFD04008

// Main PLL register address offsets
#define PLL_VCO             0x00000040
#define PLL_MPUCLK          0x00000048 
// Lock status
#define PLL_INTER           0x00000008

/*              PLL_VCO bit fields                                */
// 313029282726252423222120191817161514131211100908070605040302010
// 1 1 1 0 0 1 1 1 1 0 0 0 1 1 1 0 1 1 1 0 0 0 1 1 1 0 0 0 0 1 1 1
// - - - - - - - - - - - - - - - - 1 0 1 1 1 0 0 0 1 1 1 0 0 - - -     numerator
// - - - - - - - - - - 1 1 1 1 1 1 - - - - - - - - - - - - - - - -     denominator

/*              PLL_MPUCLK bit fields                                */
//  Divides the VCO/2 frequency by the value+1 in this field.
//  3130292827262524232221201918171615141312111009080706050403020100
//  - - - - - - - - - - - - - - - - - - - - - - - 1 0 0 0 0 1 1 1 0 

//  Fref = Fin/N
//  Fvco = Fref x M = Fin x M/N
//  K = 2 for C0, Ci[0..5] for each counter with 0-511 value range
//  Fout = Fvco/(Ci x K)
//  Fout = Fvco/(2 x (Ci + 1))
//  numer 0-4095, the most significant bit of this field is reserved and should not be used.
//  denom 0-63

    unsigned int freqSettings::freqList[] = {925,900,850,800,750,700,650,600,550,500,450,400,350,300,275,250,225,200,175,150,137,125,112,100};
    unsigned int freqSettings::denominatorList[] = {0};
    unsigned int freqSettings::numeratorList[] = {73,71,67,63,59,55,51,47,43,39,35,31,27,23,21,19,17,15,13,11,10,9,8,7};


unsigned int binary_conversion(unsigned int num){
    
    if (num == 0)  return 0;
    else            return (num % 2) + 10 * binary_conversion(num / 2);

}


void delay(unsigned int milliseconds){

    clock_t start = clock();

    while((clock() - start) * 1000 / CLOCKS_PER_SEC < milliseconds);
}


unsigned int update_MAIN_PLL_VCO_NUMER(unsigned int VCO_ptr_val, unsigned int target){

    unsigned int registerVal = VCO_ptr_val;

    // Apply Mask to avoid out of range values
    target &= (~NUMER_MASK >> 3);


   // printf("\n-------------MAIN_PLL_VCO_NUMER_UPDATE-----------\n");
   // printf("\nNumerator input: %d\n", target);
  //  printf("Current VCO Register Value: %X\n", registerVal);
    registerVal &= NUMER_MASK;
   // printf("VCO Register after clearing numerator bitfield: %X\n", registerVal);
    registerVal |= (target << 3); 
   // printf("Updated Register value: %X\n", registerVal);
    
    return registerVal;

}


unsigned int update_MAIN_PLL_VCO_DENOM(unsigned int VCO_ptr_val, unsigned int target){

    unsigned int registerVal = VCO_ptr_val;

    // Apply Mask to avoid out of range values
    target &= (~DENOM_MASK >> 16);

    //printf("\n-------------MAIN_PLL_VCO_DENOM_UPDATE-----------\n");
   // printf("\nDenominator input: %d\n", target);
   // printf("Current VCO Register Value: %X\n", registerVal);
    registerVal &= DENOM_MASK;
   // printf("VCO Register after clearing denominator bitfield: %X\n", registerVal);
    registerVal |= (target << 16); 
  //  printf("Updated Register value: %X\n", registerVal);
    
    return registerVal;

}

unsigned int update_MAIN_PLL_MPUCLK_CNT(unsigned int MPUCLK_ptr_val, unsigned int target){

    unsigned int registerVal = MPUCLK_ptr_val;

    // Apply Mask to avoid out of range values
    target &= (~MPUCLK_MASK);
    
   // printf("\n-------------MAIN_PLL_MPUCLK_CNT_UPDATE-----------\n");
   // printf("\nCounter (CNT) input: %d\n", target);
    //printf("Current MPCLK Register Value: %X\n", registerVal);
    registerVal &= DENOM_MASK;
    //printf("MPUCLK Register after clearing denominator bitfield: %X\n", registerVal);
    registerVal |= (target); 
   // printf("Updated Register value: %X\n", registerVal);
    
    return registerVal;

}

unsigned int check_frequency_setting(unsigned int VCO_ptr_val, unsigned int MPUCLK_ptr_val){

    
    unsigned int num = (VCO_ptr_val & (~NUMER_MASK)) >> 3;
    unsigned int den = (VCO_ptr_val & (~DENOM_MASK)) >> 16;
    unsigned int C0 = MPUCLK_ptr_val & (~MPUCLK_MASK);

    unsigned int N = den + 1;
    unsigned int M = num + 1;

    //printf("\n-------------CURRENT MAIN PLL SETTINGS -----------\n");
    //printf("\nCurrent Value from VCO register NUM: %d,  DEN: %d\n", num, den);
    //printf("Current Value from MPUCLK register C0: %d \n", C0);
 
    double Fref = OSC1 / N;
    double Fvco = Fref * M;
    double Fout = Fvco/ (K_FOR_C0 * (C0 + 1));  // output freq in Hertz

    //printf("Frequency based on current setting, MPU_CLK: %.2f MHz\n\n", Fout/1000000);

    return (Fout/1000000);

}

void check_MAIN_PLL_LOCK_STATUS(unsigned int LOCK_ptr_val){

    unsigned int mainpllachieved = LOCK_ptr_val & LOCK_MASK;
    /*
    printf("\n-------------MAIN PLL LOCK STATUS -----------\n");

    if(mainpllachieved == 1)
        printf("\nMain PLL has achieved lock at least once since this bit was cleared.\n");  
    else    
       printf("\nMain PLL has not achieved lock since this bit was cleared.\n");   
    */

}

unsigned int clear_MAIN_PLL_LOCK_STATUS(unsigned int LOCK_ptr_val){

    unsigned int clear = LOCK_ptr_val & (~LOCK_MASK);
    
   // printf("\nMain PLL lock status bit was cleared.\n\n");   

    return clear;

}

bool check_frequency_change(unsigned int currentFreqency, unsigned int desiredFrequency){

    bool limit = false;
    float change =  fabs((1 - (float)desiredFrequency/(float)currentFreqency) *100);

    //printf("\nChange = %f!\n", change); 

    if (change > 20){
         //printf("\nThe frequency change larger than 20 percent is not allowed!\n"); 
    }else if (change == 0){
         //printf("\nDesired frequency matches the current frequency!\n"); 
    }else{
        limit = true;
       // printf("\nFrequency will be changed to %d MHz!\n", desiredFrequency); 
    }

    return limit;
}

void find_frequency(unsigned int desiredFrequency, unsigned int* requiredDenom, unsigned int* requiredNumer, int* retIndex){

     bool found = false;
     int  index = 0;
    //printf("\nBefore Desired frequency = %d ", desiredFrequency);

    for (int j =0; j<FREQLIST; j++) {

    if (desiredFrequency == freqSettings::freqList[j]){
	    found = true;
	    index = j;
    }

}
    *retIndex = index;
    //printf("\n M Desired frequency = %d ", desiredFrequency);

    if(found){
    *requiredDenom = freqSettings::denominatorList[index];
    *requiredNumer = freqSettings::numeratorList[index];

	}else{

	printf("\nDesired frequency is not available!\n");
    printf("\nDesired frequency = %d ", desiredFrequency);
	exit(1); 

}



}

void set_hps_freq(unsigned int desiredFrequencyIndex){

// virtual address pointer to VCO
    volatile int* VCO_ptr;
    volatile int* MPUCLK_ptr;
    volatile int* LOCK_ptr;
    // used to open /dev/mem
    int fd = -1;
    void* clkmgr_virtual;
    unsigned int currentF;

    bool frequencyChangeLimit = false;

    // Create virtual memory access to HPS clock manager

    if ((fd = open_physical (fd)) == -1)
       exit(0); //return(-1);

    if((clkmgr_virtual = map_physical (fd, CLKMGR_BASE, CLKMGR_LENGTH)) ==NULL)
        exit(0); //return(-1);

    // Set virtual address pointer

    
    VCO_ptr = (volatile int*)((unsigned int*)(clkmgr_virtual + PLL_VCO));
    MPUCLK_ptr = (volatile int*)((unsigned int*)(clkmgr_virtual + PLL_MPUCLK));
    LOCK_ptr = (volatile int*)((unsigned int*)(clkmgr_virtual + PLL_INTER));
    

	


		
    if(freqSettings::freqList[desiredFrequencyIndex] <= 925 && freqSettings::freqList[desiredFrequencyIndex] >= 50){


    int val = 0;

    //printf("\nDesired Frequency = %u MHz\n", desiredFrequency);

	

    // Check register values
    
    /*printf("\n-------------Real values from registers -----------\n");
    printf("\nVCO = %X", *VCO_ptr);
    printf("\nMPU = %X", *MPUCLK_ptr);
    printf("\nLOCK = %X\n\n", *LOCK_ptr);
*/

    int freqIndex;
    unsigned int targetN =0;
    unsigned int targetD =73;
    unsigned int targetMPU = 0;

	
    
   clear_MAIN_PLL_LOCK_STATUS(*LOCK_ptr);

   currentF = check_frequency_setting(*VCO_ptr, *MPUCLK_ptr);


    

     //printf("\nDesired index = %d", desiredFrequencyIndex);
   // printf("\nCurrent frequency = %d MHz \n", currentF);
    
    frequencyChangeLimit = check_frequency_change(currentF, freqSettings::freqList[desiredFrequencyIndex]);
    
     printf("\nDesired index = %d", desiredFrequencyIndex);

    if(frequencyChangeLimit){
   
        

	    find_frequency(freqSettings::freqList[desiredFrequencyIndex], &targetD, &targetN,  &freqIndex);

	    *VCO_ptr = update_MAIN_PLL_VCO_NUMER(*VCO_ptr, targetN);
        delay(20);  

         // needs a small delay, cannot update at the same time
        *VCO_ptr = update_MAIN_PLL_VCO_DENOM(*VCO_ptr, targetD);

        currentF = check_frequency_setting(*VCO_ptr, *MPUCLK_ptr);
       //printf("New frequency = %d MHz\n", currentF);

        check_MAIN_PLL_LOCK_STATUS(*LOCK_ptr);
       }else {

         
  // printf("\nyes");
    

        find_frequency(currentF, &targetD, &targetN, &freqIndex);
        int newIndex = freqIndex;


       
          //       printf("New index = %d \n", newIndex);

        while(currentF != freqSettings::freqList[desiredFrequencyIndex]){

             //printf("\nCurrent frequency is ===== %d MHz \n", check_frequency_setting(*VCO_ptr, *MPUCLK_ptr));  

            if(freqSettings::freqList[desiredFrequencyIndex] > currentF)
                --newIndex;
            else  ++newIndex;    

             // printf("\nNew index1 = %d", newIndex);

            if(check_frequency_change(currentF, freqSettings::freqList[newIndex])){

                find_frequency(freqSettings::freqList[newIndex], &targetD, &targetN, &freqIndex);

	            *VCO_ptr = update_MAIN_PLL_VCO_NUMER(*VCO_ptr, targetN);
                   delay(20); 

                currentF = check_frequency_setting(*VCO_ptr, *MPUCLK_ptr);
                //printf("New frequency = %d MHz\n", currentF);

                check_MAIN_PLL_LOCK_STATUS(*LOCK_ptr);

            }


        }

       }

	}

	
	
    unmap_physical (clkmgr_virtual, CLKMGR_LENGTH);
    close_physical (fd); 
   
    
   




}

