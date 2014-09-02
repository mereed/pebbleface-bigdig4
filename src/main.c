#include "pebble.h"

int STYLE_KEY = 1;

GColor background_color = GColorBlack;
GColor foreground_color = GColorWhite;
GCompOp compositing_mode = GCompOpAssign;

static uint8_t batteryPercent;

static Window *window;
static Layer *window_layer;

static bool appStarted = false;

BitmapLayer *layer_conn_img;
GBitmap *img_bt_connect;
GBitmap *img_bt_disconnect;

static GBitmap *day_name_image;
static BitmapLayer *day_name_layer;

static GBitmap *background_image;
static BitmapLayer *background_layer;

static GBitmap *time_format_image;
static BitmapLayer *time_format_layer;

const int DAY_NAME_IMAGE_RESOURCE_IDS[] = {
  RESOURCE_ID_IMAGE_DAY_NAME_SUN,
  RESOURCE_ID_IMAGE_DAY_NAME_MON,
  RESOURCE_ID_IMAGE_DAY_NAME_TUE,
  RESOURCE_ID_IMAGE_DAY_NAME_WED,
  RESOURCE_ID_IMAGE_DAY_NAME_THU,
  RESOURCE_ID_IMAGE_DAY_NAME_FRI,
  RESOURCE_ID_IMAGE_DAY_NAME_SAT
};

#define TOTAL_TIME_DIGITS 2
static GBitmap *time_digits_images[TOTAL_TIME_DIGITS];
static BitmapLayer *time_digits_layers[TOTAL_TIME_DIGITS];

const int BIG_DIGIT_IMAGE_RESOURCE_IDS[] = {
  RESOURCE_ID_IMAGE_NUM_0,
  RESOURCE_ID_IMAGE_NUM_1,
  RESOURCE_ID_IMAGE_NUM_2,
  RESOURCE_ID_IMAGE_NUM_3,
  RESOURCE_ID_IMAGE_NUM_4,
  RESOURCE_ID_IMAGE_NUM_5,
  RESOURCE_ID_IMAGE_NUM_6,
  RESOURCE_ID_IMAGE_NUM_7,
  RESOURCE_ID_IMAGE_NUM_8,
  RESOURCE_ID_IMAGE_NUM_9
};

#define TOTAL_MTIME_DIGITS 2
static GBitmap *mtime_digits_images[TOTAL_MTIME_DIGITS];
static BitmapLayer *mtime_digits_layers[TOTAL_MTIME_DIGITS];

const int MIN_DIGIT_IMAGE_RESOURCE_IDS[] = {
  RESOURCE_ID_IMAGE_NUM_M0,
  RESOURCE_ID_IMAGE_NUM_M1,
  RESOURCE_ID_IMAGE_NUM_M2,
  RESOURCE_ID_IMAGE_NUM_M3,
  RESOURCE_ID_IMAGE_NUM_M4,
  RESOURCE_ID_IMAGE_NUM_M5,
  RESOURCE_ID_IMAGE_NUM_M6,
  RESOURCE_ID_IMAGE_NUM_M7,
  RESOURCE_ID_IMAGE_NUM_M8,
  RESOURCE_ID_IMAGE_NUM_M9
};

#define TOTAL_DATE_DIGITS 2	
static GBitmap *date_digits_images[TOTAL_DATE_DIGITS];
static BitmapLayer *date_digits_layers[TOTAL_DATE_DIGITS];

const int DATE_DIGIT_IMAGE_RESOURCE_IDS[] = {
  RESOURCE_ID_IMAGE_DATENUM_0,
  RESOURCE_ID_IMAGE_DATENUM_1,
  RESOURCE_ID_IMAGE_DATENUM_2,
  RESOURCE_ID_IMAGE_DATENUM_3,
  RESOURCE_ID_IMAGE_DATENUM_4,
  RESOURCE_ID_IMAGE_DATENUM_5,
  RESOURCE_ID_IMAGE_DATENUM_6,
  RESOURCE_ID_IMAGE_DATENUM_7,
  RESOURCE_ID_IMAGE_DATENUM_8,
  RESOURCE_ID_IMAGE_DATENUM_9
};

#define TOTAL_SECONDS_DIGITS 2
static GBitmap *seconds_digits_images[TOTAL_SECONDS_DIGITS];
static BitmapLayer *seconds_digits_layers[TOTAL_SECONDS_DIGITS];

