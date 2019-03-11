#include "perfmodel.h"
#include "common_defines.h"
#include "clock.h"
#include <array>



int perfModel::status = 0;

int perfModel::perfDone = 0;
FILE* perfModel::dataFile = fopen("perf", "a");



perfModel::perfModel(){
   
}

float perfModel::predict(int freqIndex){


cout << "\nPredicted FPS: " << modelData[freqIndex]; 
predictedFPS = modelData[freqIndex];
return predictedFPS; 

}

void perfModel::perf(int perfStatus, float* y){



if(perfStatus == 1 && perfDone == 0){

x = freqSettings::freqList;  // x axis

     cout << "\nBuilding Performance Model.. \n";

    for(k = 0; k < d+1; k++){
        for (i = 0; i < n; i++) {
            for (j = 0; j < d+1; j++) {
                 A[k][j] =  A[k][j] + pow(x[i], j + k);
            }
        
            A[k][d+1] += y[i]*pow(x[i], k);
        }    
    }

    for(k = 0; k < d+1; k++){       // invert matrix
         q = A[k][k];         //   divide A[k][] by A[k][k]
                                    //   if q == 0, would need to swap rows                                 
        for(i = 0; i < d+2; i++){
            A[k][i] /= q;}
        for(j = 0; j < d+1; j++){   //   zero out column A[][k]
            if(j == k)
                continue;
             m = A[j][k];         
            for(i = 0; i < d+2; i++){
                A[j][i] -= m*A[k][i];}}}


    cout << endl << "The equation is the following:" << endl;
    for(k = d; k >= 2; k--)
        cout << A[k][d+1] << " x^" << k << " + ";
        cout << A[1][d+1] << " x" << " + " << A[0][d+1] << endl;
    


    float sum = 0;
    float error = 0;

    cout << "\nModel \t\t Raw\t\t Error(%)";

    for (i = 0; i < n; i++){

        for(k = d; k >= 2; k--)
            sum += A[k][d+1]*pow(x[i],k);
            sum += A[1][d+1]*x[i] + A[0][d+1];
            
        error = (fabs(sum - y[i])/y[i])*100;

        if(error > PERF_ERROR){
            cout << "\n\nModel will be re-built using new data!\n";
            recollect();
            return;}
        else cout << "\n"<< sum << "\t\t"<< y[i] << "\t\t"<< error;
    
        modelData[i] = sum;
        sum = 0;
    }

     status = SUCCESS; // Model was built successfully

    fprintf(dataFile, "\n--------------Modeled data--------------\n");
    for (i = 0; i < n; i++)
         fprintf(dataFile, "%f\n", modelData[i]);



    fclose(dataFile);
    cout << "\nPerformance Model was built successfully.. \n";
    perfDone++;

}






 /// save perf model to a file
 /// print model and real data side by side and show error in %






}


void perfModel::recollect(){


status = FAILED;


}


perfModel::~perfModel(){
     
}






