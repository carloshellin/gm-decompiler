#define GM_HEADER_POSITION 0x1E3660
#define GM_MAGIC_NUMBER 1234321
#define GM_TOTAL_EVENTS 11

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
    i32 begin;
    i32 end;
} gm_character_range_t;

typedef struct
{
    const char *name;
    const char *font_name;
    i32 size;
    i32 bold;
    i32 italic;
    gm_character_range_t character_range;
    gm_image_t image;
} gm_font_t;

typedef enum
{
    ACTION_NORMAL,
    ACTION_BEGINGROUP,
    ACTION_ENDGROUP,
    ACTION_ELSE,
    ACTION_EXIT,
    ACTION_REPEAT,
    ACTION_VARIABLE,
    ACTION_CODE,
    ACTION_PLACEHOLDER,
    ACTION_SEPARATOR,
    ACTION_LABEL
} gm_action_kind_t;

typedef enum
{
    ACTIONTYPE_NOTHING,
    ACTIONTYPE_FUNCTION,
    ACTIONTYPE_CODE
} gm_action_type_t;

typedef enum
{
    ARGUMENT_EXPRESSION,
    ARGUMENT_STRING,
    ARGUMENT_BOTH,
    ARGUMENT_BOOLEAN,
    ARGUMENT_MENU,
    ARGUMENT_SPRITE,
    ARGUMENT_SOUND,
    ARGUMENT_BACKGROUND,
    ARGUMENT_PATH,
    ARGUMENT_SCRIPT,
    ARGUMENT_OBJECT,
    ARGUMENT_ROOM,
    ARGUMENT_FONT,
    ARGUMENT_COLOR,
    ARGUMENT_TIMELINE,
    ARGUMENT_FONTSTRING
} gm_argument_kind_t;

typedef struct
{
    i32 lib_id;
    i32 id;
    gm_action_kind_t kind;
    i32 may_be_relative;
    i32 question;
    i32 applies_something;
    gm_action_type_t type;
    const char *name;
    const char *code;
    i32 arguments_used;
    gm_argument_kind_t *argument_kinds;
    i32 applies_object_index;
    i32 relative;
    char **arguments;
    i32 not_flag;
} gm_action_t;

typedef struct
{
    i32 position;
    gm_action_t *actions;
} gm_moment_t;

typedef struct
{
    const char *name;
    gm_moment_t *moments;
} gm_timeline_t;

typedef struct
{
    i32 value;
    i32 action_count;
    gm_action_t *actions;
} gm_event_t;

typedef struct
{
    const char *name;
    i32 sprite_index;
    i32 solid;
    i32 visible;
    i32 depth;
    i32 persistent;
    i32 parent_object_index;
    i32 mask_sprite_index;
    i32 reserved;
    gm_event_t *events[GM_TOTAL_EVENTS];
} gm_object_t;

typedef struct
{
    i32 visible;
    i32 foreground_image;
    i32 background_image_index;
    i32 x;
    i32 y;
    i32 tile_hor;
    i32 tile_vert;
    i32 hor_speed;
    i32 vert_speed;
    i32 stretch;
} gm_room_background_t;

typedef struct
{
    i32 visible;
    i32 x;
    i32 y;
    i32 width;
    i32 height;
    i32 port_x;
    i32 port_y;
    i32 port_w;
    i32 port_h;
    i32 hbor;
    i32 vbor;
    i32 hsp;
    i32 vsp;
    i32 object_following;
} gm_view_t;

typedef struct
{
    i32 x;
    i32 y;
    i32 object_index;
    i32 id;
    const char *creation_code;
    struct gm_instance_t *next;
} gm_instance_t;

typedef struct
{
    const char *name;
    const char *caption;
    i32 width;
    i32 height;
    i32 speed;
    i32 persistent;
    i32 background_color;
    i32 draw_background_color;
    const char *creation_code;
    gm_room_background_t *backgrounds;
    i32 enable_views;
    gm_view_t *views;
    gm_instance_t *instances;
} gm_room_t;

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
    gm_font_t *fonts;
    gm_timeline_t *timelines;
    gm_object_t *objects;
    gm_room_t *rooms;
    i32 id_last_instance_placed;
    i32 id_last_tile_placed;
    i32 *executable_rooms;
} gm_t;