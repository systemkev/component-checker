#ifndef RES_H
#define RES_H

    #include <project.h>
        
    #define V_DD 3273
    #define R_REF_HIGH 47000
    #define R_REF_SMALL 933
        
    uint32 resistor_test(void); 
    void run_dc_sweep(uint32 res);
    void resistor(void);
    
#endif 