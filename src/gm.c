#include "gm.h"
#include "miniz.c"

static gm_file_t * gm_decompress_data(i32 **at)
{
    i32 size = *at[0]++;
    i32 data_size = 5 * 1024 * 1024; // 5 MB
    u8 *data = malloc(data_size);
    
    u8 *compressed_data = (u8 *) at[0];
    at[0] = (i32 *) (compressed_data + size);
    
    mz_stream stream = {0};
    stream.next_in = compressed_data;
    stream.avail_in = size;
    stream.next_out = data;
    stream.avail_out = data_size;
    mz_inflateInit(&stream);
    int result = mz_inflate(&stream, MZ_NO_FLUSH);
    if (result != MZ_OK && result != MZ_STREAM_END)
    {
        fprintf(stderr, "mz_inflate() failed to decompress data: %s\n", mz_error(result));
    }
    mz_inflateEnd(&stream);
    
    gm_file_t *file = malloc(sizeof(gm_file_t));
    file->size = stream.total_out;
    file->data = malloc(file->size);
    memcpy(file->data, data, file->size);
    
    free(data);
    
    return file;
}

static const char * gm_load_string(i32 **at)
{
    i32 string_length = *at[0]++;
    if (!string_length) return "";
    
    char *string = malloc(string_length + 1);
    
    char *c = (char *) at[0];
    at[0] = (i32 *) (c + string_length);
    
    memcpy(string, c, string_length);
    
    return string;
}

static void gm_skip_string(i32 **at)
{
    i32 string_length = *at[0]++;
    
    char *c = (char *) at[0];
    at[0] = (i32 *) (c + string_length);
    
    while (string_length--)
    {
        c++;
    }
}

static i32 * gm_decrypt_data(const i32 seed, const i32 data_size, u8 *encrypted_data, i32 diff_position)
{
    i32 swap_table[2][256] = {0};
    i32 lower_seed = 6 + (seed % 250);
    i32 upper_seed = seed / 250;
    
    for (i32 i = 0; i < 256; i++)
    {
        swap_table[0][i] = i;
    }
    for (usize i = 1; i < 10001; i++)
    {
        i32 table_index = 1 + ((i * lower_seed + upper_seed) % 254);
        i32 value = swap_table[0][table_index];
        swap_table[0][table_index] = swap_table[0][table_index + 1];
        swap_table[0][table_index + 1] = value;
    }
    for (i32 i = 1; i < 256; i++)
    {
        swap_table[1][swap_table[0][i]] = i;
    }
    
    for (usize byte = 0; byte < data_size; byte++) 
    {
        encrypted_data[byte] =
            (swap_table[1][(i32) encrypted_data[byte]] - diff_position) 
            & 0xFF;
        if (diff_position) diff_position++;
    }
    
    return (i32 *) (encrypted_data + sizeof(i32));
}

