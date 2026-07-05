/* ========================================
 * helper functions
 * ========================================
*/

#include <helper.h>
#include <lcd.h>
#include <cap.h>
#include <res.h>

void uint32ToString(uint32 value, char* buffer) {
    // max uint32 is 10 digits + null terminator
    char temp[11];
    int i = 10; 
    temp[i--] = '\0';       // null terminator
    
    if (value == 0) temp[i--] = '0'; 
    else {
        while(value > 0) {
            char low_bit = '0' + value % 10; 
            value /= 10;
            temp[i--] = low_bit;
        }
    }
    
    char* ptr = &temp[i+1];
    
    while(*ptr) {
        *buffer++ = *ptr++; 
    }
    
    *buffer = '\0';
}

uint8 choose_analysis(void) {
    for(;;) {
        uint32 pot = get_potentiometer_value();
        uint8 choice = (pot / 410) % 5; 
        update_analysis(choice);
        CyDelay(10);
        
        uint8 press = BUTTON1_Read();
        
        if(press == 0) {
            return choice;
        }
    }
}

uint32 get_potentiometer_value(void) {
    int16 raw_value = 0;
    
    // wait the ~2 microseconds for the free-running SAR to finish its current loop
    if(ADC_pot_IsEndConversion(ADC_pot_WAIT_FOR_RESULT)) {
        
        // grab the raw 16-bit result from the SAR
        raw_value = ADC_pot_GetResult16();
        
        // clamp it to zero to prevent negative electrical noise
        if (raw_value < 0) {
            raw_value = 0;
        }
    }
    
    return (uint32)raw_value;
}