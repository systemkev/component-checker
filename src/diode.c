/* ========================================
 * diode tester
 * ========================================
*/

#include <diode.h> 
#include <lcd.h>
#include <helper.h>

uint16 diode_test(void) {
    res47k_SetDriveMode(res47k_DM_ALG_HIZ);
    res47k_Write(0);
    
    res470_SetDriveMode(res470_DM_STRONG);
    res470_Write(1); 
    
    CyDelay(5);
    
    uint16 vth;
    
    ADC_DelSig_IsEndConversion(ADC_DelSig_RETURN_STATUS); 
    ADC_DelSig_GetResult32(); 
    while(ADC_DelSig_IsEndConversion(ADC_DelSig_WAIT_FOR_RESULT) == 0);
    int16 mvolts = ADC_DelSig_CountsTo_mVolts(ADC_DelSig_GetResult32());
    
    if(mvolts < 0) mvolts = 0;
    
    if(mvolts > 3200) {
        vth = 0;         // reverse the polarity / diode OFF
    } else {
        vth = mvolts;       // vth = vdut 
    } 
    
    res47k_SetDriveMode(res47k_DM_ALG_HIZ);
    res47k_Write(0);
    
    res470_SetDriveMode(res470_DM_ALG_HIZ);
    res470_Write(0); 
    
    return vth;
}

void diode_dc_sweep(void) {
    IDAC8_Start();
    IDAC8_SetRange(IDAC8_RANGE_2mA);
    
    uint16 volts[256];
    uint16 amps[256];
    uint32 valid_points = 0; 
    uint16 i = 0;
    
    while(i < 256) {
        IDAC8_SetValue(i);
        CyDelay(1);
        
        ADC_DelSig_IsEndConversion(ADC_DelSig_RETURN_STATUS); 
        ADC_DelSig_GetResult32(); 

        while(ADC_DelSig_IsEndConversion(ADC_DelSig_WAIT_FOR_RESULT) == 0);
        int32 clean_result = ADC_DelSig_GetResult32();
        int16 mvolts = ADC_DelSig_CountsTo_mVolts(clean_result);
        
        if(mvolts > 3200) {
            break;
        }
        
        volts[i] = mvolts;      // volts
        amps[i] = i * 8;        // microamps 
        valid_points++;
        i++;
    }
    IDAC8_Stop();
    
    LCD_graph(volts, amps, valid_points);
}

void diode(void) {
    LCD_print(10, 0x5, "Diode/LED", COLOR_MINT, COLOR_BLACK, 4);
    LCD_draw_diode(280, 20, COLOR_MINT);
    LCD_print(0x7, 45, "Your results are below!", COLOR_WHITE, COLOR_BLACK, 2);
    
    LCD_print(10, 100, "Thresh. Vlt.(mV):", COLOR_WHITE, COLOR_BLACK, 2);
    LCD_print(220, 100, "...", COLOR_RED, COLOR_BLACK, 2);
    
    LCD_print(16, 200, "Current vs Volts", COLOR_MINT, COLOR_BLACK, 3);
    
    uint16 diode = diode_test();
    LCD_print(220, 100, "   ", COLOR_WHITE, COLOR_BLACK, 2);
    
    if(diode > 0) {
        char disp[12];
        uint32ToString(diode, disp);
        LCD_print(220, 100, disp, COLOR_ICE_BLUE, COLOR_BLACK, 2);
        diode_dc_sweep();
        
        if (diode < 100) {
            LCD_print(10, 120, "Type: SHORTED", COLOR_RED, COLOR_BLACK, 2);
        } else if (diode > 100 && diode < 450) {
            LCD_print(10, 120, "Type: SCHOTTKY", COLOR_MINT, COLOR_BLACK, 2);
        } else if (diode >= 450 && diode < 900) {
            LCD_print(10, 120, "Type: SILICON", COLOR_MINT, COLOR_BLACK, 2);
        } else {
            LCD_print(10, 120, "Type: LED", COLOR_MINT, COLOR_BLACK, 2);
        }
       
    } else {
        LCD_print(220, 100, "REVERSE", COLOR_ICE_BLUE, COLOR_BLACK, 2);
        LCD_print(220, 120, "POLARITY", COLOR_ICE_BLUE, COLOR_BLACK, 2);
    }
}