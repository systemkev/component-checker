#ifndef HELPER_H
#define HELPER_H
    
    #define TOUCH_CMD_X 0xD0
    #define TOUCH_CMD_Y 0x90

    #include <project.h>
        
    void uint32ToString(uint32 value, char* buffer);
    uint8 choose_analysis(void);
    uint32 get_potentiometer_value(void);

#endif 