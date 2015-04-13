#include <pebble.h>

static time_t start_time;
static time_t end_time;
  
static Window *window;
static TextLayer *time_layer;
static TextLayer *left_time_layer;
static TextLayer *left_time_layer2;
static TextLayer *percent_layer;
static Layer *progress_layer;

static float percent = 0;

#define MSG_START_TIME 0
#define MSG_END_TIME 1
  
static void fucking_russian(int number, char* text, char* def, char* ending1, char* ending234)
{
  if (number >= 10 && number <= 19)
    strcat(text, def);
  else
    switch (number % 10)
    {
      case 0:
      case 5:
      case 6:
      case 7:
      case 8:
      case 9:
        strcat(text, def);
        break;
      case 1:
        strcat(text, ending1);
        break;
      case 2:
      case 3:
      case 4:
        strcat(text, ending234);        
        break;
    }
}

static void handle_tick(struct tm *tick_time, TimeUnits units_changed)
{
  time_t now = time(NULL);
  
  long int left_total = end_time - now;
  if (left_total < 0) left_total *= -1;  
  int left_mins = (left_total / 60) % 60;
  int left_hours = (left_total / (60*60)) % 24;
  int left_days = left_total / (60*60*24);
  
  static char left_time_str[200];
  char minut[15];
  char chas[15];
  char den[15];
  
  strcpy(minut, "минут");
  strcpy(chas, "час");
  strcpy(den, "д");  
  if (end_time > now)
    fucking_russian(left_mins, minut, "", "а", "ы");
  else
    fucking_russian(left_mins, minut, "", "у", "ы");
  fucking_russian(left_hours, chas, "ов", "", "а");
  fucking_russian(left_days, den, "ней", "ень", "ня");

  if (end_time > now)
    text_layer_set_text(left_time_layer, "До дембеля осталось");
  else
    text_layer_set_text(left_time_layer, "Ты дембель уже");
  snprintf(left_time_str, sizeof(left_time_str), "%d %s,\n%d %s\nи %d %s", 
          left_days, den, left_hours, chas, left_mins, minut);
  text_layer_set_text(left_time_layer2, left_time_str);   
  
  percent = 100.0 * (now - start_time) / (end_time - start_time);  
  if (percent > 100) percent = 100;  
  if (percent < 0) percent = 0;
  static char percent_str[200];
  snprintf(percent_str, sizeof(percent_str), "Ты отслужил %d.%02d%%", (int)percent, (int)(percent*100)%100);
  text_layer_set_text(percent_layer, percent_str);
  layer_mark_dirty(progress_layer);
  
  static char time_str[20];
  if (clock_is_24h_style())
    strftime(time_str, sizeof(time_str), "%H:%M", tick_time);
  else
    strftime(time_str, sizeof(time_str), "%I:%M", tick_time);
  text_layer_set_text(time_layer, time_str);
}

static void progress_layer_update_callback(Layer *layer, GContext *ctx)
{
  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_context_set_fill_color(ctx, GColorBlack);
  GRect rect = layer_get_bounds(layer);
  graphics_draw_round_rect(ctx, rect, 0);
  rect.origin.x += 2;
  rect.origin.y += 2;
  rect.size.h -= 4;
  rect.size.w = (rect.size.w-4)*percent / 100;
  graphics_fill_rect(ctx, rect, 0, GCornerNone);
}

static void in_received_handler(DictionaryIterator *received, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Received config");
  Tuple *tuple = dict_find(received, MSG_START_TIME);
  if (tuple) {
    start_time = tuple->value->int32;
    persist_write_int(MSG_START_TIME, start_time);
  }
  tuple = dict_find(received, MSG_END_TIME);
  if (tuple) {
    end_time = tuple->value->int32;
    persist_write_int(MSG_END_TIME, end_time);
  }
  time_t now = time(NULL);
  handle_tick(localtime(&now), 0);
}

static void handle_init(void) {
  struct tm tick_time;
  tick_time.tm_year = 2014 - 1900;
  tick_time.tm_mon = 12 - 1;
  tick_time.tm_mday = 9;
  tick_time.tm_hour = 12 - 1;
  tick_time.tm_min = 0;
  tick_time.tm_sec = 0;
  start_time = mktime(&tick_time);
  tick_time.tm_year = 2015 - 1900;
  end_time = mktime(&tick_time);

  // Read stored settings
  if (persist_exists(MSG_START_TIME))
    start_time = persist_read_int(MSG_START_TIME);
  if (persist_exists(MSG_END_TIME))
    end_time = persist_read_int(MSG_END_TIME);

  // Open communication channel
  app_message_register_inbox_received(in_received_handler);
  app_message_open(64, 64);
  
	// Create a window and text layer
	window = window_create();

  // Current time layer
  time_layer = text_layer_create(GRect(0, 0, 144, 50));	
	// Set the text, font, and text alignment
	text_layer_set_font(time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_background_color(time_layer, GColorClear);
	text_layer_set_text_alignment(time_layer, GTextAlignmentCenter);	
	// Add the text layer to the window
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(time_layer));

  // Left time layer
  left_time_layer = text_layer_create(GRect(0, 47, 144, 80));	
	// Set the text, font, and text alignment
	text_layer_set_font(left_time_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  text_layer_set_background_color(left_time_layer, GColorClear);
	text_layer_set_text_alignment(left_time_layer, GTextAlignmentCenter);	
	// Add the text layer to the window
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(left_time_layer));

  // Another Left time layer
  left_time_layer2 = text_layer_create(GRect(0, 67, 144, 80));	
	// Set the text, font, and text alignment
	text_layer_set_font(left_time_layer2, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_background_color(left_time_layer2, GColorClear);
	text_layer_set_text_alignment(left_time_layer2, GTextAlignmentCenter);	
	// Add the text layer to the window
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(left_time_layer2));

  // Percent layer
  percent_layer = text_layer_create(GRect(0, 130, 144, 80));	
	// Set the text, font, and text alignment
	text_layer_set_font(percent_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD));
  text_layer_set_background_color(percent_layer, GColorClear);
	text_layer_set_text_alignment(percent_layer, GTextAlignmentCenter);	
	// Add the text layer to the window
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(percent_layer));

  // Create progress bar
  progress_layer = layer_create(GRect(2, 150, 140, 15));
  layer_set_update_proc(progress_layer, &progress_layer_update_callback);
  layer_add_child(window_get_root_layer(window), progress_layer);
  
  time_t now = time(NULL);
  handle_tick(localtime(&now), 0);
  
	// Push the window
	window_stack_push(window, true);

  // Subscribe to time updates
  tick_timer_service_subscribe(MINUTE_UNIT, handle_tick); 
  
  //light_enable(true); // For taking photos only!
}

static void handle_deinit(void) {
  tick_timer_service_unsubscribe();
  
	// Destroy layers
	text_layer_destroy(time_layer);
  text_layer_destroy(left_time_layer);
  text_layer_destroy(left_time_layer2);
  text_layer_destroy(percent_layer);
  layer_destroy(progress_layer);
	
	// Destroy the window
	window_destroy(window);
}

int main(void) {
	handle_init();
	app_event_loop();
	handle_deinit();
}
