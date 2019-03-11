#ifndef FUNC_H
#define FUNC_H



// Fixed input clock
#define OSC1                25000000                
#define K_FOR_C0            2

typedef struct {

    unsigned int denominator;
    unsigned int numerator;
    unsigned int Ci_value;

}pll_setting;

int open_physical (int);
void* map_physical (int,unsigned int,unsigned int);
void close_physical (int);
int unmap_physical (void*,unsigned int);
double estimate_frequency(pll_setting *set);




#endif