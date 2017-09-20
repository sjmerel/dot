#include <pebble.h>

uint8_t g_colors[] =
{
GColorOxfordBlueARGB8,
GColorDukeBlueARGB8,
GColorBlueARGB8,
GColorDarkGreenARGB8,
GColorMidnightGreenARGB8,
GColorCobaltBlueARGB8,
GColorBlueMoonARGB8,
GColorIslamicGreenARGB8,
GColorJaegerGreenARGB8,
GColorTiffanyBlueARGB8,
GColorVividCeruleanARGB8,
GColorGreenARGB8,
GColorMalachiteARGB8,
GColorMediumSpringGreenARGB8,
GColorCyanARGB8,
GColorBulgarianRoseARGB8,
GColorImperialPurpleARGB8,
GColorIndigoARGB8,
GColorElectricUltramarineARGB8,
GColorArmyGreenARGB8,
GColorDarkGrayARGB8,
GColorLibertyARGB8,
GColorVeryLightBlueARGB8,
GColorKellyGreenARGB8,
GColorMayGreenARGB8,
GColorCadetBlueARGB8,
GColorPictonBlueARGB8,
GColorBrightGreenARGB8,
GColorScreaminGreenARGB8,
GColorMediumAquamarineARGB8,
GColorElectricBlueARGB8,
GColorDarkCandyAppleRedARGB8,
GColorJazzberryJamARGB8,
GColorPurpleARGB8,
GColorVividVioletARGB8,
GColorWindsorTanARGB8,
GColorRoseValeARGB8,
GColorPurpureusARGB8,
GColorLavenderIndigoARGB8,
GColorLimerickARGB8,
GColorBrassARGB8,
GColorLightGrayARGB8,
GColorBabyBlueEyesARGB8,
GColorSpringBudARGB8,
GColorInchwormARGB8,
GColorMintGreenARGB8,
GColorCelesteARGB8,
GColorRedARGB8,
GColorFollyARGB8,
GColorFashionMagentaARGB8,
GColorMagentaARGB8,
GColorOrangeARGB8,
GColorSunsetOrangeARGB8,
GColorBrilliantRoseARGB8,
GColorShockingPinkARGB8,
GColorChromeYellowARGB8,
GColorRajahARGB8,
GColorMelonARGB8,
GColorRichBrilliantLavenderARGB8,
GColorYellowARGB8,
GColorIcterineARGB8,
GColorPastelYellowARGB8,
GColorWhiteARGB8,
    };
int g_colorIndex = 0;

const GColor k_bgColor = { GColorBlackARGB8 };

const GColor k_dotColor = { GColorWhiteARGB8 };
enum { k_dotRadius = 70 };
enum { k_dotSize = 6 };

const GColor k_hourColor = { GColorOrangeARGB8 };
enum { k_hourRadius = 30 };
enum { k_hourSize = 6 };

const GColor k_minuteColor = { GColorChromeYellowARGB8 };
enum { k_minuteRadius = 50 };
enum { k_minuteSize = 6 };

const GColor k_dateColor = { GColorWhiteARGB8 };
enum { k_dateRadius = 70 };
enum { k_dateSize = 12 };

enum { k_pathWidth = 2 };

const GPathInfo k_minutePathInfo =
{
    .num_points = 4,
    .points = (GPoint[]) {{ 0, -k_pathWidth }, { k_minuteRadius, -k_pathWidth }, { k_minuteRadius, k_pathWidth }, { 0, k_pathWidth }}
};
GPath* g_minutePath = NULL;

const GPathInfo k_hourPathInfo =
{
    .num_points = 4,
    .points = (GPoint[]) {{ 0, -k_pathWidth }, { k_hourRadius, -k_pathWidth }, { k_hourRadius, k_pathWidth }, { 0, k_pathWidth }}
};
GPath* g_hourPath = NULL;

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

GPoint getPoint(int minute, int radius)
{
    int angle = getAngle(minute);
    int cos = cos_lookup(angle);
    int sin = sin_lookup(angle);
    return GPoint(g_center.x + cos * radius / TRIG_MAX_RATIO,
                  g_center.y + sin * radius / TRIG_MAX_RATIO);
}

