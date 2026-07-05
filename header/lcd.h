#ifndef LCD_H
#define LCD_H

    #include <project.h>

    // ==========================================
    // 16-BIT RGB565 COLOR PALETTE
    // ==========================================

    // Standard Primary Colors 
    #define COLOR_BLACK         0x0000
    #define COLOR_WHITE         0xFFFF
    #define COLOR_RED           0xF800
    #define COLOR_GREEN         0x07E0
    #define COLOR_BLUE          0x001F

    // High-Contrast Neons (Best for drawing graphs, lines, and borders) 
    #define COLOR_CYAN          0x07FF
    #define COLOR_YELLOW        0xFFE0
    #define COLOR_MAGENTA       0xF81F
    #define COLOR_ORANGE        0xFD20
    #define COLOR_NEON_PINK     0xF8B2
    #define COLOR_ELECTRIC_BLUE 0x043F
    #define COLOR_NEON_GREEN    0x07EF

    //  Pastels 
    #define COLOR_SKYBLUE       0x867D
    #define COLOR_MINT          0x97F2
    #define COLOR_PEACH         0xFBEA
    #define COLOR_LAVENDER      0xE73F
    #define COLOR_LIGHT_PINK    0xFDF9
    #define COLOR_PALE_YELLOW   0xFFF0
    #define COLOR_ICE_BLUE      0xAEDC

    //  UI & System States 
    #define COLOR_ERROR         0xF800  
    #define COLOR_WARNING       0xFDF0  
    #define COLOR_SUCCESS       0x07E0  
    #define COLOR_INFO          0x07FF 

    //  Grayscales (Best for disabled buttons, shadows, and background grids) 
    #define COLOR_DARK_GRAY     0x4A49
    #define COLOR_GRAY          0x8410
    #define COLOR_LIGHT_GRAY    0xC618
    #define COLOR_SILVER        0xE71C
        
    #define LCD_WIDTH  320
    #define LCD_HEIGHT 480
        
    #define X_ORIGIN 20
    #define Y_ORIGIN 465

    #define ABS(x) ((x) > 0 ? (x) : -(x))
        
    /* Function Prototypes (The Menu) */
    void LCD_Init(void);
    void LCD_FillScreen(uint16 color);
    void LCD_write_cmd(uint8 cmd); 
    void LCD_write_data(uint8 data);
    void LCD_set_window (uint16 x0, uint16 y0, uint16 x1, uint16 y1);
    void LCD_fill_screen (uint16 color); 
    void LCD_init(void);
    void LCD_draw_pixel(uint16 x, uint16 y, uint16 color);
    void LCD_write_pixel_fast(uint16 x, uint16 y, uint16 color);
    void LCD_draw_rect(uint16 x0, uint16 y0, uint16 x1, uint16 y1, uint16 color); 
    void LCD_draw_char(uint16 x, uint16 y, char c, uint16 color, uint16 background, uint8 size);
    void LCD_print(uint16 x, uint16 y, char str[], uint16 color, uint16 background, uint8 size);
    void LCD_welcome(void);
    void update_analysis(uint8 choice);
    void LCD_draw_axes(void); 
    void LCD_draw_line(uint16 x0, uint16 y0, uint16 x1, uint16 y1, uint16 color);
    void LCD_graph(uint16 *volts, uint16 *amps, uint32 num_points); 
    void LCD_draw_capacitor(uint16 x, uint16 y, uint16 color);
    void LCD_draw_resistor(uint16 x, uint16 y, uint16 color);
    void LCD_draw_inductor(uint16 x, uint16 y, uint16 color);
    void LCD_draw_diode(uint16 x, uint16 y, uint16 color);
    void LCD_clear_menu(void);
    void LCD_clear_resistor(void);
    void LCD_clear_capacitor(void);
    void LCD_clear_inductor(void);
    void LCD_clear_diode(void);

#endif 