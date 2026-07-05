/* ========================================
 *capacitor test routine
 * ========================================
*/

#include <cap.h> 
#include <helper.h>
#include <lcd.h>
#include <res.h>
void discharge(void){
    res470_SetDriveMode(res470_DM_STRONG);     
    res470_Write(0); 
    res47k_SetDriveMode(res47k_DM_ALG_HIZ);      
    res47k_Write(0); 
    IDAC8_Stop();
    
    while(1) {
        ADC_DelSig_IsEndConversion(ADC_DelSig_RETURN_STATUS); 
        ADC_DelSig_GetResult32(); 
        
        while(ADC_DelSig_IsEndConversion(ADC_DelSig_WAIT_FOR_RESULT) == 0);
        int16 mvolts = ADC_DelSig_CountsTo_mVolts(ADC_DelSig_GetResult32());
        
        if (mvolts <= 5) {
            break; 
        }
    }
}
uint32 capacitor_test(void) {
    // ------------------------------------------------
    // range 1: small capacitor (use 47k resistor) 
    // ------------------------------------------------
    discharge(); 
    
    res470_SetDriveMode(res470_DM_ALG_HIZ);
    res470_Write(0);
    
    CyDelay(2);
    
    res47k_SetDriveMode(res47k_DM_STRONG);
    res47k_Write(1); 
    
    uint32 ticks = 0; 
    uint32 final_capacitance = 0;
    
    uint16 volts_graph[250]; 
    uint16 time_graph[250];
    uint16 graph_index = 0;
    uint16 sample_rate = 1; // Start by capturing every tick
    int16 mvolts; 
    uint16 k;
    
    while(1) {
        while(ADC_DelSig_IsEndConversion(ADC_DelSig_WAIT_FOR_RESULT) == 0);
        mvolts = ADC_DelSig_CountsTo_mVolts(ADC_DelSig_GetResult32());
    
        ticks++; 
        
        // Dynamic Graphing Algorithm
        if (ticks % sample_rate == 0) {
            time_graph[graph_index] = ticks;
            volts_graph[graph_index] = mvolts;
            graph_index++;
            
            // If array is full, compress it to make room for more data
            if (graph_index >= 250) {
                for (k = 0; k < 125; k++) {
                    time_graph[k] = time_graph[k * 2];
                    volts_graph[k] = volts_graph[k * 2];
                }
                graph_index = 125; // Reset index to halfway
                sample_rate *= 2;  // Double the time between samples
            }
        }
        
        if (mvolts >= 2069) {
            final_capacitance = 100 * ticks / 47; 
            break; // Success! Exit the loop.
            
        } else if (ticks > 223) {
            break; // Took > 223ms. Cap is too big. Move to Range 2.
        }
    }
    
    // ------------------------------------------------
    // range 2: large capacitor (use 470 ohm resistor)
    // ------------------------------------------------
    if (ticks > 223) {
        discharge();
        ticks = 0;
        graph_index = 0;
        sample_rate = 1; // Reset sample rate for the new test
        final_capacitance = 0;
        
        res47k_SetDriveMode(res47k_DM_ALG_HIZ);
        res47k_Write(0); 
        
        CyDelay(2);
        
        res470_SetDriveMode(res470_DM_STRONG);
        res470_Write(1);
        
        // flush the ADC buffer before counting!
        ADC_DelSig_IsEndConversion(ADC_DelSig_RETURN_STATUS); 
        ADC_DelSig_GetResult32();
        
        while(1) {
            while(ADC_DelSig_IsEndConversion(ADC_DelSig_WAIT_FOR_RESULT) == 0);
            mvolts = ADC_DelSig_CountsTo_mVolts(ADC_DelSig_GetResult32());
        
            ticks++; 
            
            // Dynamic Graphing Algorithm
            if (ticks % sample_rate == 0) {
                time_graph[graph_index] = ticks;
                volts_graph[graph_index] = mvolts;
                graph_index++;
                
                // If array is full, compress it to make room for more data
                if (graph_index >= 250) {
                    for (k = 0; k < 125; k++) {
                        time_graph[k] = time_graph[k * 2];
                        volts_graph[k] = volts_graph[k * 2];
                    }
                    graph_index = 125;
                    sample_rate *= 2;
                }
            }
            
            if (mvolts >= 2069) {
                // scaled math for 932 ohms
                final_capacitance = 100000 * ticks / 932;
                break; 
            }
            
            // timeout after 5 seconds 
            if (ticks > 50000) { 
                final_capacitance = 0; // open circuit
                break;
            }
        }
    }
    
    // clean up 
    res47k_SetDriveMode(res47k_DM_ALG_HIZ);
    res47k_Write(0);
    res470_SetDriveMode(res470_DM_ALG_HIZ);
    res470_Write(0);
    CyDelay(2);
    
    LCD_graph(time_graph, volts_graph, graph_index);
    
    return final_capacitance;
}

