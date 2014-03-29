#include <pebble.h>

// constants
#define NONE 0
#define UP 1
#define DOWN 2
#define LEFT 3
#define RIGHT 4

// ui objects
static Window *window;
static TextLayer *text_layer;
static Layer *draw_layer_main; //main draw layer
static Layer *grid_layer; //layer for main game grid and cells
static TextLayer **grid_entries_text_layers; //array of TextLayers for grid cells
static TextLayer *score_display;
static TextLayer *message; // message

// stuff that I added
// grid stuff
int boundary_side = 34; //side length of each cell in grid
int grid_slen = 4; // number of cells per side of grid
int sentinel_value = -1; // sentinel value of cell
int goal = 2048; // 2048!
int * grid_entries; // array of grid cells as ints
char* grid_entries_text; // array of grid cells as strings
int grid_entry_text_len = 5; // string length of each grid cell string
struct GPoint grid_origin = {3, 3}; // origin of entire grid
struct GPoint text_stagger = {2, 4}; // origin of each TextLayer relative to each cell

// scoring stuff
int score = 0;
static int update_score(int dscore);
static void send_score_to_android();
/*BitmapLayer *win_layer;
BitmapLayer *lose_layer;*/
BitmapLayer *game_end_display;
GBitmap *win_img;
GBitmap *lose_img;

// accelerometer stuff
static AppTimer *timer;
int accel_sensitivity = 600;
int accel_interval = 250;

// other stuff
bool game_end = false;

static int pwr2(int pwr)
{
  return ((pwr == 0) ? 1 : (2 * pwr2(pwr - 1)));
}



static void update_cell(int index, int new_number)
{
  *(grid_entries + index) = new_number;

  if (new_number != -1)
    snprintf((grid_entries_text + (grid_entry_text_len*index)), grid_entry_text_len, "%d", (*(grid_entries + index)));
  else
    snprintf((grid_entries_text + (grid_entry_text_len*index)), grid_entry_text_len, " ");

  text_layer_set_text(*(grid_entries_text_layers + index), (grid_entries_text + (grid_entry_text_len*index)));

  return;
}

static void add_random_entry()
{
  int index = 0;
  do
  {
    index = (grid_slen * (rand() % grid_slen)) + (rand() % grid_slen);
  } 
  while (*(grid_entries + index) != sentinel_value);

  update_cell(index, pwr2((rand() % 2) + 1));


  return;

}

// draw_grid: draws a 4x4 grid onto screen
static void draw_grid(Layer *layer, GContext* ctx)
{

  graphics_draw_rect(ctx, (GRect){ .origin = grid_origin, 
    .size = { 4*boundary_side, 4*boundary_side }} );

  for (int y = 0; y < grid_slen; y++)
  {
    for (int x = 0; x < grid_slen; x++)
    {
      graphics_draw_rect(ctx, (GRect) {.origin = {grid_origin.x + (x*boundary_side), grid_origin.y + (y*boundary_side)}, .size = { boundary_side, boundary_side }} );
    }
  }

  return;
}