const int SEC_IMAGE_RESOURCE_IDS[] = {
  RESOURCE_ID_IMAGE_NUM_S0,
  RESOURCE_ID_IMAGE_NUM_S1,
  RESOURCE_ID_IMAGE_NUM_S2,
  RESOURCE_ID_IMAGE_NUM_S3,
  RESOURCE_ID_IMAGE_NUM_S4,
  RESOURCE_ID_IMAGE_NUM_S5,
  RESOURCE_ID_IMAGE_NUM_S6,
  RESOURCE_ID_IMAGE_NUM_S7,
  RESOURCE_ID_IMAGE_NUM_S8,
  RESOURCE_ID_IMAGE_NUM_S9,
  RESOURCE_ID_IMAGE_TINY_PERCENT
};

#define TOTAL_BATTERY_PERCENT_DIGITS 3
static GBitmap *battery_percent_image[TOTAL_BATTERY_PERCENT_DIGITS];
static BitmapLayer *battery_percent_layers[TOTAL_BATTERY_PERCENT_DIGITS];

static GBitmap *battery_image;
static BitmapLayer *battery_image_layer;
static BitmapLayer *battery_layer;
int charge_percent = 0;


static void set_container_image(GBitmap **bmp_image, BitmapLayer *bmp_layer, const int resource_id, GPoint origin) {
  GBitmap *old_image = *bmp_image;
  *bmp_image = gbitmap_create_with_resource(resource_id);
  GRect frame = (GRect) {
    .origin = origin,
    .size = (*bmp_image)->bounds.size
  };
  bitmap_layer_set_bitmap(bmp_layer, *bmp_image);
  layer_set_frame(bitmap_layer_get_layer(bmp_layer), frame);
  if (old_image != NULL) {
	gbitmap_destroy(old_image);
	old_image = NULL;
  }
}

void update_battery(BatteryChargeState charge_state) {

  batteryPercent = charge_state.charge_percent;

  if(batteryPercent==100) {
	layer_set_hidden(bitmap_layer_get_layer(battery_image_layer), false);
    for (int i = 0; i < TOTAL_BATTERY_PERCENT_DIGITS; ++i) {
      layer_set_hidden(bitmap_layer_get_layer(battery_percent_layers[i]), true);
    }  
    return;
  }

 for (int i = 0; i < TOTAL_BATTERY_PERCENT_DIGITS; ++i) {
	layer_set_hidden(bitmap_layer_get_layer(battery_image_layer), true);
    layer_set_hidden(bitmap_layer_get_layer(battery_percent_layers[i]), false);
  }  
  set_container_image(&battery_percent_image[0], battery_percent_layers[0], SEC_IMAGE_RESOURCE_IDS[charge_state.charge_percent/10], GPoint(6, 40));
  set_container_image(&battery_percent_image[1], battery_percent_layers[1], SEC_IMAGE_RESOURCE_IDS[charge_state.charge_percent%10], GPoint(13, 40));
  set_container_image(&battery_percent_image[2], battery_percent_layers[2], SEC_IMAGE_RESOURCE_IDS[10], GPoint(20, 40));
 
}


static void toggle_bluetooth_icon(bool connected) {
  if (connected) {
        bitmap_layer_set_bitmap(layer_conn_img, img_bt_connect);
    } else {
        bitmap_layer_set_bitmap(layer_conn_img, img_bt_disconnect);
	  
	          vibes_long_pulse();

    }
}

void bluetooth_connection_callback(bool connected) {
  toggle_bluetooth_icon(connected);
}

void handle_appfocus(bool in_focus){
    if (in_focus) {
    toggle_bluetooth_icon(bluetooth_connection_service_peek());
    }
}

unsigned short get_display_hour(unsigned short hour) {
  if (clock_is_24h_style()) {
    return hour;
  }
  unsigned short display_hour = hour % 12;
  // Converts "0" to "12"
  return display_hour ? display_hour : 12;
}

static void update_days(struct tm *tick_time) {
  set_container_image(&day_name_image, day_name_layer, DAY_NAME_IMAGE_RESOURCE_IDS[tick_time->tm_wday], GPoint(30, 1));
  set_container_image(&date_digits_images[0], date_digits_layers[0], DATE_DIGIT_IMAGE_RESOURCE_IDS[tick_time->tm_mday/10], GPoint(2, 1));
  set_container_image(&date_digits_images[1], date_digits_layers[1], DATE_DIGIT_IMAGE_RESOURCE_IDS[tick_time->tm_mday%10], GPoint(2, 16));
}

