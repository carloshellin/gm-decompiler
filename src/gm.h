#define GM_HEADER_POSITION 0x1E3660
#define GM_MAGIC_NUMBER 1234321

typedef struct
{
    i32 size;
    u8 *data;
} gm_file_t;

typedef struct
{
    const char *name;
    const char *value;
} gm_constant_t;

typedef struct
{
    i32 magic_number;
    i32 debug_flag;
    i32 fullscreen;
    i32 interpolate;
    i32 dont_draw_border;
    i32 display_cursor;
    i32 scaling;
    i32 allow_window_resize;
    i32 on_top;
    i32 color_outside_room;
    i32 set_resolution;
    i32 color_depth;
    i32 resolution;
    i32 frequency;
    i32 dont_show_buttons;
    i32 vsync;
    i32 let_F4;
    i32 let_F1;
    i32 let_esc;
    i32 let_F5;
    i32 let_F9;
    i32 treat_close_as_esc;
    i32 process_priority;
    i32 freeze_looses_focus;
    i32 loading_progress_bar;
    i32 show_custom_image;
    gm_file_t *custom_image;
    i32 transparent;
    i32 translucency;
    i32 scale_progress_bar;
    i32 error_display;
    i32 error_log;
    i32 error_abort;
    i32 treat_as_zero;
    gm_constant_t *constants;
} gm_t;