static void update_grid(int direction, bool is_auto)
{ 
  int num_changes_made = 0;
  int num_blanks = 0;


  int x1 = 0;
  int x2 = 0;
  int dx = 0;

  int y1 = 0;
  int y2 = 0;
  int dy = 0;

  if (direction == UP)
  {
    x1 = 0;
    x2 = grid_slen - 1;
    dx = 1;

    y1 = grid_slen - 1;
    y2 = 0;
    dy = -1;

  }
  else if (direction == DOWN)
  {
    x1 = 0;
    x2 = grid_slen - 1;
    dx = 1;

    y1 = grid_slen - 1;
    y2 = 0;
    dy = -1;

  }
  else if (direction == LEFT)
  {
    x1 = 0;
    x2 = grid_slen - 1;
    dx = 1;

    y1 = grid_slen - 1;
    y2 = 0;
    dy = -1;

  }
  else if (direction == RIGHT)
  {
    x1 = grid_slen - 1;
    x2 = 0;
    dx = -1;

    y1 = 0;
    y2 = grid_slen - 1;
    dy = 1;

  }

  for (int y = y1; y != y2 + dy; y += dy)
  {
    for (int x = x1; x != x2 + dx; x += dx)
    {
      int index = (grid_slen * y) + x;
      int this_entry = *(grid_entries + index);
      int compare_index = -1;
      int new_val = -1;

      if (this_entry == -1)
      {
        num_blanks++;
        continue;
      }

      if ((y > 0) && (direction == UP))
      {
        compare_index = (grid_slen * (y - 1)) + x;
      }
      else if ((y < grid_slen - 1) && (direction == DOWN))
      {
        compare_index = (grid_slen * (y + 1)) + x;
      }
      else if ((x > 0) && (direction == LEFT))
      {
        compare_index = (grid_slen * y) + (x - 1);
      }
      else if ((x < grid_slen - 1) && (direction == RIGHT))
      {
        compare_index = (grid_slen * y + (x + 1));
      }


      if (compare_index != -1)
      {
        if ((this_entry == *(grid_entries + compare_index)))
        {
          new_val = 2*this_entry;
          update_score(2*this_entry);

          if (new_val == goal)
          {
            // win!
            game_end = true;
            bitmap_layer_set_bitmap(game_end_display, win_img);
          }
        }
        else if (*(grid_entries + compare_index) == -1)
        {
          new_val = this_entry;
        }
      }

      /*
      if (this_entry == *(grid_entries + compare_index))
      {
        new_val = 2*this_entry;
      }
      else if (*(grid_entries + compare_index) == -1)
      {
        new_val = this_entry;
      }
      */

      if (new_val != -1)
      {
        update_cell(compare_index, new_val);
        update_cell(index, -1);
        num_changes_made++;
      }

    }
  }

  if (num_blanks == 0 && num_changes_made == 0)
  {
    //game over
    static const uint32_t const dur[] = { 250, 250, 500, 250, 2000 };
    VibePattern pat = { .durations = dur, .num_segments = 5 };
    //vibes_enqueue_custom_pattern(pat);

    game_end = true;
    bitmap_layer_set_bitmap(game_end_display, lose_img);
  }
  else if (num_changes_made != 0)
  {
    // check to push furthest
    bool push_further = false;
    if (direction == UP) //check top row
    {
      for (int i = 0; i < grid_slen; i++)
      {
        if (((*(grid_entries + i)) == -1) &&
            (((*(grid_entries + (grid_slen * 1) + i) != -1) ||
            ((*(grid_entries + (grid_slen * 2) + i) != -1)) ||
            ((*(grid_entries + (grid_slen * 3) + i) != -1)) )))
        {
          push_further = true;
          break;
        }
      }
    }
    else if (direction == DOWN) //check bottom row
    {
      for (int i = 0; i < grid_slen; i++)
      {
        if (((*(grid_entries + (grid_slen * 3) + i)) == -1) &&
            (((*(grid_entries + (grid_slen * 0) + i) != -1) ||
            ((*(grid_entries + (grid_slen * 1) + i) != -1)) ||
            ((*(grid_entries + (grid_slen * 2) + i) != -1)) )))
        {
          push_further = true;
          break;
        }
      }
    }
    else if (direction == LEFT) //check left column
    { 
      for (int i = 0; i < grid_slen; i++)
      {
        if (((*(grid_entries + (grid_slen * i) + 0)) == -1) &&
            (((*(grid_entries + (grid_slen * i) + 1) != -1) ||
            ((*(grid_entries + (grid_slen * i) + 2) != -1)) ||
            ((*(grid_entries + (grid_slen * i) + 3) != -1)) )))
        {
          push_further = true;
          break;
        }
      }
    }
    else if (direction == RIGHT) //check right column
    {
      for (int i = 0; i < grid_slen; i++)
      {
        if (((*(grid_entries + (grid_slen * i) + 3)) == -1) &&
            (((*(grid_entries + (grid_slen * i) + 0) != -1) ||
            ((*(grid_entries + (grid_slen * i) + 1) != -1)) ||
            ((*(grid_entries + (grid_slen * i) + 2) != -1)) )))
        {
          push_further = true;
          break;
        }
      }
    }

    if (push_further)
    {
      update_grid(direction, true);
      return;
    }

    // 
    add_random_entry();

  } 

  return;
}