static void gm_decompiler(const u8 *buffer)
{
    i32 *at = (i32 *) (buffer + GM_HEADER_POSITION);
    
    gm_t *gm = malloc(sizeof(gm_t));
    
    gm->magic_number = *at++;
    if (gm->magic_number != GM_MAGIC_NUMBER)
    {
        fprintf(stderr, "Executable not contain a valid magic number");
    }

    at++; // Version
    gm->debug_flag = *at++;

    at++; // Version
    gm->fullscreen = *at++;
    gm->interpolate = *at++;
    gm->dont_draw_border = *at++;
    gm->display_cursor = *at++;
    gm->scaling = *at++;
    gm->allow_window_resize = *at++;
    gm->on_top = *at;
    gm->color_outside_room = *at++;
    gm->set_resolution = *at++;
    gm->color_depth = *at++;
    gm->resolution = *at++;
    gm->frequency = *at++;
    gm->dont_show_buttons = *at++;
    gm->vsync = *at++;
    gm->let_F4 = *at++;
    gm->let_F4 = *at++; // TODO: Duplicate?
    gm->let_F1 = *at++;
    gm->let_esc = *at++;
    gm->let_F5 = *at++;
    gm->let_F9 = *at++;
    gm->treat_close_as_esc = *at++;
    gm->process_priority = *at++;
    gm->freeze_looses_focus = *at++;

    gm->loading_progress_bar = *at++;
    // TODO: if Loading Progress Bar == 2
    
    gm->show_custom_image = *at++;
    if (gm->show_custom_image)
    {
        gm->custom_image = gm_decompress_data(&at);
    }

    gm->transparent = *at++;
    gm->translucency = *at++;
    gm->scale_progress_bar = *at++;
    gm->error_display = *at++;
    gm->error_log = *at++;
    gm->error_abort = *at++;
    gm->treat_as_zero = *at++;

    i32 total_constants = *at++;
    gm->constants = 0;
    for (usize i = 0; i < total_constants; i++)
    {
        gm_constant_t constant = {0};
        constant.name = gm_load_string(&at);
        constant.value = gm_load_string(&at);
        arrput(gm->constants, constant);
    }

    gm_skip_string(&at);
    
    i32 dll_size = *at++;
    at += (dll_size / sizeof(i32));
    
    const gm_file_t *resources = gm_decompress_data(&at);
    at = (i32 *) resources->data;
    
    i32 seed_position = *at++;
    i32 encrypted_data_position = *at++;
    
    at += seed_position;
    i32 seed = *at++;
    
    u8 *encrypted_data = (u8 *) (at + encrypted_data_position);
    
    u8 *end_data = (resources->data + resources->size);
    i32 data_size = (i32) (end_data - encrypted_data);
    
    i32 diff_position = (i32) (encrypted_data - resources->data);
    
    at = gm_decrypt_data(seed, data_size, encrypted_data, diff_position);
    
    at++; // GameID
    at += sizeof(i32); // Unclear
    at++; // Version
    at++; // Unknown
    
    at++; // Version
    i32 sound_ids = *at++;
    gm->sounds = 0;
    for (usize i = 0; i < sound_ids; i++)
    {
        gm_sound_t sound = {0};
        i32 sound_exists = *at++;
        if (sound_exists)
        {
            sound.name = gm_load_string(&at);
            
            at++; // Version
            sound.kind = (gm_sound_kind_t) *at++;
            sound.filetype = gm_load_string(&at);
            sound.filename = gm_load_string(&at);
            
            i32 music_exists = *at++;
            if (music_exists)
            {
                sound.file = gm_decompress_data(&at);
            }
            
            sound.effects = (gm_sound_effects_t) *at++;
            sound.volume = *(f64 *) at;
            at += sizeof(i16);
            sound.pan = *(f64 *) at;
            at += sizeof(i16);
            sound.preload = *at++;
        }
        
        arrput(gm->sounds, sound);
    }

    at++; // Version
    i32 sprite_ids = *at++;
    gm->sprites = 0;
    for (usize i = 0; i < sprite_ids; i++)
    {
        gm_sprite_t sprite = {0};
        i32 sprite_exists = *at++;
        if (sprite_exists)
        {
            sprite.name = gm_load_string(&at);
            
            at++; // Version
            sprite.width = *at++;
            sprite.height = *at++;
            sprite.bounding_box.left = *at++;
            sprite.bounding_box.right = *at++;
            sprite.bounding_box.bottom = *at++;
            sprite.bounding_box.top = *at++;
            sprite.transparent = *at++;
            sprite.smooth_edges = *at++;
            sprite.preload_texture = *at++;
            sprite.bounding_box_kind = (gm_bounding_box_kind_t) *at++;
            sprite.precise_collision_checking = *at++;
            sprite.origin.x = *at++;
            sprite.origin.y = *at++;
            
            i32 subimages = *at++;
            while (subimages--)
            {
                at++; // Version
                i32 subimage_exists = *at++;
                if (subimage_exists)
                {
                    gm_image_t subimage = {0};
                    subimage.width = *at++;
                    subimage.height = *at++;
                    subimage.bytes_per_pixel = 4;
                    subimage.file = gm_decompress_data(&at);
                    
                    arrput(sprite.subimages, subimage);
                }
            }
        }
        
        arrput(gm->sprites, sprite);
    }

    at++; // Version
    i32 background_ids = *at++;
    gm->backgrounds = 0;
    for (usize i = 0; i < background_ids; i++)
    {
        gm_background_t background = {0};
        i32 background_exists = *at++;
        if (background_exists)
        {
            background.name = gm_load_string(&at);
            
            at++; // Version
            background.width = *at++;
            background.height = *at++;
            background.transparent = *at++;
            background.smooth_edges = *at++;
            background.preload_texture = *at++;
            background.use_as_tileset = *at++;
            
            at++; // Version
            i32 image_exists = *at++;
            if (image_exists)
            {
                background.image.width = *at++;
                background.image.height = *at++;
                background.image.bytes_per_pixel = 4;
                background.image.file = gm_decompress_data(&at);
            }
        }
        
        arrput(gm->backgrounds, background);
    }

    at++; // Version
    i32 path_ids = *at++;
    gm->paths = 0;
    for (usize i = 0; i < path_ids; i++)
    {
        gm_path_t path = {0};
        i32 path_exists = *at++;
        if (path_exists)
        {
            path.name = gm_load_string(&at);
            at++; // Version
            path.connection_kind = (gm_path_connection_kind_t) *at++;
            path.closed = *at++;
            path.precision = *at++;
            i32 points_count = *at++;
            while (points_count--)
            {
                gm_point_t point = {0};
                point.x = *(f64 *) at;
                at += sizeof(i16);
                point.y = *(f64 *) at;
                at += sizeof(i16);
                point.speed = *(f64 *) at;
                at += sizeof(i16);
                
                arrput(path.points, point);
            }
        }
        
        arrput(gm->paths, path);
    }

    at++; // Version
    i32 script_ids = *at++;
    gm->scripts = 0;
    for (usize i = 0; i < script_ids; i++)
    {
        gm_script_t script = {0};
        i32 script_exists = *at++;
        if (script_exists)
        {
            script.name = gm_load_string(&at);
            at++; // Version
            
            script.file = gm_decompress_data(&at);
            i32 seed = 12345;
            i32 diff_position = 0;
            gm_decrypt_data(seed, script.file->size,
                          script.file->data, diff_position);
        }
        
        arrput(gm->scripts, script);
    }
}