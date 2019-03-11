#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include "clock.h"
#include "perf.h"
#include "pacl_utils.h"

#include <sys/time.h>                // for gettimeofday()
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <string>

using std::cout;
using std::endl;



 int perfHost::sampleCount1 = 0;
int perfHost::perfStatus = 0; 
int perfHost::freqCount = 0; 
int perfHost::profBegin = 0;
int perfHost::checkBegin = 0;
int perfHost::sampleCount = 0;
float perfHost::sum = 0;
float perfHost::checkSum = 0;
float perfHost::currentFPS = 0;

float perfHost::freqIndx = 0;





//FILE* perfHost::dataFile = fopen("perf", "w");
FILE* perfHost::perfConstrainFile = fopen("perfConstrain", "w");
FILE* perfHost::perfDataFile = fopen("perfData", "w");
//FILE* perfHost::freqOverheadsFile = fopen("freqOverheads", "w");



perfHost::perfHost(){

	dataFile.open("perf", std::ios::out | std::ios::in );

  
   
	//fprintf(dataFile, "\n--------------Raw data--------------\n");
	//fprintf(dataFile, "FPS \n");
    //printf("\nHost Performance Profiling Has Started!..");
}


void perfHost::begin(int freqNum, int setSamples, int pwrModelStatus){

	if (checkStatus() == 0 && pwrModelStatus == 1)
		collect(freqNum, setSamples);
	/*else if(modelStatus == FAILED){
		perfStatus = 0;
		//dataFile = fopen("perf", "w");
		collect(freqNum, setSamples);
	}*/

}

void perfHost::checkPerf(int setSamples){

	
	if(perfStatus == 1)   // check only after profiling has finished
	{
		switch (checkBegin){
			case 0:
			start();
			checkBegin = 1;
			break;
			
			case 1:
			stop();
			checkSum +=execTime;

		   // printf("\n Sample Count1 = %d\n", sampleCount1);

			if(sampleCount1 == setSamples-1){
				
				currentFPS =  1.0 / (checkSum / setSamples);
				
				//printf("\n checkSum = %f\n", checkSum);
				cout << "\nPACL:: Current FPS = " << currentFPS;

				sampleCount1 = 0;
				checkSum = 0;
				//checkBegin = 0;

			}else sampleCount1++;

			start();
			break;
		}

	}

}

int perfHost::checkStatus(){
  
  return perfStatus;
}

int perfHost::readPerfFile(){
	std::string line;
	float tmp;
	int j = 0;
	if (dataFile.is_open()){
		while ( getline (dataFile,line)){
			tmp=std::stof(line);;
			averageFPS[j] = tmp;
			cout << "\nline = " << averageFPS[j];
			j++;
		}
		dataFile.close();
		return 1;
	}
	else {
		cout << "Unable to open file";
		return 0;
	}
}




void perfHost::setPerformanceConstrain(float perfConstrain){


	//if(currentFPS !=0 && perfStatus == 1){
	if(currentFPS !=0 && lastFPSConstrain != perfConstrain){
	
		if(perfConstrain > MAX_FPS() || perfConstrain < MIN_FPS()){
			cout << "\n *********************************************************";
			cout << "\n Performance requirement is not within recommended limit !";
			cout << "\n MIN_FPS = " << MIN_FPS() << ", MAX_FPS = " << MAX_FPS();
			cout << "\n *********************************************************";

		}

		float perfError = (perfConstrain - currentFPS);
		int tempIndx;

		fprintf(perfDataFile, "\n%f", currentFPS);
		fprintf(perfConstrainFile, "\n%f", perfConstrain);
		cout << "\n New Perf Constrain = "<<perfConstrain;

		//cout << "\n perError = " << perfError;
		//cout << "\n currentFPS = " << currentFPS;
	
		float minError = fabs(perfConstrain - averageFPS[0]);
		//cout << "\n minError = " << minError;

		float tError;
		int newIndx;
	
		if(fabs(perfError) > PERF_THRESHOLD){

			for(int i=1; i< FREQLIST; i++){

				tError = fabs(perfConstrain - averageFPS[i]);
			    //cout << "\ntError = " << tError;

				if( tError < minError){
					//cout << "\nLUT ERROR = " << lutError;
					//cout << "\nIndex = " << i;
					newIndx = i;
					minError = tError;
				}
			}
		freqIndx = newIndx; 

		/*
		if(perfError > 0 && freqIndx < FREQLIST && freqIndx >=0){
			if(freqIndx !=0){
				tempIndx = freqIndx - ceil(fabs(perfError));
				if (tempIndx > 0) freqIndx = tempIndx;
				else freqIndx--;
			}
				
		}else if (perfError < 0 && freqIndx < FREQLIST && freqIndx >=0){
			if(freqIndx !=FREQLIST-1){
				tempIndx = freqIndx + ceil(fabs(perfError));
				if (tempIndx < FREQLIST-1) freqIndx = tempIndx;
				else freqIndx++;
			}
				
		}

		*/

		//cout << "\n NewINdex = " << newIndx;
		//cout << "\n FreqINdex = " << freqIndx;




		struct timeval t1, t2;
		double elapsedTime;

		// start timer
		gettimeofday(&t1, NULL);

		///////////////////////////////////
		set_hps_freq(freqIndx);
		////////////////////////////////////

		// stop timer
		gettimeofday(&t2, NULL);

		// compute and print the elapsed time in millisec
		elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;      // sec to ms
		elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;   // us to ms
		cout << "\n"<<elapsedTime << " ms.\n";
		
		//fprintf(freqOverheadsFile, "%f\n", elapsedTime);


		lastFPSConstrain = perfConstrain;





		}else cout << "\n Performance requirement was met successfully!";
	}
}