// scoring stuff

static int update_score(int dscore)
{
  score += dscore;
  char* score_text = malloc(sizeof(char) * 10);
  snprintf(score_text, 10, "%d", score);
  text_layer_set_text(score_display, score_text);
  free (score_text);
  send_score_to_android();
  return 0;
}

static void send_score_to_android()
{

}



// functions that i didn't add

bool selectToggle = false;

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  //text_layer_set_text(text_layer, "Select");
  if (game_end)
    window_stack_pop_all(true);

  selectToggle = !selectToggle;

  if(selectToggle)
    vibes_double_pulse();
}


static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  //text_layer_set_text(text_layer, "Up");

  if (game_end)
    window_stack_pop_all(true);


  if (selectToggle)
  {
    update_grid(LEFT, 0);
  }
  else
  {
    update_grid(UP, 0);
  }
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  //text_layer_set_text(text_layer, "Down");

  if (game_end)
    window_stack_pop_all(true);

  if (selectToggle)
  {
    update_grid(RIGHT, 0);
  }
  else
  {
    update_grid(DOWN, 0);
  }
}

static void left_click_handler(ClickRecognizerRef recognizer, void *context) {
  //text_layer_set_text(text_layer, "Down");

    window_stack_pop_all(true);
  }

void accel_data_handler(AccelData *data, uint32_t num_samples) {

  accel_service_peek(data);

  if (data->x < -(accel_sensitivity) && abs(data->y) < accel_sensitivity) 
  {
    update_grid(LEFT, 0);
  }
  else if (data->x > (accel_sensitivity) && abs(data->y) < accel_sensitivity)
  {
    update_grid(RIGHT, 0);
  }
  else if (data->y < -accel_sensitivity && abs(data->x) < accel_sensitivity)
  {
    update_grid(DOWN, 0);
  }
  else if (data->y > accel_sensitivity && abs(data->x) < accel_sensitivity)
  {
    update_grid(UP, 0);
  }
}

void accel_tap_handler(AccelAxisType axis, int32_t direction) {
  // Process tap on ACCEL_AXIS_X, ACCEL_AXIS_Y or ACCEL_AXIS_Z
  // Direction is 1 or -1

  if (ACCEL_AXIS_X == -1) 
  {
    update_grid(LEFT, 0);
  }
  else if (ACCEL_AXIS_X == 1)
  {
    update_grid(RIGHT, 0);
  }
  else if (ACCEL_AXIS_Y == -1)
  {
    update_grid(DOWN, 0);
  }
  else if (ACCEL_AXIS_Y == 1)
  {
    update_grid(UP, 0);
  }

  return;
}

