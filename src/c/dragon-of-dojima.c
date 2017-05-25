#include <pebble.h>
#include "dragon-of-dojima.h"

static Window *s_window;
static TextLayer *s_text_layer;

static GBitmap *s_bitmap;
static BitmapLayer *s_bitmap_layer;

static Layer *s_bg_layer;

static GPath *s_hour_hand, *s_minute_hand;
static Layer *s_hands_layer;

static void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {
  layer_mark_dirty(s_hands_layer);
}

static int32_t get_hour_angle(int hour) {
  return (hour * 360) /12;
}

static int32_t get_minute_angle(int hour) {
  return (hour * 360) /60;
}

static void bg_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  GRect frame = grect_inset(bounds, GEdgeInsets(4));
  GRect inner_hour_frame = grect_inset(bounds, GEdgeInsets(4 + 4));
  GRect inner_minute_frame = grect_inset(bounds, GEdgeInsets(4 + 2));

  /* Hour markings */
  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_context_set_stroke_width(ctx, 2);

  for (int i = 0; i < 12; i++) {
    int hour_angle = get_hour_angle(i);
    GPoint p0 = gpoint_from_polar(frame, GOvalScaleModeFitCircle, DEG_TO_TRIGANGLE(hour_angle));
    GPoint p1 = gpoint_from_polar(inner_hour_frame, GOvalScaleModeFitCircle, DEG_TO_TRIGANGLE(hour_angle));
    graphics_draw_line(ctx, p0, p1);
  }

  /* Minute markings */
  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_context_set_stroke_width(ctx, 1);

  for (int i = 0; i < 60; i++) {
    if (i % 5) {
      int minute_angle = get_minute_angle(i);
      GPoint p0 = gpoint_from_polar(frame, GOvalScaleModeFitCircle, DEG_TO_TRIGANGLE(minute_angle));
      GPoint p1 = gpoint_from_polar(inner_minute_frame, GOvalScaleModeFitCircle, DEG_TO_TRIGANGLE(minute_angle));
      graphics_draw_line(ctx, p0, p1);
    }
  }
}

static void hands_update_proc(Layer *layer, GContext *ctx) {
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  
  GRect bounds = layer_get_bounds(layer);

  /* Hour hand */
  graphics_context_set_fill_color(ctx, GColorRed);
  gpath_rotate_to(s_hour_hand, (TRIG_MAX_ANGLE * (((t->tm_hour % 12) * 6) + (t->tm_min / 10))) / (12 * 6));
  gpath_draw_filled(ctx, s_hour_hand);

  /* Minute hand */
  graphics_context_set_fill_color(ctx, GColorRed);
  gpath_rotate_to(s_minute_hand, TRIG_MAX_ANGLE * t->tm_min / 60);
  gpath_draw_filled(ctx, s_minute_hand);

  /* Centre dot */
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_context_set_stroke_color(ctx, GColorRed);
  graphics_context_set_stroke_width(ctx, 1);
  graphics_fill_circle(ctx, GPoint(bounds.size.w / 2, bounds.size.h / 2), 4);
  graphics_draw_circle(ctx, GPoint(bounds.size.w / 2, bounds.size.h / 2), 4);
}

static void prv_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  /* Bitmap image layer */
  s_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BACKGROUND_IMAGE);
  s_bitmap_layer = bitmap_layer_create(GRect(0, 0, bounds.size.w, bounds.size.h));
  bitmap_layer_set_compositing_mode(s_bitmap_layer, GCompOpSet);
  bitmap_layer_set_bitmap(s_bitmap_layer, s_bitmap);
  layer_add_child(window_get_root_layer(window), 
    bitmap_layer_get_layer(s_bitmap_layer));

  /* Background layer */
  s_bg_layer = layer_create(bounds);
  layer_set_update_proc(s_bg_layer, bg_update_proc);
  layer_add_child(window_layer, s_bg_layer);

  /* Hands layer */
  s_minute_hand = gpath_create(&MINUTE_HAND_POINTS);
  s_hour_hand = gpath_create(&HOUR_HAND_POINTS);
  GPoint center = grect_center_point(&bounds);
  gpath_move_to(s_minute_hand, center);
  gpath_move_to(s_hour_hand, center);
  s_hands_layer = layer_create(bounds);
  layer_set_update_proc(s_hands_layer, hands_update_proc);
  layer_add_child(window_layer, s_hands_layer);
}

static void prv_window_unload(Window *window) {
  text_layer_destroy(s_text_layer);
}

static void prv_init(void) {
  s_window = window_create();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = prv_window_load,
    .unload = prv_window_unload,
  });
  const bool animated = true;
  window_stack_push(s_window, animated);

  tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);
}

static void prv_deinit(void) {
  gbitmap_destroy(s_bitmap);
  bitmap_layer_destroy(s_bitmap_layer);

  gpath_destroy(s_hour_hand);
  gpath_destroy(s_minute_hand);
  
  layer_destroy(s_hands_layer);
  layer_destroy(s_bg_layer);

  window_destroy(s_window);

  tick_timer_service_unsubscribe();
}

int main(void) {
  prv_init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", s_window);

  app_event_loop();
  prv_deinit();
}
