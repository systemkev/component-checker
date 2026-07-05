/* ========================================
 * scope / voltmeter 
 * ========================================
*/

#include <volt.h>
#include <stdio.h>
#include <lcd.h>
#include <helper.h>

uint16 volt_test(void) {
    // clear ADC
    ADC_DelSig_IsEndConversion(ADC_DelSig_RETURN_STATUS); 
    ADC_DelSig_GetResult32();
    
    // fetch ADC value
    while(!ADC_DelSig_IsEndConversion(ADC_DelSig_WAIT_FOR_RESULT));
    uint32 final_result = ADC_DelSig_GetResult32();
    int16 mvolts = ADC_DelSig_CountsTo_mVolts(final_result); 
    
    // ensure it is nonnegative
    if (mvolts < 0) mvolts = 0;
    
    return mvolts;
}

void graph(void) {
    uint16 nums[256];
    uint16 print[256]; 
    int i;
    for(i = 0; i < 256; i++) {
        nums[i] = i;
        print[i] = volt_test();
        
        if(i == 100) {
            char val[12];
            uint32ToString(print[i], val);
            LCD_print(180, 100, "            ", COLOR_WHITE, COLOR_BLACK, 2);
            LCD_print(180, 100, val, COLOR_WHITE, COLOR_BLACK, 2);
        }
    }
    
    //clear graph
    LCD_draw_rect(21, 298, 319, 464, COLOR_BLACK);
    LCD_draw_rect(0, 240, 319, 276, COLOR_BLACK);
    LCD_draw_axes();
    
    LCD_graph(nums, print, 256);
}

void scope(void) {
    // 1. Setup UI
    LCD_fill_screen(COLOR_BLACK);
    LCD_print(40, 5, "Scope Mode", COLOR_ICE_BLUE, COLOR_BLACK, 4);
    LCD_print(10, 45, "BTN1: Exit   BTN2: Freeze", COLOR_WHITE, COLOR_BLACK, 2);
    
    LCD_print(16, 200, "Voltage vs Time", COLOR_ICE_BLUE, COLOR_BLACK, 3);
    
    // 2. Disconnect all active drive pins to isolate the probe
    res470_SetDriveMode(res470_DM_ALG_HIZ);
    res470_Write(0);
    res47k_SetDriveMode(res47k_DM_ALG_HIZ);
    res47k_Write(0);
    INDUCTOR_DRIVER_SetDriveMode(INDUCTOR_DRIVER_DM_ALG_HIZ);
    INDUCTOR_DRIVER_Write(0);
    IDAC8_Stop();
    
    uint16 num_samples = 100;
    uint16 time_data[100];
    uint16 volt_data[100];
    
    uint8 is_frozen = 0; // 0 = Live, 1 = Frozen
    
    for(;;) {
        
        // Only gather data and update the screen if we are NOT frozen
        if (!is_frozen) {
            // --- DATA ACQUISITION PHASE ---
            uint16 i;
            for(i = 0; i < num_samples; i++) {
                time_data[i] = i;              
                volt_data[i] = volt_test();   
            }
            
            // --- DISPLAY PHASE ---
            LCD_draw_rect(0, 230, 319, 479, COLOR_BLACK); // clear old graph
            
            char print_v[16];
            uint16 current_v = volt_data[num_samples - 1]; 
            sprintf(print_v, "%u.%03u V   ", current_v / 1000, current_v % 1000);
            LCD_print(76, 100, print_v, COLOR_ICE_BLUE, COLOR_BLACK, 4);
            
            LCD_graph(time_data, volt_data, num_samples);
        }
        
        // --- BUTTON POLLING ---
        
        // BUTTON 1 (Left): Exit completely
        if(BUTTON1_Read() == 0) {
            while(BUTTON1_Read() == 0); // debounce
            break; // Exit the for(;;) loop and return to main()
        }
        
        // BUTTON 2 (Right): Toggle Freeze state
        if(BUTTON2_Read() == 0) {
            while(BUTTON2_Read() == 0); // debounce
            
            is_frozen = !is_frozen; // Flip between 0 and 1
            
            // Update the UI to show the current state
            if (is_frozen) {
                LCD_print(10, 45, "                         ", COLOR_WHITE, COLOR_BLACK, 2);
                LCD_print(10, 45, "Status: FROZEN           ", COLOR_RED, COLOR_BLACK, 2);
            } else {
                LCD_print(10, 45, "                         ", COLOR_WHITE, COLOR_BLACK, 2);
                LCD_print(10, 45, "BTN1: Exit   BTN2: Freeze", COLOR_WHITE, COLOR_BLACK, 2);
            }
        }
    }
}