static void timer_callback(void *data) {
  AccelData accel = (AccelData) { .x = 0, .y = 0, .z = 0 };
  accel_service_peek(&accel);

  if (accel.x < -accel_sensitivity) // && abs(accel.y) < accel_sensitivity) 
  {
    update_grid(LEFT, 0);
  }
  else if (accel.x > accel_sensitivity) // && abs(accel.y) < accel_sensitivity)
  {
    update_grid(RIGHT, 0);
  }
  else if (accel.y < -accel_sensitivity) // && abs(accel.x) < accel_sensitivity)
  {
    update_grid(DOWN, 0);
  }
  else if (accel.y > accel_sensitivity) // && abs(accel.x) < accel_sensitivity)
  {
    update_grid(UP, 0);
  }

  timer = app_timer_register(accel_interval, timer_callback, NULL);
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
  window_single_click_subscribe(BUTTON_ID_BACK, left_click_handler);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  //main text layer
  text_layer = text_layer_create((GRect) { .origin = { 0, 72 }, .size = { bounds.size.w, 20 } });
  text_layer_set_text(text_layer, "");
  text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(text_layer));

  //main draw layer
  draw_layer_main = layer_create((GRect) {.origin = {0, 0}, .size = { bounds.size.w, bounds.size.h }});
  layer_add_child(window_layer, draw_layer_main);
  layer_set_update_proc(draw_layer_main, draw_grid);

  //draws
  grid_entries_text_layers = malloc(sizeof(struct TextLayer*) * (grid_slen * grid_slen));
  for (int y = 0; y < grid_slen; y++)
  {
    for (int x = 0; x < grid_slen; x++)
    {
      int index = (grid_slen * y) + x;
      *(grid_entries_text_layers + index) = text_layer_create((GRect) { .origin = {grid_origin.x + (x*boundary_side) + text_stagger.x, grid_origin.y + (y*boundary_side) + text_stagger.y}, .size = { 30, 25 } });
      text_layer_set_text(*(grid_entries_text_layers + index), " ");
      text_layer_set_text_alignment(*(grid_entries_text_layers + index), GTextAlignmentCenter);
      layer_add_child(window_layer, text_layer_get_layer(*(grid_entries_text_layers + index)));
    }
  }

  // score display
  score_display = text_layer_create((GRect) { .origin = { 0, 135 }, .size = { 30, 20 } });
  text_layer_set_text(score_display, "0");
  text_layer_set_text_alignment(score_display, GTextAlignmentLeft);
  text_layer_set_background_color(score_display, GColorClear);
  layer_add_child(window_layer, text_layer_get_layer(score_display));

  //set end game displays bitmap layers and img
  game_end_display = bitmap_layer_create(GRect(0, 0, 144, 168));
  bitmap_layer_set_compositing_mode(game_end_display, GCompOpAssignInverted);
  bitmap_layer_set_background_color(game_end_display, GColorClear);
  layer_add_child(window_layer, bitmap_layer_get_layer(game_end_display));
  win_img = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_WIN);
  lose_img = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_LOSE);



}

static void window_unload(Window *window) {
  text_layer_destroy(text_layer);
}

static void init(void) {
  grid_entries = malloc(sizeof(int) * (grid_slen * grid_slen));
  grid_entries_text = malloc(grid_entry_text_len * sizeof(char) * (grid_slen * grid_slen));
  for (int y = 0; y < grid_slen; y++)
  {
    for (int x = 0; x < grid_slen; x++)
    {
      int index = (grid_slen * y) + x;
      *(grid_entries + index) = -1;
    }
  }


  window = window_create();
  window_set_click_config_provider(window, click_config_provider);
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  const bool animated = true;
  window_stack_push(window, animated);

  // initialize random, add entry twice
  srand(time(NULL));
  add_random_entry();
  add_random_entry();

  accel_service_set_sampling_rate(ACCEL_SAMPLING_10HZ);
  accel_data_service_subscribe(0, NULL);
  timer = app_timer_register(accel_interval, timer_callback, NULL);
  //accel_tap_service_subscribe(&accel_tap_handler);


}

static void deinit(void) {
  window_destroy(window);
  
  free(grid_entries);
  free(grid_entries_text);
  for (int y = 0; y < grid_slen; y++)
  {
    for (int x = 0; x < grid_slen; x++)
    {
      free(grid_entries_text_layers + ((grid_slen * y) + x));
    }
  }

  free(grid_entries_text_layers);
  accel_data_service_unsubscribe();
  bitmap_layer_destroy(game_end_display);
  gbitmap_destroy(win_img);
  gbitmap_destroy(lose_img);
  //accel_tap_service_unsubscribe();
}

int main(void) {
  init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);

  app_event_loop();
  deinit();
}