uint16 esr_test(void) {
    discharge(); 
    
    res470_SetDriveMode(res470_DM_ALG_HIZ);     
    res470_Write(0); 
    res47k_SetDriveMode(res47k_DM_ALG_HIZ);      
    res47k_Write(0); 
    
    IDAC8_Start();
    CyDelay(2);
    
    IDAC8_SetRange(IDAC8_RANGE_2mA);
    IDAC8_SetValue(0xFF); // 2.04 mA
    
    ADC_DelSig_IsEndConversion(ADC_DelSig_RETURN_STATUS); 
    ADC_DelSig_GetResult32(); 

    while(ADC_DelSig_IsEndConversion(ADC_DelSig_WAIT_FOR_RESULT) == 0);
    int32 clean_result = ADC_DelSig_GetResult32();
    
    // use microvolts for extreme precision 
    int32 uvolts = ADC_DelSig_CountsTo_uVolts(clean_result);
    
    IDAC8_Stop(); 
    
    if (uvolts < 0) uvolts = 0;

    // (uvolts * 100) / 204
    return (uint16)((uvolts * 100) / 204);
}

uint32 leakage_current(void) {
    discharge(); 
    
    // ==========================================
    // phase 1: fast charge using 470 ohms
    // ==========================================
    res47k_SetDriveMode(res47k_DM_ALG_HIZ);
    res47k_Write(0);
    
    res470_SetDriveMode(res470_DM_STRONG);
    res470_Write(1); 
    
    uint32 timeout = 0;
    while(1) {
        ADC_DelSig_IsEndConversion(ADC_DelSig_RETURN_STATUS); 
        ADC_DelSig_GetResult32(); 
        while(ADC_DelSig_IsEndConversion(ADC_DelSig_WAIT_FOR_RESULT) == 0);
        int16 mvolts = ADC_DelSig_CountsTo_mVolts(ADC_DelSig_GetResult32());
        
        // Stop fast charging when we get close to the top
        if(mvolts >= 3200) break; 
        
        timeout++;
        if (timeout > 50000) break; // 5-second safety timeout
    }
    
    // ==========================================
    // phase 2: precision top off using 470k
    // ==========================================
    res470_SetDriveMode(res470_DM_ALG_HIZ);
    res470_Write(0);
    
    res47k_SetDriveMode(res47k_DM_STRONG);
    res47k_Write(1); 
    
    int16 last_mvolts = -100;
    uint32 plateau_counter = 0;
    
    while(1) {
        ADC_DelSig_IsEndConversion(ADC_DelSig_RETURN_STATUS); 
        ADC_DelSig_GetResult32(); 
        while(ADC_DelSig_IsEndConversion(ADC_DelSig_WAIT_FOR_RESULT) == 0);
        int16 mvolts = ADC_DelSig_CountsTo_mVolts(ADC_DelSig_GetResult32());
        
        if(mvolts >= 3295) break; 
        
        // 2mV noise tolerance 
        int16 diff = mvolts - last_mvolts;
        if (diff >= -2 && diff <= 2) {
            plateau_counter++;
            if (plateau_counter > 500) {
                break; // plateau found!
            }
        } else {
            plateau_counter = 0; 
            last_mvolts = mvolts;
        }
    }
    
    CyDelay(10); 

    ADC_DelSig_IsEndConversion(ADC_DelSig_RETURN_STATUS); 
    ADC_DelSig_GetResult32();
    while(ADC_DelSig_IsEndConversion(ADC_DelSig_WAIT_FOR_RESULT) == 0);
    int16 final_mvolts = ADC_DelSig_CountsTo_mVolts(ADC_DelSig_GetResult32());

    res47k_SetDriveMode(res47k_DM_ALG_HIZ);
    res47k_Write(0);

    if (final_mvolts >= 3290) {
        return 0xFFFFFFFF; // infinite resistance 
    } 
    
    return (uint32) 47000 * final_mvolts / (3300 - final_mvolts); 
}