static void update_hours(struct tm *tick_time) {
	
  unsigned short display_hour = get_display_hour(tick_time->tm_hour);

  set_container_image(&time_digits_images[0], time_digits_layers[0], BIG_DIGIT_IMAGE_RESOURCE_IDS[display_hour/10], GPoint(78, 2));
  set_container_image(&time_digits_images[1], time_digits_layers[1], BIG_DIGIT_IMAGE_RESOURCE_IDS[display_hour%10], GPoint(111,2));

  if (!clock_is_24h_style()) {
    
	  if (tick_time->tm_hour >= 12) {
      set_container_image(&time_format_image, time_format_layer, RESOURCE_ID_IMAGE_PM_MODE, GPoint(62, 2));
      layer_set_hidden(bitmap_layer_get_layer(time_format_layer), false);
    } 
    
	  if (tick_time->tm_hour <= 11)  {
     set_container_image(&time_format_image, time_format_layer, RESOURCE_ID_IMAGE_AM_MODE, GPoint(62, 2));
     layer_set_hidden(bitmap_layer_get_layer(time_format_layer), false);
    } 
    
    if (display_hour/10 == 0) {
      layer_set_hidden(bitmap_layer_get_layer(time_digits_layers[0]), true);
    }
    else {
      layer_set_hidden(bitmap_layer_get_layer(time_digits_layers[0]), false);
    }

  }
}

static void update_minutes(struct tm *tick_time) {
  set_container_image(&mtime_digits_images[0], mtime_digits_layers[0], MIN_DIGIT_IMAGE_RESOURCE_IDS[tick_time->tm_min/10], GPoint(1, 57));
  set_container_image(&mtime_digits_images[1], mtime_digits_layers[1], MIN_DIGIT_IMAGE_RESOURCE_IDS[tick_time->tm_min%10], GPoint(76, 57));
}

static void handle_tick(struct tm *tick_time, TimeUnits units_changed) {
  if (units_changed & DAY_UNIT) {
    update_days(tick_time);
  }
  if (units_changed & HOUR_UNIT) {
    update_hours(tick_time);
  }
	  if (units_changed & MINUTE_UNIT) {
   update_minutes(tick_time);
  }	
		
}

void force_update(void) {
    update_battery(battery_state_service_peek());
    toggle_bluetooth_icon(bluetooth_connection_service_peek());
}

void set_style(void) {
    bool inverse = persist_read_bool(STYLE_KEY);
    
    background_color  = inverse ? GColorWhite : GColorBlack;
    foreground_color  = inverse ? GColorBlack : GColorWhite;
    compositing_mode  = inverse ? GCompOpAssignInverted : GCompOpAssign;
    
    window_set_background_color(window, background_color);
    bitmap_layer_set_compositing_mode(day_name_layer, compositing_mode);
    bitmap_layer_set_compositing_mode(background_layer, compositing_mode);
    bitmap_layer_set_compositing_mode(layer_conn_img, compositing_mode);
    bitmap_layer_set_compositing_mode(battery_image_layer, compositing_mode);
    bitmap_layer_set_compositing_mode(battery_layer, compositing_mode);
    bitmap_layer_set_compositing_mode(time_format_layer, compositing_mode);
	
    for (int i = 0; i < TOTAL_TIME_DIGITS; ++i) {
		bitmap_layer_set_compositing_mode(time_digits_layers[i], compositing_mode);
	}
	
    for (int i = 0; i < TOTAL_MTIME_DIGITS; ++i) {
		bitmap_layer_set_compositing_mode(mtime_digits_layers[i], compositing_mode);
	}

    for (int i = 0; i < TOTAL_BATTERY_PERCENT_DIGITS; ++i) {
		bitmap_layer_set_compositing_mode(battery_percent_layers[i], compositing_mode);
	}
	
	for (int i = 0; i < TOTAL_DATE_DIGITS; ++i) {
		bitmap_layer_set_compositing_mode(date_digits_layers[i], compositing_mode);
	}
}

void handle_tap(AccelAxisType axis, int32_t direction) {
    persist_write_bool(STYLE_KEY, !persist_read_bool(STYLE_KEY));
    set_style();
    force_update();
    vibes_long_pulse();
    accel_tap_service_unsubscribe();
}

