#ifndef IND_H
#define IND_H
    
    #define C_REF 46920 // picofarads
    #define CONST_L 43976208  // Derived from 10^21 / (4 * pi^2 * 24MHz^2)
    #include <project.h> 
    
    uint32 inductor_test(uint8 *ring_count, uint32* time);
    void inductor(void);
    void inductor_print(uint32 inductance);
    
#endif