/* ========================================
 * resistor tester
 * ========================================
*/

#include <res.h>
#include <helper.h>
#include <lcd.h>
#include <stdio.h>

uint32 resistor_test(void) {
    // start with 47k resistor
    res470_SetDriveMode(res470_DM_ALG_HIZ);     // disconnect 470 ohm resistor
    res470_Write(0); 
    
    res47k_SetDriveMode(res47k_DM_STRONG);      // set drive mode to strong
    res47k_Write(1); 
    
    IDAC8_Stop();
    
    CyDelay(2);
    
    // wait for end of conversion
    // then grab as 32 bit result
    ADC_DelSig_GetResult32(); 
    while(!ADC_DelSig_IsEndConversion(ADC_DelSig_WAIT_FOR_RESULT));
    uint32 final_result = ADC_DelSig_GetResult32();
    int16 mvolts = ADC_DelSig_CountsTo_mVolts(final_result); 
    
    // prevent negative values from propagating 
    if (mvolts < 0) mvolts = 0;
    
    // prevent divide-by-zero if the ZIF socket is empty
    if (mvolts >= V_DD) mvolts = V_DD - 1;
    
    // voltage divider relationship 
    uint32 R_dut = (uint32) mvolts * R_REF_HIGH / (V_DD - mvolts); 
    
    if (R_dut < 8000) {
        res470_SetDriveMode(res470_DM_STRONG);     // connect 470 ohm resistor
        res470_Write(1); 
        
        res47k_SetDriveMode(res47k_DM_ALG_HIZ);      // set drive mode to high imp
        res47k_Write(0); 
        
        CyDelay(2);
        
        ADC_DelSig_GetResult32(); 
        while(!ADC_DelSig_IsEndConversion(ADC_DelSig_WAIT_FOR_RESULT));
        final_result = ADC_DelSig_GetResult32();
        mvolts = ADC_DelSig_CountsTo_mVolts(final_result); 
        
        // prevent negative values from propagating 
        if (mvolts < 0) mvolts = 0;
        
        // prevent divide-by-zero if the ZIF socket is empty
        if (mvolts >= V_DD) mvolts = V_DD - 1;
        
        // voltage divider relationship 
        R_dut = (uint32) mvolts * R_REF_SMALL / (V_DD - mvolts); 
    }
    
    return R_dut;
}

void run_dc_sweep(uint32 res) {
    // disconnect the reference resistors
    res470_SetDriveMode(res470_DM_ALG_HIZ);     
    res470_Write(0); 
    res47k_SetDriveMode(res47k_DM_ALG_HIZ);      
    res47k_Write(0); 
    
    // bootup the IDAC
    IDAC8_Start();
    CyDelay(2);
    
    uint32 i;
    uint32 end; 
    
    if (res == 0) res = 1; 

    uint32 max_safe_uA = 2500000 / res;
    
    // logic for resistors
    if (res <= 1200) {
        // under 1.2k: use 2mA range (8 uA per step)
        IDAC8_SetRange(IDAC8_RANGE_2mA); 
        i = 1; 
        end = max_safe_uA / 8; 
    } else if (res <= 10000) {
        // 1.2k to 10k: use 255uA range (1 uA per step)
        IDAC8_SetRange(IDAC8_RANGE_255uA); 
        i = 1; 
        end = max_safe_uA;     
    } else {
        // 10k to ~4M: use 32uA range (0.125 uA per step)
        IDAC8_SetRange(IDAC8_RANGE_32uA); 
        i = 1; 
        end = max_safe_uA * 8; // dividing by 0.125 is the same as multiplying by 8
    }
    
    // if the resistor is too big to even get 2 data points, cancel the sweep
    if (end < 2) {
        IDAC8_Stop(); 
        return; 
    }
    
    // hard cap the IDAC hardware limit
    if (end > 255) end = 255;
    
    uint16 volts[256];
    uint16 amps[256];
    uint32 valid_points = 0; 
    
    while (i <= end) {
        IDAC8_SetValue(i);
        CyDelay(1);
        
        ADC_DelSig_IsEndConversion(ADC_DelSig_RETURN_STATUS); 
        ADC_DelSig_GetResult32(); 

        while(ADC_DelSig_IsEndConversion(ADC_DelSig_WAIT_FOR_RESULT) == 0);
        int32 clean_result = ADC_DelSig_GetResult32();
        int16 mvolts = ADC_DelSig_CountsTo_mVolts(clean_result);
        
        if (mvolts < 0) mvolts = 0;
        
        // cutoff
        if (mvolts >= 2500) break; 
        
        amps[valid_points] = i;
        volts[valid_points] = mvolts;
        
        valid_points++;
        i++;
    }
    
    IDAC8_Stop();
    LCD_graph(volts, amps, valid_points); 
}

void print_scaled_ohms(uint32 ohms, uint16 x, uint16 y) {
    char str[16];
    
    if (ohms >= 1000000) {
        // Format as MegaOhms (e.g., "1.24 M")
        sprintf(str, "%lu.%02lu M", ohms / 1000000, (ohms % 1000000) / 10000);
    } else if (ohms >= 1000) {
        // Format as KiloOhms (e.g., "47.5 k")
        sprintf(str, "%lu.%01lu k", ohms / 1000, (ohms % 1000) / 100);
    } else {
        // Format as regular Ohms
        sprintf(str, "%lu", ohms);
    }
    
    LCD_print(x, y, str, COLOR_PEACH, COLOR_BLACK, 2);
}

void resistor(void) {
    LCD_print(52, 0x5, "Resistors", COLOR_PEACH, COLOR_BLACK, 4);
    LCD_print(15, 60, "Results are below!", COLOR_WHITE, COLOR_BLACK, 2); 
    LCD_draw_resistor(280, 65, COLOR_WHITE);
        
    // call resistor test
    uint32 res = resistor_test();
        
    // send to LCD display
    LCD_print(10, 100, "Resistance[Ohm]:", COLOR_WHITE, COLOR_BLACK, 2);
    
    
    if (res < 5) {
        LCD_print(10, 120, "CONTINUITY / SHORT", COLOR_GREEN, COLOR_BLACK, 2);
    } else if (res > 5000000) {
        // If the resistance is > 5 MegaOhms, it's effectively open
        LCD_print(10, 120, "OPEN CIRCUIT", COLOR_RED, COLOR_BLACK, 2);
    } else {
        print_scaled_ohms(res, 220, 100);
    }
    
    LCD_print(16, 200, "Current vs Volts", COLOR_PEACH, COLOR_BLACK, 3);
    
    run_dc_sweep(res);
}