void handle_tap_timeout(void* data) {
    accel_tap_service_unsubscribe();
	
	 // Avoids a blank screen on watch start.
  time_t now = time(NULL);
  struct tm *tick_time = localtime(&now);  
  handle_tick(tick_time, DAY_UNIT + HOUR_UNIT + MINUTE_UNIT );

}

static void init(void) {
  memset(&time_digits_layers, 0, sizeof(time_digits_layers));
  memset(&time_digits_images, 0, sizeof(time_digits_images));
  memset(&mtime_digits_layers, 0, sizeof(mtime_digits_layers));
  memset(&mtime_digits_images, 0, sizeof(mtime_digits_images));
  memset(&battery_percent_layers, 0, sizeof(battery_percent_layers));
  memset(&battery_percent_image, 0, sizeof(battery_percent_image));
  memset(&date_digits_layers, 0, sizeof(date_digits_layers));
  memset(&date_digits_images, 0, sizeof(date_digits_images));
	
  window = window_create();
  if (window == NULL) {
      //APP_LOG(APP_LOG_LEVEL_DEBUG, "OOM: couldn't allocate window");
      return;
  }
  window_stack_push(window, true /* Animated */);
  window_layer = window_get_root_layer(window);
  
  background_color  = GColorBlack;
  window_set_background_color(window, background_color);	
	
  background_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND);
  GRect frame2 = (GRect) {
    .origin = { .x = 0, .y = 0 },
    .size = background_image->bounds.size
  };
  background_layer = bitmap_layer_create(frame2);
  bitmap_layer_set_bitmap(background_layer, background_image);
  layer_add_child(window_layer, bitmap_layer_get_layer(background_layer));  

     // resources
	img_bt_connect     = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BLUETOOTHON);
    img_bt_disconnect  = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BLUETOOTHOFF);
	
    layer_conn_img  = bitmap_layer_create(GRect(32, 38, 34, 14));
    bitmap_layer_set_bitmap(layer_conn_img, img_bt_connect);
    layer_add_child(window_layer, bitmap_layer_get_layer(layer_conn_img)); 

  battery_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BATTERY);
  GRect frame4 = (GRect) {
    .origin = { .x = 5, .y = 43 },
    .size = battery_image->bounds.size
  };
  battery_layer = bitmap_layer_create(frame4);
  battery_image_layer = bitmap_layer_create(frame4);
  bitmap_layer_set_bitmap(battery_image_layer, battery_image);
  
  layer_add_child(window_layer, bitmap_layer_get_layer(battery_image_layer));
  layer_add_child(window_layer, bitmap_layer_get_layer(battery_layer));
	
  // Create time and date layers
  GRect dummy_frame = { {0, 0}, {0, 0} };
  day_name_layer = bitmap_layer_create(dummy_frame);
  layer_add_child(window_layer, bitmap_layer_get_layer(day_name_layer));
	
  for (int i = 0; i < TOTAL_TIME_DIGITS; ++i) {
    time_digits_layers[i] = bitmap_layer_create(dummy_frame);
    layer_add_child(window_layer, bitmap_layer_get_layer(time_digits_layers[i]));
  }
	
	for (int i = 0; i < TOTAL_MTIME_DIGITS; ++i) {
    mtime_digits_layers[i] = bitmap_layer_create(dummy_frame);  
    layer_add_child(window_layer, bitmap_layer_get_layer(mtime_digits_layers[i]));
  }
	
	GRect frame5 = (GRect) {
    .origin = { .x = 63, .y = 2 },
    .size = {.w = 11, .h = 26}
  };
  time_format_layer = bitmap_layer_create(frame5);
  if (clock_is_24h_style()) {
    time_format_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_24_HOUR_MODE);
    bitmap_layer_set_bitmap(time_format_layer, time_format_image);
	}
  layer_add_child(window_layer, bitmap_layer_get_layer(time_format_layer));
	
	
   for (int i = 0; i < TOTAL_DATE_DIGITS; ++i) {
    date_digits_layers[i] = bitmap_layer_create(dummy_frame);
    layer_add_child(window_layer, bitmap_layer_get_layer(date_digits_layers[i]));
  }
	
	for (int i = 0; i < TOTAL_BATTERY_PERCENT_DIGITS; ++i) {
    battery_percent_layers[i] = bitmap_layer_create(dummy_frame);
    layer_add_child(window_layer, bitmap_layer_get_layer(battery_percent_layers[i]));
  }


	// style
    set_style();
	
		 // Avoids a blank screen on watch start.
  time_t now = time(NULL);
  struct tm *tick_time = localtime(&now);  
  handle_tick(tick_time, DAY_UNIT + HOUR_UNIT + MINUTE_UNIT );


  appStarted = true;
  
 
	 // handlers
    battery_state_service_subscribe(&update_battery);
    bluetooth_connection_service_subscribe(&bluetooth_connection_callback);
    app_focus_service_subscribe(&handle_appfocus);
    tick_timer_service_subscribe(MINUTE_UNIT, handle_tick);
    accel_tap_service_subscribe(handle_tap);
    app_timer_register(10000, handle_tap_timeout, NULL);

	 // draw first frame
    force_update();

}