float perfHost::MAX_FPS(){
	if (perfStatus) return averageFPS[0]; 
	//else cout << "\n Application Profiling in Progress, MAX FPS measurement is not available!";
}
float perfHost::MIN_FPS(){
	if (perfStatus) return averageFPS[FREQLIST-1];
	//else cout << "\n Application Profiling in Progress, MIN FPS measurement is not available!"; 
}

void perfHost::collect(int freqNum, int setSamples){
        
		

        if (freqCount < freqNum ){


			switch (profBegin) {
        		case 0:
					profBegin = 1;
					set_hps_freq(0); // Start using default frequency of 925 MHz
					start();
					perfStatus = 0;
		   			break;
                    
				case 1:
					stop();
					//printf("\nExec Time: %.2f s\n", execTime);
					sum += execTime;	
					//printf("\n Sample Count = %d", sampleCount);
   					if(sampleCount == setSamples-1){
						averageFPS[freqCount] = 1.0 / (sum / setSamples); 
                    
	                    //fprintf(dataFile, "%f \t %d\n", averageFPS[freqCount], freqSettings::freqList[freqCount]);
						//fprintf(dataFile, "%f\n", averageFPS[freqCount]);
						dataFile << averageFPS[freqCount] << endl;


						printf("\nPACL::FPS = %.3f at %d MHz", averageFPS[freqCount], freqSettings::freqList[freqCount]);
						sum =0;
						sampleCount = 0;
						//printf("\n Frequencies applied = %d", freqCount+1);
						freqCount++;


		

						
						set_hps_freq(freqCount);



					}else sampleCount++;
                    
					start();
					break; 
			}
		}else {
    
		    //fclose(dataFile);
            dataFile.close();  

			 perfStatus = 1; 

			 profBegin = 0;;
			 freqCount = 0;
			 sampleCount = 0;
			 sum = 0;
           
			 set_hps_freq(0); // set back to default frequency after profiling is done

/*
			fprintf(freqOverheadsFile, "\n");


			struct timeval t11, t22;
    		double elapsedTime0;

			set_hps_freq(23);
			sleep(1);

			 // start timer
    		gettimeofday(&t11, NULL);
			set_hps_freq(0);
			// stop timer
    		gettimeofday(&t22, NULL);

   			// compute and print the elapsed time in millisec
  			elapsedTime0 = (t22.tv_sec - t11.tv_sec) * 1000.0;      // sec to ms
   			elapsedTime0 += (t22.tv_usec - t11.tv_usec) / 1000.0;   // us to ms
			
	
			fprintf(freqOverheadsFile, "%f\n", elapsedTime0);


					 // start timer
    		gettimeofday(&t11, NULL);
			set_hps_freq(23);
			// stop timer
    		gettimeofday(&t22, NULL);

   			// compute and print the elapsed time in millisec
  			elapsedTime0 = (t22.tv_sec - t11.tv_sec) * 1000.0;      // sec to ms
   			elapsedTime0 += (t22.tv_usec - t11.tv_usec) / 1000.0;   // us to ms
			fprintf(freqOverheadsFile, "%f\n", elapsedTime0);




					 // start timer
    		gettimeofday(&t11, NULL);
			set_hps_freq(10);
			// stop timer
    		gettimeofday(&t22, NULL);

   			// compute and print the elapsed time in millisec
  			elapsedTime0 = (t22.tv_sec - t11.tv_sec) * 1000.0;      // sec to ms
   			elapsedTime0 += (t22.tv_usec - t11.tv_usec) / 1000.0;   // us to ms
			fprintf(freqOverheadsFile, "%f\n", elapsedTime0);

			*/
			
			//fclose(freqOverheadsFile);
			
			//set_hps_freq(0);			
	
			cout << "\n***********************************************************************";
			cout << "\n\n Host Performance Profiling Has Finished!..";
			cout << "\n Recommended performance constrain range: [ " << MIN_FPS() << " .. " << MAX_FPS() << " ]";
			cout << "\n\n*********************************************************************";
            //this->perfHost::~perfHost();	
		}
        
  }


  float* perfHost::output(){

		 return averageFPS;

  }

  void perfHost::start(){

    start_timer = captureTime();

  }

  void perfHost::stop(){

	execTime = captureTime() - start_timer;
    //printf("\nMeasurement Time: %.3f s\n", execTime);
	
  }

  double perfHost::captureTime(){

     timespec tt;
     clock_gettime(CLOCK_MONOTONIC, &tt);
     return (double(tt.tv_nsec) * 1.0e-9) + double(tt.tv_sec);

  }

  perfHost::~perfHost(){
    
	printf("\nPerformance Object Deleted!..");	
  }  

