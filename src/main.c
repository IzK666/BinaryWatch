/*
 
 Binary Watch
  
 Based on "A Bit Different"
 Created with cloudpebble.net
 */

#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"


#define MY_UUID { 0x11, 0x28, 0xB6, 0x9D, 0x9F, 0xBA, 0x48, 0x27, 0x8B, 0x18, 0xD2, 0x47, 0x74, 0x49, 0x7A, 0x4B }
PBL_APP_INFO(MY_UUID,
             "Binary Watch", "IzK",
             1, 0, /* App version */
             DEFAULT_MENU_ICON,
             APP_INFO_WATCH_FACE);

Window window;
Layer display_layer;
TextLayer Day_layer;
TextLayer Month_layer;


static char DayText[] = "     ";
static char MonthText[] = " /    ";

#define CIRCLE_RADIUS 10
#define CIRCLE_LINE_THICKNESS 2

void draw_cell(GContext* ctx, GPoint center, bool filled) {
        // Each "cell" represents a binary digit or 0 or 1.
        
        graphics_context_set_fill_color(ctx, GColorWhite);
        
        graphics_fill_circle(ctx, center, CIRCLE_RADIUS);
        
        if (!filled) {
                // This is our ghetto way of drawing circles with a line thickness
                // of more than a single pixel.
                graphics_context_set_fill_color(ctx, GColorBlack);
                
                graphics_fill_circle(ctx, center, CIRCLE_RADIUS - CIRCLE_LINE_THICKNESS);
        }
        
}

#define CELLS_PER_ROW 2
#define CELLS_PER_COLUMN 6

#define CIRCLE_PADDING 12 - CIRCLE_RADIUS // Number of padding pixels on each side
#define TOP_PADDING 60

GPoint get_center_hour(unsigned short x)
{
	// Cell location (0) is left. location (4) is right.
	int cell = 2 * (CIRCLE_RADIUS + CIRCLE_PADDING); // One "cell" is the square that contains the circle.
	if (clock_is_24h_style())
	{
		cell += 4;
		return GPoint((cell/2) + (cell * x) + 2, TOP_PADDING + (cell/2));
	}
	cell += 10;
	return GPoint((cell/2) + (cell * x) + 4, TOP_PADDING + (cell/2));
}

GPoint get_center_min(unsigned short x)
{
	// Cell location (0) is left. location (5) is right.
	int cell = 2 * (CIRCLE_RADIUS + CIRCLE_PADDING); // One "cell" is the square that contains the circle.
	return GPoint((cell/2) + (cell * x), TOP_PADDING + (cell*2) + (cell/2));
}

void draw_cell_hour(GContext* ctx, unsigned short digit)
{
	// Converts the supplied decimal hour into binary form and
	// then draws the first row of cells on screen--'1' binary values are filled, 
	// '0' binary values are not filled.
	
	// max sets the numbers of cells depending on the format's clock.
	int max = (clock_is_24h_style())?5:4;
	for (int cell_row_index = 0; cell_row_index < max; cell_row_index++)
	{
		draw_cell(ctx, get_center_hour(max - cell_row_index - 1), (digit >> (cell_row_index)) & 0x1);
	}
}

void draw_cell_min(GContext* ctx, unsigned short digit)
{
	// Converts the supplied decimal minutes into binary form and
	// then draws the second row of cells on screen--'1' binary values are filled, 
	// '0' binary values are not filled.
	
	// The number of cells are fixed: 6.
	for (int cell_row_index = 0; cell_row_index < 6; cell_row_index++)
	{
		draw_cell(ctx, get_center_min(5 - cell_row_index), (digit >> (cell_row_index)) & 0x1);
	}
}

unsigned short get_display_hour(unsigned short hour) 
{
	if (clock_is_24h_style())
	{
		return hour;
	}
      
	// convert 24hr to 12hr
	unsigned short display_hour = hour % 12;
	// Converts "0" to "12"
	return display_hour ? display_hour : 12;
}


void display_layer_update_callback(Layer *me, GContext* ctx) {
	(void)me;

	PblTm t;
	get_time(&t);

	unsigned short display_hour = get_display_hour(t.tm_hour);

	draw_cell_hour(ctx, display_hour);
	draw_cell_min(ctx, t.tm_min);
}

void update_watchface()
{
	PblTm currentTime;
	get_time(&currentTime);

	int mon=currentTime.tm_mon + 1; // The month is zero-based.
	int day=currentTime.tm_mday;
	int bin[5]={0,1,3,7,15};

	for (int i = 3; i >=0; i--)
	{
		MonthText[5-i] = ((mon >> (i)) & 0x1)?'1':'0';
	}

	for (int i = 4; i >=0; i--)
	{
		if (day > bin[i]) // It avoids writing the most-left 0 of the binary day. Ex: 101 instead 00101.
		{
			DayText[4-i] = ((day >> (i)) & 0x1)?'1':'0';
		}
	}
	text_layer_set_text(&Day_layer, DayText);
	text_layer_set_text(&Month_layer, MonthText);
}


void handle_init(AppContextRef ctx) {
        // initializing app
        
        (void)ctx;

        window_init(&window, "BinaryWatch");
        window_stack_push(&window, true /* Animated */);
        window_set_background_color(&window, GColorBlack);
        
        // init the bit layer
        layer_init(&display_layer, window.layer.frame);
        display_layer.update_proc = &display_layer_update_callback; // REF: .update_proc points to a function that draws the layer
        layer_add_child(&window.layer, &display_layer);
        
        
        resource_init_current_app(&APP_RESOURCES);

        // init the date layer
        text_layer_init(&Day_layer, GRect(20, 0, 69, TOP_PADDING));
        text_layer_set_font(&Day_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_GOTHAM_BOLD_18)));
        text_layer_set_text_color(&Day_layer, GColorWhite);
        text_layer_set_background_color(&Day_layer, GColorClear);
		text_layer_set_text_alignment(&Day_layer, GTextAlignmentRight);
        layer_add_child(&window.layer, &Day_layer.layer);

        // init the date layer
        text_layer_init(&Month_layer, GRect(90, 0, 59, TOP_PADDING));
        text_layer_set_font(&Month_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_GOTHAM_LIGHT_18)));
        text_layer_set_text_color(&Month_layer, GColorWhite);
        text_layer_set_background_color(&Month_layer, GColorClear);
        layer_add_child(&window.layer, &Month_layer.layer);

        // load watchface immediately
        update_watchface();
}


void handle_tick(AppContextRef ctx, PebbleTickEvent *t) {
        // doing something on the second
        
        (void)ctx;
        update_watchface();
}


void pbl_main(void *params) {
  PebbleAppHandlers handlers = {
    .init_handler = &handle_init,
          .tick_info = {
                  .tick_handler = &handle_tick,
                  .tick_units = MINUTE_UNIT
          }
  };
  app_event_loop(params, &handlers);
}