static void deinit(void) {
  
  tick_timer_service_unsubscribe();
  bluetooth_connection_service_unsubscribe();
  battery_state_service_unsubscribe();
  app_focus_service_unsubscribe();
  accel_tap_service_unsubscribe();
  

  layer_remove_from_parent(bitmap_layer_get_layer(time_format_layer));
  bitmap_layer_destroy(time_format_layer);
  gbitmap_destroy(time_format_image);

  layer_remove_from_parent(bitmap_layer_get_layer(layer_conn_img));
  bitmap_layer_destroy(layer_conn_img);
  gbitmap_destroy(img_bt_connect);
  gbitmap_destroy(img_bt_disconnect);
 
  layer_remove_from_parent(bitmap_layer_get_layer(background_layer));
  bitmap_layer_destroy(background_layer);
  gbitmap_destroy(background_image);
  background_image = NULL;
	
  layer_remove_from_parent(bitmap_layer_get_layer(day_name_layer));
  bitmap_layer_destroy(day_name_layer);
  gbitmap_destroy(day_name_image);
  day_name_image = NULL;
	
  layer_remove_from_parent(bitmap_layer_get_layer(battery_layer));
  bitmap_layer_destroy(battery_layer);
  gbitmap_destroy(battery_image);
  
  layer_remove_from_parent(bitmap_layer_get_layer(battery_image_layer));
  bitmap_layer_destroy(battery_image_layer);
	
	for (int i = 0; i < TOTAL_TIME_DIGITS; i++) {
    layer_remove_from_parent(bitmap_layer_get_layer(time_digits_layers[i]));
    gbitmap_destroy(time_digits_images[i]);
    time_digits_images[i] = NULL;
    bitmap_layer_destroy(time_digits_layers[i]);
	time_digits_layers[i] = NULL;
  
  }

	for (int i = 0; i < TOTAL_MTIME_DIGITS; i++) {
    layer_remove_from_parent(bitmap_layer_get_layer(mtime_digits_layers[i]));
    gbitmap_destroy(mtime_digits_images[i]);
    mtime_digits_images[i] = NULL;
    bitmap_layer_destroy(mtime_digits_layers[i]);
	mtime_digits_layers[i] = NULL;
  
  }

	for (int i = 0; i < TOTAL_SECONDS_DIGITS; i++) {
    layer_remove_from_parent(bitmap_layer_get_layer(seconds_digits_layers[i]));
    gbitmap_destroy(seconds_digits_images[i]);
    seconds_digits_images[i] = NULL;
    bitmap_layer_destroy(seconds_digits_layers[i]);
	seconds_digits_layers[i] = NULL;
  
  }

	for (int i = 0; i < TOTAL_BATTERY_PERCENT_DIGITS; ++i) {
	layer_remove_from_parent(bitmap_layer_get_layer(battery_percent_layers[i]));
    gbitmap_destroy(battery_percent_image[i]);
    battery_percent_image[i] = NULL;
    bitmap_layer_destroy(battery_percent_layers[i]);
	battery_percent_layers[i] = NULL;
  }
	
	for (int i = 0; i < TOTAL_DATE_DIGITS; ++i) {
	layer_remove_from_parent(bitmap_layer_get_layer(date_digits_layers[i]));
    gbitmap_destroy(date_digits_images[i]);
    date_digits_images[i] = NULL;
    bitmap_layer_destroy(date_digits_layers[i]);
	date_digits_layers[i] = NULL;
  }
	
  layer_remove_from_parent(window_layer);
  layer_destroy(window_layer);
	
  //window_destroy(window);

}

int main(void) {
  init();
  app_event_loop();
  deinit();
}