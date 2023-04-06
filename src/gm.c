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

static void gm_decompiler(u8 *buffer)
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

    i32 totalConstants = *at++;
    gm->constants = 0;
    for (usize i = 0; i < totalConstants; i++)
    {
        gm_constant_t constant = {0};
        constant.name = gm_load_string(&at);
        constant.value = gm_load_string(&at);
        arrput(gm->constants, constant);
    }
    
}