uint32 dielectric_absorption(void) {
    discharge(); 
    
    // ==========================================
    // phase 1: fast charge using 470 ohms
    // ==========================================
    res47k_SetDriveMode(res47k_DM_ALG_HIZ);
    res47k_Write(0);
    
    res470_SetDriveMode(res470_DM_STRONG);
    res470_Write(1); 
    
    uint32 timeout = 0;
    while(1) {
        ADC_DelSig_IsEndConversion(ADC_DelSig_RETURN_STATUS); 
        ADC_DelSig_GetResult32(); 
        while(ADC_DelSig_IsEndConversion(ADC_DelSig_WAIT_FOR_RESULT) == 0);
        int16 mvolts = ADC_DelSig_CountsTo_mVolts(ADC_DelSig_GetResult32());
        
        // Stop fast charging when we get close to the top
        if(mvolts >= 3295) break; 
        
        timeout++;
        if (timeout > 50000) break; // 5-second safety timeout
    }
    
    // let it soak at 3.3V
    CyDelay(2000);
    
    // ==========================================
    // phase 2: short circuit it then disconnect everything
    // ==========================================
    discharge(); 
    
    res47k_SetDriveMode(res47k_DM_ALG_HIZ);
    res47k_Write(0);
    res470_SetDriveMode(res470_DM_ALG_HIZ);
    res470_Write(0);
    
    CyDelay(4000);
    
    // ==========================================
    // phase 4: measure
    // ==========================================
    ADC_DelSig_IsEndConversion(ADC_DelSig_RETURN_STATUS); 
    ADC_DelSig_GetResult32(); 
    while(ADC_DelSig_IsEndConversion(ADC_DelSig_WAIT_FOR_RESULT) == 0);
    int16 new_volts = ADC_DelSig_CountsTo_mVolts(ADC_DelSig_GetResult32());
    
    if (new_volts < 0) new_volts = 0;
    
    // Returning the recovered millivolts
    return (uint32)new_volts;
}

#include <stdio.h> // Required for sprintf

// Scale Capacitance (Base unit: nanoFarads)
void print_scaled_capacitance(uint32 nf, uint16 x, uint16 y) {
    char str[16];
    LCD_print(x, y, "          ", COLOR_BLACK, COLOR_BLACK, 2); // Clear old value
    
    if (nf >= 1000000) {
        // milliFarads (mF)
        sprintf(str, "%lu.%02lu mF", nf / 1000000, (nf % 1000000) / 10000);
    } else if (nf >= 1000) {
        // microFarads (uF)
        sprintf(str, "%lu.%02lu uF", nf / 1000, (nf % 1000) / 10);
    } else {
        // nanoFarads (nF)
        sprintf(str, "%lu nF", nf);
    }
    LCD_print(x, y, str, COLOR_NEON_GREEN, COLOR_BLACK, 2);
}

