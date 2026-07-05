/* ========================================
 * inductor tester
 * ========================================
*/

#include <ind.h>
#include <lcd.h>
#include <helper.h>
#include <stdio.h>

uint32 inductor_test(uint8 *ring_count, uint32* time) {
    // ensure resistive pins are open 
    res470_SetDriveMode(res470_DM_ALG_HIZ); 
    res470_Write(0);
    res47k_SetDriveMode(res47k_DM_ALG_HIZ);
    res47k_Write(0);
    
    // send impulse for 5 us
    INDUCTOR_DRIVER_SetDriveMode(INDUCTOR_DRIVER_DM_STRONG);
    INDUCTOR_DRIVER_Write(1);
    CyDelayUs(5); // give it a few microseconds to build up magnetic field
    
    // clear the FIFO right before releasing the driver
    // this prevents the initial voltage spike from registering as a valid bounce
    Timer_1_ClearFIFO(); 
    INDUCTOR_DRIVER_SetDriveMode(INDUCTOR_DRIVER_DM_ALG_HIZ);
    INDUCTOR_DRIVER_Write(0);
    
    uint32 t1 = 0;
    uint32 t2 = 0;
    uint16 timeout = 10000;
    uint8 internal_rings = 0;
    
    // wait for the first capture
    // i.e., FIFO not empty
    while(!(Timer_1_ReadStatusRegister() & Timer_1_STATUS_FIFONEMP) && timeout) {
        timeout--;
    }
    if(timeout > 0) {
        t1 = Timer_1_ReadCapture();
        internal_rings++;
    }
    
    //wait for second capture
    while(!(Timer_1_ReadStatusRegister() & Timer_1_STATUS_FIFONEMP) && timeout) {
        timeout--;
    }
    if(timeout > 0) {
        t2 = Timer_1_ReadCapture();
        internal_rings++;
    }
    
    // ensure we didnt time out and that t1 is greater than t2 (timer counts down)
    if (timeout == 0 || t1 <= t2) {
        *ring_count = 0;
        *time = 0;
        return 0;
    }
    
    uint16 decay_timeout = 5000; 
    
    while(decay_timeout > 0) {
        // If we catch another ring...
        if(Timer_1_ReadStatusRegister() & Timer_1_STATUS_FIFONEMP) {
            Timer_1_ReadCapture(); // Read it to clear it out
            internal_rings++;
            decay_timeout = 5000;  // Reset the clock! Listen for the next one.
        } else {
            decay_timeout--;       // No ring yet, keep counting down...
        }
    }
    
    *ring_count = internal_rings;
    
    uint32 period_ticks = t1 - t2;
    *time = period_ticks;
    
    return (uint64) period_ticks * period_ticks * CONST_L / C_REF;
}

void inductor_print(uint32 inductance) {
    char str[16];
    
    if(inductance > 1000000000) {
        sprintf(str, "%lu.%02lu mH", (inductance / 1000000000), (inductance % 1000000000) / 10000000);
    } else if(inductance > 1000000) {
        sprintf(str, "%lu.%02lu uH", (inductance / 1000000), (inductance % 1000000) / 10000);
    } else {
        sprintf(str, "%lu", inductance);
    }
    
    LCD_print(180, 100, str, COLOR_ICE_BLUE, COLOR_BLACK, 2);
}

void LCD_draw_LC_tank(uint16 center_x, uint16 center_y, uint16 color) {
    // draw the top branch (Inductor)
    // Offset by -30 pixels on the Y axis
    LCD_draw_inductor(center_x, center_y - 37, color);

    // draw the bottom branch (Capacitor)
    // offset by +30 pixels on the Y axis
    LCD_draw_capacitor(center_x, center_y + 30, color);

    // extend the inductor wires out a bit further to line up
    LCD_draw_line(center_x - 35, center_y - 30, center_x - 25, center_y - 30, color);
    LCD_draw_line(center_x + 25, center_y - 30, center_x + 35, center_y - 30, color);

    // extend the capacitor wires out to line up with the inductor wires
    LCD_draw_line(center_x - 35, center_y + 30, center_x - 20, center_y + 30, color);
    LCD_draw_line(center_x + 20, center_y + 30, center_x + 35, center_y + 30, color);

    // draw the left and right vertical parallel connecting wires
    LCD_draw_line(center_x - 35, center_y - 30, center_x - 35, center_y + 30, color);
    LCD_draw_line(center_x + 35, center_y - 30, center_x + 35, center_y + 30, color);

    // draw the input/output probe lines coming from the middle 
    LCD_draw_line(center_x - 60, center_y, center_x - 35, center_y, color);
    LCD_draw_line(center_x + 35, center_y, center_x + 60, center_y, color);

    // draw junction connection dots (small 3x3 filled rectangles)
    LCD_draw_rect(center_x - 36, center_y - 1, center_x - 34, center_y + 1, color);
    LCD_draw_rect(center_x + 34, center_y - 1, center_x + 36, center_y + 1, color);

    LCD_print(center_x - 5, center_y - 65, "L", COLOR_YELLOW, COLOR_BLACK, 2);
    LCD_print(center_x - 5, center_y + 55, "C", COLOR_YELLOW, COLOR_BLACK, 2);
    
  }

void inductor(void) {
    LCD_print(52, 5, "Inductors", COLOR_SUCCESS, COLOR_BLACK, 4);
    LCD_print(15, 60, "Results are below!", COLOR_WHITE, COLOR_BLACK, 2); 
    LCD_draw_inductor(280, 65, COLOR_WHITE);
    LCD_print(10, 100, "Inductance:", COLOR_WHITE, COLOR_BLACK, 2);
    LCD_print(180, 100, "...", COLOR_RED, COLOR_BLACK, 2);
    LCD_print(10, 120, "Ring Count:", COLOR_WHITE, COLOR_BLACK, 2);
    LCD_print(180, 120, "...", COLOR_RED, COLOR_BLACK, 2);
    LCD_print(10, 140, "Ref Cap:", COLOR_WHITE, COLOR_BLACK, 2);
    LCD_print(180, 140, "47 nF", COLOR_ICE_BLUE, COLOR_BLACK, 2);
    LCD_print(10, 160, "Ticks:", COLOR_WHITE, COLOR_BLACK, 2);
    LCD_print(180, 160, "...", COLOR_RED, COLOR_BLACK, 2);
    
    uint8 rings = 0;
    uint32 time1 = 0;

    uint32 LnH = inductor_test(&rings, &time1);
    
    LCD_print(195, 100, "                 ", COLOR_BLACK, COLOR_BLACK, 2);
    inductor_print(LnH);
    
    LCD_print(180, 120, "                 ", COLOR_BLACK, COLOR_BLACK, 2);
    LCD_print(180, 160, "                 ", COLOR_BLACK, COLOR_BLACK, 2);
    
    // print rings
    char str2[12]; 
    uint32ToString(rings, str2);
    LCD_print(180, 120, str2, COLOR_ICE_BLUE, COLOR_BLACK, 2);
    
    //print time ticks
    char str3[12]; 
    uint32ToString(time1, str3);
    LCD_print(180, 160, str3, COLOR_ICE_BLUE, COLOR_BLACK, 2);
    
    LCD_draw_LC_tank(160, 300, COLOR_WHITE);
}