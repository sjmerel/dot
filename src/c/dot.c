#include <pebble.h>


const GColor k_bgColor = { GColorBlackARGB8 };

const GColor k_dotColor = { GColorWhiteARGB8 };
enum { k_dotRadius = 90 };
enum { k_dotSize = 6 };

const GColor k_hourColor = { GColorOrangeARGB8 };
enum { k_hourRadius = 35 };
enum { k_hourSize = 8 };

const GColor k_minuteColor = { GColorChromeYellowARGB8 };
enum { k_minuteRadius = 65 };
enum { k_minuteSize = 8 };

const GColor k_dateColor = { GColorWhiteARGB8 };
enum { k_dateRadius = 82 };
enum { k_dateSize = 12 };

Window* g_window = NULL;
Layer* g_layer = NULL;
BatteryChargeState g_batteryState;
GBitmap* g_bluetoothBitmap = NULL;
GFont g_dateFont;
GFont g_batteryFont;
bool g_connected = false;
GSize g_size;
GPoint g_center;

int getAngle(int minute)
{
    return TRIG_MAX_ANGLE * (minute - 15) / 60;
}

#ifdef PBL_PLATFORM_BASALT
GPoint getPoint(int minute, int radiusPct)
{
    int angle = getAngle(minute);
    int cos = cos_lookup(angle);
    int sin = sin_lookup(angle);
    int x;
    int y;
    const int my = 84*radiusPct/100;
    const int mx = 72*radiusPct/100;
    if (minute > 53 || minute < 7)
    {
        y = -my-1;
        x = y * cos / sin;
    }
    else if (minute >= 7 && minute < 24)
    {
        x = mx;
        y = x * sin / cos;
    }
    else if (minute >= 24 && minute < 37)
    {
        y = my;
        x = y * cos / sin;
    }
    else
    {
        x = -mx-1;
        y = x * sin / cos;
    }
    return GPoint(g_center.x + x, g_center.y + y);
}
#else
GPoint getPoint(int minute, int radiusPct)
{
    int angle = getAngle(minute);
    int cos = cos_lookup(angle);
    int sin = sin_lookup(angle);
    int radius = g_size.w*radiusPct/100;
    return GPoint(g_center.x + cos * radius / TRIG_MAX_RATIO,
                  g_center.y + sin * radius / TRIG_MAX_RATIO);
}
#endif

static void layerUpdateProc(Layer* layer, GContext* context)
{
    //APP_LOG(APP_LOG_LEVEL_DEBUG, "%d %d %d %d", cos, sin, p.x, p.y);

    graphics_context_set_stroke_color(context, k_dotColor);
    graphics_context_set_antialiased(context, true);
    /*
    for (int m = 0; m < 60; ++m)
    {
        GPoint p = getPoint(m, k_dotRadius);
        graphics_draw_circle(context, p, 2);
    }
    */

    for (int h = 0; h < 12; ++h)
    {
        GPoint p = getPoint(h*5, k_dotRadius);
        graphics_draw_circle(context, p, k_dotSize);
    }

    time_t t = time(NULL);
    tm* tm = localtime(&t);
    int hour = tm->tm_hour % 12;
    int minute = tm->tm_min;
    int date = tm->tm_mday;
    static char dateBuf[3];
    snprintf(dateBuf, sizeof(dateBuf), "%d", date);

    if (!g_connected)
    {
        GPoint p = getPoint(30, 22);
        GRect rect = gbitmap_get_bounds(g_bluetoothBitmap);
        rect.origin.x = p.x - rect.size.w/2; 
        rect.origin.y = p.y - rect.size.h/2; 
        graphics_draw_bitmap_in_rect(context, g_bluetoothBitmap, rect);
    }

    if (g_batteryState.is_charging || g_batteryState.charge_percent < 30)
    {
        static char batteryBuf[5];
        snprintf(batteryBuf, sizeof(batteryBuf), "%d%%", g_batteryState.charge_percent);
        GPoint p = getPoint(0, 22);
        GRect rect = GRect(p.x - 30, p.y - 10, 60, 20);
        graphics_draw_text(context, batteryBuf, g_batteryFont, rect, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
    }

    // hour hand
    int hmin = hour*5 + minute/12;
    GPoint hourPoint = getPoint(hmin, k_hourRadius);
    graphics_context_set_fill_color(context, k_hourColor);
    graphics_fill_circle(context, hourPoint, k_hourSize);
    graphics_context_set_stroke_color(context, k_hourColor);
    graphics_context_set_stroke_width(context, 4);
    graphics_draw_line(context, g_center, getPoint(hmin, k_hourRadius));

    // minute hand
    GPoint minutePoint = getPoint(minute, k_minuteRadius);
    graphics_context_set_fill_color(context, k_minuteColor);
    graphics_fill_circle(context, minutePoint, k_minuteSize);
    graphics_context_set_stroke_color(context, k_minuteColor);
    graphics_context_set_stroke_width(context, 4);
    graphics_draw_line(context, g_center, getPoint(minute, k_minuteRadius));

    graphics_context_set_fill_color(context, k_minuteColor);
    graphics_fill_circle(context, g_center, 10);
    graphics_context_set_fill_color(context, k_hourColor);
    graphics_fill_circle(context, g_center, 5);

    GPoint datePoint = getPoint(15, k_dateRadius);
    graphics_context_set_stroke_width(context, 1);
    graphics_context_set_fill_color(context, k_bgColor);
    graphics_fill_circle(context, datePoint, k_dateSize);
    graphics_context_set_stroke_color(context, k_dateColor);
    graphics_draw_circle(context, datePoint, k_dateSize);
    GRect dateRect = GRect(datePoint.x - k_dateSize, datePoint.y - k_dateSize, k_dateSize*2, k_dateSize*2);
    //graphics_draw_rect(context, dateRect);
    dateRect.origin.y += 0;
    dateRect.origin.x += 1;
    graphics_draw_text(context, dateBuf, g_dateFont, dateRect, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);

}

static void tickTimerHandler(struct tm* tick_time, TimeUnits units_changed) 
{
    layer_mark_dirty(g_layer);
}

static void bluetoothConnectionHandler(bool connected)
{
    g_connected = connected;
    if (!connected)
    {
        vibes_short_pulse();
    }
    layer_mark_dirty(g_layer);
}

static void batteryStateHandler(BatteryChargeState charge)
{
    g_batteryState = charge;
    layer_mark_dirty(g_layer);
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

    g_batteryFont = fonts_get_system_font(FONT_KEY_GOTHIC_14);
    g_dateFont = fonts_get_system_font(FONT_KEY_GOTHIC_18);
    // g_dateFont = fonts_get_system_font(FONT_KEY_LECO_20_BOLD_NUMBERS);
    g_bluetoothBitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BLUETOOTH);

    // subscribe to services
    tick_timer_service_subscribe(MINUTE_UNIT, &tickTimerHandler);
    bluetooth_connection_service_subscribe(bluetoothConnectionHandler);
    battery_state_service_subscribe(batteryStateHandler);

    bluetoothConnectionHandler(bluetooth_connection_service_peek());
    batteryStateHandler(battery_state_service_peek());
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
