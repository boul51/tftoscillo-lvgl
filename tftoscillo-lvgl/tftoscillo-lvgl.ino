#include <lvgl.h>
#include <TFT_eSPI.h>
#include <User_Setup.h>  // Include this to get TFT_WIDTH and TFT_HEIGHT

static const uint16_t g_screenWidth  = TFT_HEIGHT;
static const uint16_t g_screenHeight = TFT_WIDTH;

static lv_disp_draw_buf_t g_displayDrawBuffer;
static lv_color_t g_buffer[g_screenWidth * 10];

TFT_eSPI g_tft = TFT_eSPI(g_screenWidth, g_screenHeight);

#if LV_USE_LOG != 0
void my_print(const char * buf)
{
    SERIAL_IFACE.printf(buf);
    SERIAL_IFACE.flush();
}
#endif

void my_disp_flush( lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p )
{
    uint32_t w = ( area->x2 - area->x1 + 1 );
    uint32_t h = ( area->y2 - area->y1 + 1 );

    g_tft.startWrite();
    g_tft.setAddrWindow(area->x1, area->y1, w, h);
    g_tft.pushColors(reinterpret_cast<uint16_t *>(&color_p->full), w * h, true);
    g_tft.endWrite();

    lv_disp_flush_ready( disp );
}

void led_timer_callback(lv_timer_t* timer)
{
    lv_obj_t* led = reinterpret_cast<lv_obj_t*>(timer->user_data);
    lv_led_toggle(led);
}

void setup()
{
    while (!SERIAL_IFACE) {}

    SERIAL_IFACE.begin(115200);

    String lvglVersionString = String("lgvl v.") + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();

    SERIAL_IFACE.print("Starting initialization, ");
    SERIAL_IFACE.println(lvglVersionString);

    lv_init();

#if LV_USE_LOG != 0
    lv_log_register_print_cb( my_print ); /* register print function for debugging */
#endif

    g_tft.begin();
    g_tft.setRotation(3);  // Landscape orientation, flipped

    lv_disp_draw_buf_init(&g_displayDrawBuffer, g_buffer, NULL, g_screenWidth * 10);

    static lv_disp_drv_t dispDrv;
    lv_disp_drv_init( &dispDrv );
    dispDrv.hor_res = g_screenWidth;
    dispDrv.ver_res = g_screenHeight;
    dispDrv.flush_cb = my_disp_flush;
    dispDrv.draw_buf = &g_displayDrawBuffer;
    lv_disp_drv_register( &dispDrv );

    lv_obj_t *label = lv_label_create(lv_scr_act());
    lv_label_set_text(label, lvglVersionString.c_str());
    lv_obj_align(label, LV_ALIGN_CENTER, 0, -30);

    lv_obj_t *led = lv_led_create(lv_scr_act());
    lv_led_set_color(led, lv_color_hex(0xFF0000));
    lv_obj_align(led, LV_ALIGN_CENTER, 0, 30);

    lv_timer_create(led_timer_callback, 1000, led);

    SERIAL_IFACE.println("Initialization done");
}

void loop()
{
    static int prev_millis = millis();
    int cur_millis = millis();
    lv_tick_inc(cur_millis - prev_millis);
    prev_millis = cur_millis;

    lv_timer_handler();
}