// Scale ESR (Base unit: milliOhms)
void print_scaled_esr(uint16 mohm, uint16 x, uint16 y) {
    char str[16];
    LCD_print(x, y, "          ", COLOR_BLACK, COLOR_BLACK, 2); 
    
    if (mohm >= 1000) {
        // Ohms
        sprintf(str, "%u.%02u Ohm", mohm / 1000, (mohm % 1000) / 10);
    } else {
        // milliOhms
        sprintf(str, "%u mOhm", mohm);
    }
    LCD_print(x, y, str, COLOR_NEON_GREEN, COLOR_BLACK, 2);
}

// Scale Resistance (Base unit: Ohms)
void print_scaled_resistance(uint32 ohms, uint16 x, uint16 y) {
    char str[16];
    LCD_print(x, y, "          ", COLOR_BLACK, COLOR_BLACK, 2);
    
    if (ohms == 0xFFFFFFFF) { // Your infinite resistance flag
        sprintf(str, "OPEN");
    } else if (ohms >= 1000000) {
        // MegaOhms
        sprintf(str, "%lu.%02lu MOhm", ohms / 1000000, (ohms % 1000000) / 10000);
    } else if (ohms >= 1000) {
        // KiloOhms
        sprintf(str, "%lu.%02lu kOhm", ohms / 1000, (ohms % 1000) / 10);
    } else {
        // Ohms
        sprintf(str, "%lu Ohm", ohms);
    }
    LCD_print(x, y, str, COLOR_NEON_GREEN, COLOR_BLACK, 2);
}

// Scale Voltage (Base unit: milliVolts)
void print_scaled_voltage(uint32 mv, uint16 x, uint16 y) {
    char str[16];
    LCD_print(x, y, "          ", COLOR_BLACK, COLOR_BLACK, 2);
    
    if (mv >= 1000) {
        // Volts
        sprintf(str, "%lu.%02lu V", mv / 1000, (mv % 1000) / 10);
    } else {
        // milliVolts
        sprintf(str, "%lu mV", mv);
    }
    LCD_print(x, y, str, COLOR_NEON_GREEN, COLOR_BLACK, 2);
}

void capacitor(void) {
    LCD_print(40, 0x5, "Capacitors", COLOR_ICE_BLUE, COLOR_BLACK, 4);
    LCD_print(15, 60, "Results are below!", COLOR_WHITE, COLOR_BLACK, 2); 
    LCD_draw_capacitor(280, 65, COLOR_WHITE);
    
    // Removed the hardcoded units from the labels
    LCD_print(10, 100, "Capacitance:", COLOR_WHITE, COLOR_BLACK, 2);
    LCD_print(10, 120, "ESR:", COLOR_WHITE, COLOR_BLACK, 2);
    LCD_print(10, 140, "Leakage R:", COLOR_WHITE, COLOR_BLACK, 2); 
    LCD_print(10, 160, "Dielec. Abs:", COLOR_WHITE, COLOR_BLACK, 2); 
    
    
    LCD_print(16, 200, "Voltage vs. Time", COLOR_ICE_BLUE, COLOR_BLACK, 3);
    
    // Draw placeholders
    LCD_print(180, 100, "...", COLOR_WHITE, COLOR_BLACK, 2);
    LCD_print(180, 120, "...", COLOR_WHITE, COLOR_BLACK, 2);
    LCD_print(180, 140, "...", COLOR_WHITE, COLOR_BLACK, 2);
    LCD_print(180, 160, "...", COLOR_WHITE, COLOR_BLACK, 2);
    
    // ==== CAPACITANCE TEST ====
    uint32 cap = capacitor_test(); 
    print_scaled_capacitance(cap, 180, 100);
    
    // ==== ESR TEST ====
    uint16 esr = esr_test(); 
    print_scaled_esr(esr, 180, 120);
    
    // ==== LEAKAGE RESISTANCE TEST ====
    uint32 leakage_R = leakage_current(); 
    print_scaled_resistance(leakage_R, 180, 140);
    
    // ==== DIELECTRIC ABSORPTION TEST ==== 
    uint32 da = dielectric_absorption();
    print_scaled_voltage(da, 180, 160);
    
    discharge();
}