#include <pebble.h>

const GColor k_bgColor = { GColorBlackARGB8 };

const GColor k_dotColor = { GColorWhiteARGB8 };
const int k_dotRadius = 70;
const int k_dotSize = 4;

const GColor k_hourColor = { GColorRedARGB8 };
const int k_hourRadius = 35;
const int k_hourSize = 6;

const GColor k_minuteColor = { GColorOrangeARGB8 };
const int k_minuteRadius = 50;
const int k_minuteSize = 6;

const GColor k_dateColor = { GColorWhiteARGB8 };
const char* k_dateFont = FONT_KEY_GOTHIC_14;
const int k_dateRadius = 20;
const int k_dateSize = 20;

Window* g_window = NULL;
Layer* g_layer = NULL;
BatteryChargeState g_chargeState;
GBitmap* g_bluetoothBitmap = NULL;
bool g_connected = false;
GSize g_size;
GPoint g_center;

GPoint getPoint(int minute, int radius)
{
    int angle = TRIG_MAX_ANGLE * (minute - 15) / 60;
    int cos = cos_lookup(angle);
    int sin = sin_lookup(angle);
    return GPoint(g_center.x + cos * radius / TRIG_MAX_RATIO,
                  g_center.y + sin * radius / TRIG_MAX_RATIO);
}

static void layerUpdateProc(Layer* layer, GContext* context)
{
    //APP_LOG(APP_LOG_LEVEL_DEBUG, "%d %d %d %d", cos, sin, p.x, p.y);
    graphics_context_set_fill_color(context, k_dotColor);

    for (int h = 0; h < 12; ++h)
    {
        GPoint p = getPoint(h*5, k_dotRadius);
        graphics_fill_circle(context, p, k_dotSize);
    }

    time_t t = time(NULL);
    tm* tm = localtime(&t);
    int hour = tm->tm_hour % 12;
    int minute = tm->tm_min;
    int date = tm->tm_mday;
    char dateBuf[3];
    snprintf(dateBuf, sizeof(dateBuf), "%d", date);

    GPoint hourPoint = getPoint(hour*5 + minute/12, k_hourRadius);
    graphics_context_set_fill_color(context, k_hourColor);
    graphics_fill_circle(context, hourPoint, k_hourSize);
    graphics_context_set_stroke_color(context, k_hourColor);
    graphics_draw_line(context, g_center, hourPoint);

    GPoint minutePoint = getPoint(minute, k_minuteRadius);
    graphics_context_set_fill_color(context, k_minuteColor);
    graphics_fill_circle(context, minutePoint, k_minuteSize);
    graphics_context_set_stroke_color(context, k_minuteColor);
    graphics_draw_line(context, g_center, minutePoint);

    GPoint datePoint = g_center; // getPoint(45, k_dateRadius);
    graphics_context_set_fill_color(context, k_bgColor);
    graphics_fill_circle(context, datePoint, k_dateSize-2);
    graphics_context_set_stroke_color(context, k_dateColor);
    graphics_draw_circle(context, datePoint, k_dateSize-2);
    GFont font = fonts_get_system_font(k_dateFont);
    GRect dateRect = GRect(datePoint.x - k_dateSize, datePoint.y - k_dateSize, k_dateSize*2, k_dateSize*2);
    //graphics_draw_rect(context, dateRect);
    dateRect.origin.y += 8;
    graphics_draw_text(context, dateBuf, font, dateRect, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);

    // if (!g_connected)
    {
        GPoint bluetoothPoint = getPoint(15, 20);
        GRect bluetoothRect = gbitmap_get_bounds(g_bluetoothBitmap);
        bluetoothRect.origin.x = bluetoothPoint.x - bluetoothRect.size.w/2; 
        bluetoothRect.origin.y = bluetoothPoint.y - bluetoothRect.size.h/2; 
        graphics_draw_bitmap_in_rect(context, g_bluetoothBitmap, bluetoothRect);
    }
}

static void tickTimerHandler(struct tm* tick_time, TimeUnits units_changed) 
{
    layer_mark_dirty(g_layer);
}

static void bluetoothConnectionHandler(bool connected)
{
    g_connected = connected;
    layer_mark_dirty(g_layer);
}

static void batteryStateHandler(BatteryChargeState charge)
{
    g_chargeState = charge;
    layer_mark_dirty(g_layer);
    // text_layer_set_text(g_battery_layer, battery_str(charge));
    // bool battery_on = (charge.charge_percent < 30 || charge.is_charging);
    // g_battery_on = battery_on;
}

static void init() 
{
    g_window = window_create();
    window_set_background_color(g_window, k_bgColor);
    window_stack_push(g_window, true);

    Layer* windowLayer = window_get_root_layer(g_window);
    GRect bounds = layer_get_frame(windowLayer);
    g_size = bounds.size;
    g_center = GPoint(g_size.w/2, g_size.h/2);

    g_layer = layer_create(bounds);
    layer_set_update_proc(g_layer, layerUpdateProc);
    layer_add_child(windowLayer, g_layer);

    g_bluetoothBitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BLUETOOTH);

    // subscribe to services
    tick_timer_service_subscribe(MINUTE_UNIT, &tickTimerHandler);
    bluetooth_connection_service_subscribe(bluetoothConnectionHandler);
    battery_state_service_subscribe(batteryStateHandler);
}

static void deinit() 
{
    tick_timer_service_unsubscribe();
    bluetooth_connection_service_unsubscribe();
    battery_state_service_unsubscribe();

    gbitmap_destroy(g_bluetoothBitmap);

    layer_destroy(g_layer);
    window_destroy(g_window);
}

int main(void) 
{
    init();
    app_event_loop();
    deinit();
}
