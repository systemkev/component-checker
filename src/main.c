/* ========================================
component checker
 * ========================================
*/

#include <project.h>
#include <lcd.h>
#include <res.h>
#include <helper.h>
#include <cap.h>
#include <diode.h>
#include <ind.h>
#include <volt.h>

// finite state machine
typedef enum {
    STATE_MENU,
    STATE_RESISTOR,
    STATE_RESISTOR_WAIT,
    STATE_CAPACITOR,
    STATE_CAPACITOR_WAIT,
    STATE_INDUCTOR,
    STATE_INDUCTOR_WAIT,
    STATE_DIODE,
    STATE_DIODE_WAIT,
    STATE_SCOPE,
    STATE_SCOPE_WAIT
} SystemState;

int main()
{
    CyGlobalIntEnable;
    
    // start SPI hardware 
    SPIM_1_Start();
        
    // start ADC conversion
    ADC_DelSig_Start();
    ADC_DelSig_StartConvert();
    
    ADC_pot_Start();
    ADC_pot_StartConvert();
    
    // start comparator and counter
    Comp_1_Start();
    Timer_1_Start();
    
    // start IDAC
    IDAC8_Start();
    
    // set initial idle states for the control pins 
    CS_Write(1); 
    DC_Write(1);
    
    // initialize the LCD
    LCD_init();
    LCD_fill_screen(COLOR_BLACK);
    
    SystemState current_state = STATE_MENU; 
    uint8 press_button_1;
    uint8 press_button_2;
    
    for(;;){
        switch(current_state) {
            case STATE_MENU:
                LCD_welcome();
                uint8 selection = choose_analysis();
                
                if (selection == 0) current_state = STATE_RESISTOR;
                if (selection == 1) current_state = STATE_CAPACITOR;
                if (selection == 2) current_state = STATE_INDUCTOR;
                if (selection == 3) current_state = STATE_DIODE;
                if (selection == 4) current_state = STATE_SCOPE;
                
                LCD_clear_menu();
                
                break;
            
            case STATE_RESISTOR: 
                resistor();
                current_state = STATE_RESISTOR_WAIT;
                break; 
            
            case STATE_RESISTOR_WAIT: 
                press_button_1 = BUTTON1_Read();
                press_button_2 = BUTTON2_Read();
                
                if(press_button_1 == 0) {
                    // debounce
                    while(BUTTON1_Read() == 0); 
                    
                    current_state = STATE_MENU;
                    LCD_fill_screen(COLOR_BLACK);
                    
                } else if (press_button_2 == 0) {
                    // debounce
                    while(BUTTON2_Read() == 0);
                    
                    // loop to the run state
                    current_state = STATE_RESISTOR;
                    LCD_fill_screen(COLOR_BLACK);
                }
                
                break;
            
            case STATE_CAPACITOR: 
                capacitor();
                current_state = STATE_CAPACITOR_WAIT;
                break;
               
            case STATE_CAPACITOR_WAIT:
                press_button_1 = BUTTON1_Read();
                press_button_2 = BUTTON2_Read();
                
                if(press_button_1 == 0) {
                    // debounce
                    while(BUTTON1_Read() == 0); 
                    
                    current_state = STATE_MENU;
                    LCD_fill_screen(COLOR_BLACK);
                    
                } else if (press_button_2 == 0) {
                    // debounce
                    while(BUTTON2_Read() == 0);
                    
                    // loop to the run state
                    current_state = STATE_CAPACITOR;
                    LCD_fill_screen(COLOR_BLACK);
                }
                break;
            
            case STATE_INDUCTOR: 
                inductor();
                current_state = STATE_INDUCTOR_WAIT;
                break;
            
            case STATE_INDUCTOR_WAIT:
                press_button_1 = BUTTON1_Read();
                press_button_2 = BUTTON2_Read();
                
                if(press_button_1 == 0) {
                    // debounce
                    while(BUTTON1_Read() == 0); 
                    
                    current_state = STATE_MENU;
                    LCD_fill_screen(COLOR_BLACK);
                    
                } else if (press_button_2 == 0) {
                    // debounce
                    while(BUTTON2_Read() == 0);
                    
                    // loop to the run state
                    current_state = STATE_INDUCTOR;
                    LCD_fill_screen(COLOR_BLACK);
                }
                break;
            
            case STATE_DIODE: 
                diode();
                current_state = STATE_DIODE_WAIT;
                break; 
                
            case STATE_DIODE_WAIT:
                press_button_1 = BUTTON1_Read();
                press_button_2 = BUTTON2_Read();
                
                if(press_button_1 == 0) {
                    // debounce
                    while(BUTTON1_Read() == 0); 
                    
                    current_state = STATE_MENU;
                    LCD_fill_screen(COLOR_BLACK);
                    
                } else if (press_button_2 == 0) {
                    // debounce
                    while(BUTTON2_Read() == 0);
                    
                    // loop to the run state
                    current_state = STATE_DIODE;
                    LCD_fill_screen(COLOR_BLACK);
                }
                break;    
            case(STATE_SCOPE):
                scope();
                current_state = STATE_SCOPE_WAIT;
                break;
            case(STATE_SCOPE_WAIT):
                press_button_1 = BUTTON1_Read();
                press_button_2 = BUTTON2_Read();
                
                if(press_button_1 == 0) {
                    // debounce
                    while(BUTTON1_Read() == 0); 
                    
                    current_state = STATE_MENU;
                    LCD_fill_screen(COLOR_BLACK);
                    
                } else if (press_button_2 == 0) {
                    // debounce
                    while(BUTTON2_Read() == 0);
                    
                    // loop to the run state
                    current_state = STATE_SCOPE;
                    LCD_fill_screen(COLOR_BLACK);
                }
                break;
        }
    }
}