static void layerUpdateProc(Layer* layer, GContext* context)
{
    //APP_LOG(APP_LOG_LEVEL_DEBUG, "%d %d %d %d", cos, sin, p.x, p.y);

    graphics_context_set_stroke_color(context, k_dotColor);
    for (int h = 0; h < 12; ++h)
    {
        GPoint p = getPoint(h*5, k_dotRadius);
        graphics_draw_circle(context, p, k_dotSize);
    }

    time_t t = time(NULL);
    tm* tm = localtime(&t);
    int hour = tm->tm_hour % 12;
    int minute = tm->tm_min;
    // int date = g_colorIndex;//tm->tm_mday;
    int date = tm->tm_mday;
    static char dateBuf[3];
    snprintf(dateBuf, sizeof(dateBuf), "%d", date);

    if (!g_connected)
    {
        GPoint p = getPoint(30, 20);
        GRect rect = gbitmap_get_bounds(g_bluetoothBitmap);
        rect.origin.x = p.x - rect.size.w/2; 
        rect.origin.y = p.y - rect.size.h/2; 
        graphics_draw_bitmap_in_rect(context, g_bluetoothBitmap, rect);
    }

    if (g_batteryState.is_charging || g_batteryState.charge_percent < 30)
    {
        static char batteryBuf[5];
        snprintf(batteryBuf, sizeof(batteryBuf), "%d%%", g_batteryState.charge_percent);
        GPoint p = getPoint(0, 20);
        GRect rect = GRect(p.x - 30, p.y - 10, 60, 20);
        graphics_draw_text(context, batteryBuf, g_batteryFont, rect, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
    }

    // GColor8 k_hourColor = (GColor8){.argb=g_colors[g_colorIndex]};
    int hmin = hour*5 + minute/12;
    GPoint hourPoint = getPoint(hmin, k_hourRadius);
    graphics_context_set_fill_color(context, k_hourColor);
    graphics_fill_circle(context, hourPoint, k_hourSize);
    // graphics_context_set_stroke_color(context, k_hourColor);
    // graphics_draw_line(context, g_center, getPoint(hmin, k_hourRadius + 10));
    gpath_move_to(g_hourPath, g_center);
    gpath_rotate_to(g_hourPath, getAngle(hmin));
    gpath_draw_filled(context, g_hourPath);

    GPoint minutePoint = getPoint(minute, k_minuteRadius);
    graphics_context_set_fill_color(context, k_minuteColor);
    graphics_fill_circle(context, minutePoint, k_minuteSize);
    // graphics_context_set_stroke_color(context, k_minuteColor);
    // graphics_draw_line(context, g_center, getPoint(minute, k_minuteRadius + 10));
    gpath_move_to(g_minutePath, g_center);
    gpath_rotate_to(g_minutePath, getAngle(minute));
    gpath_draw_filled(context, g_minutePath);

    graphics_context_set_fill_color(context, k_bgColor);
    graphics_fill_circle(context, g_center, k_dotSize);
    graphics_context_set_stroke_color(context, k_dotColor);
    graphics_draw_circle(context, g_center, k_dotSize);

    GPoint datePoint = getPoint(45, k_dateRadius);
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
    layer_mark_dirty(g_layer);
}

static void batteryStateHandler(BatteryChargeState charge)
{
    g_batteryState = charge;
    layer_mark_dirty(g_layer);
}

static void clickHandler(ClickRecognizerRef recognizer, void* context)
{
    ButtonId button = click_recognizer_get_button_id(recognizer);
    if (button == BUTTON_ID_UP)
        --g_colorIndex;
    else if (button == BUTTON_ID_DOWN)
        ++g_colorIndex;
    const int numColors = sizeof(g_colors);
    if (g_colorIndex < 0) g_colorIndex = numColors-1;
    if (g_colorIndex >= numColors) g_colorIndex = 0;
    // APP_LOG(APP_LOG_LEVEL_DEBUG, "%d", g_colorIndex)
    layer_mark_dirty(g_layer);
}

static void click_config_provider(void* context)
{
    window_single_click_subscribe(BUTTON_ID_UP, clickHandler);
    window_single_click_subscribe(BUTTON_ID_DOWN, clickHandler);
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

    g_minutePath = gpath_create(&k_minutePathInfo);
    g_hourPath = gpath_create(&k_hourPathInfo);

    // subscribe to services
    tick_timer_service_subscribe(MINUTE_UNIT, &tickTimerHandler);
    bluetooth_connection_service_subscribe(bluetoothConnectionHandler);
    battery_state_service_subscribe(batteryStateHandler);

    window_set_click_config_provider(g_window, click_config_provider);

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
