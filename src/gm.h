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

typedef enum
{
    SOUND_NORMAL,
    SOUND_BACKGROUND,
    SOUND_3D,
    SOUND_MULTIMEDIA
} gm_sound_kind_t;

typedef enum
{
    SOUNDEFFECTS_CHORUS = 1,
    SOUNDEFFECTS_ECHO = 2,
    SOUNDEFFECTS_FLANGER = 4,
    SOUNDEFFECTS_GARGLE = 8,
    SOUNDEFFECTS_REVERB = 16
} gm_sound_effects_t;

typedef struct
{
    const char *name;
    gm_sound_kind_t kind;
    const char *filetype;
    const char *filename;
    gm_file_t *file;
    gm_sound_effects_t effects;
    f64 volume;
    f64 pan;
    i32 preload;
} gm_sound_t;

typedef enum
{
    BOUNDINGBOX_AUTOMIC,
    BOUNDINGBOX_FULLIMAGE,
    BOUNDINGBOX_MANUAL
} gm_bounding_box_kind_t;

typedef struct
{
    i32 left;
    i32 right;
    i32 bottom;
    i32 top;
} gm_bounding_box_t;

typedef struct
{
    i32 x;
    i32 y;
} gm_origin_t;

typedef struct
{
    i32 width;
    i32 height;
    u8 bytes_per_pixel;
    gm_file_t *file;
} gm_image_t;

typedef struct
{
    const char *name;
    i32 width;
    i32 height;
    gm_bounding_box_t bounding_box;
    i32 transparent;
    i32 smooth_edges;
    i32 preload_texture;
    gm_bounding_box_kind_t bounding_box_kind;
    i32 precise_collision_checking;
    gm_origin_t origin;
    gm_image_t *subimages;
} gm_sprite_t;

typedef struct
{
    const char *name;
    i32 width;
    i32 height;
    i32 transparent;
    i32 smooth_edges;
    i32 preload_texture;
    i32 use_as_tileset;
    gm_image_t image;
} gm_background_t;

typedef enum
{
    CONNECTION_STRAIGHTLINES,
    CONNECTION_SMOOTHCURVE
} gm_path_connection_kind_t;

typedef struct
{
    f64 x;
    f64 y;
    f64 speed;
} gm_point_t;

typedef struct
{
    const char *name;
    gm_path_connection_kind_t connection_kind;
    i32 closed;
    i32 precision;
    gm_point_t *points;
} gm_path_t;

typedef struct
{
    const char *name;
    gm_file_t *file;
} gm_script_t;

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
    gm_sound_t *sounds;
    gm_sprite_t *sprites;
    gm_background_t *backgrounds;
    gm_path_t *paths;
    gm_script_t *scripts;
} gm_t;