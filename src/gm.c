#include "gm.h"
#include "miniz.c"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

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
    string[string_length] = '\0';
    
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

static gm_t * gm_decompiler(u8 *buffer)
{
    i32 *at = (i32 *) (buffer + GM_HEADER_POSITION);
    
    gm_t *gm = malloc(sizeof(gm_t));
    
    gm->magic_number = *at++;
    if (gm->magic_number != GM_MAGIC_NUMBER)
    {
        fprintf(stderr, "Executable not contain a valid magic number");
        exit(EXIT_FAILURE);
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
    
    gm_file_t *resources = gm_decompress_data(&at);
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

            arrput(gm->sounds, sound);
        }
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

            arrput(gm->sprites, sprite);
        }
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

            arrput(gm->backgrounds, background);
        }
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

            arrput(gm->paths, path);
        }
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

            arrput(gm->scripts, script);
        }
    }

    at++; // Version
    i32 font_ids = *at++;
    gm->fonts = 0;
    for (usize i = 0; i < font_ids; i++)
    {
        gm_font_t font = {0};
        i32 font_exists = *at++;
        if (font_exists)
        {
            font.name = gm_load_string(&at);
            
            at++; // Version
            font.font_name = gm_load_string(&at);
            font.size = *at++;
            font.bold = *at++;
            font.italic = *at++;
            font.character_range.begin = *at++;
            font.character_range.end = *at++;
            at += 6144 / sizeof(i32); // Unknown
            
            font.image.width = *at++;
            font.image.height = *at++;
            font.image.bytes_per_pixel = 1;
            font.image.file = gm_decompress_data(&at);

            arrput(gm->fonts, font);
        }
    }
    
    at++; // Version
    i32 timeline_ids = *at++;
    gm->timelines = 0;
    for (usize i = 0; i < timeline_ids; i++)
    {
        gm_timeline_t timeline = {0};
        i32 timeline_exists = *at++;
        if (timeline_exists)
        {
            timeline.name = gm_load_string(&at);
            at++; // Version
            i32 moments_count = *at++;
            for (usize i = 0; i < moments_count; i++)
            {
                gm_moment_t moment = {0};
                moment.position = *at++;
                at++; // Version
                at++; // ActionsCount (1)
                at++; // Version
                gm_action_t action = {0};
                action.lib_id = *at++;
                action.id = *at++;
                action.kind = (gm_action_kind_t) *at++;
                action.may_be_relative = *at++;
                action.question = *at++;
                action.applies_something = *at++;
                action.type = (gm_action_type_t) *at++;
                
                action.name = gm_load_string(&at);
                action.code = gm_load_string(&at);
                
                action.arguments_used = *at++;
                i32 argument_kinds_count = *at++;
                for (usize i = 0; i < argument_kinds_count; i++)
                {
                    gm_argument_kind_t argument_kind = (gm_argument_kind_t) *at++;
                    arrput(action.argument_kinds, argument_kind);
                }
                action.applies_object_index = *at++;
                action.relative = *at++;
                i32 arguments_count = *at++;
                
                for (usize i = 0; i < arguments_count; i++)
                {
                    const char *argument = gm_load_string(&at);
                    arrput(action.arguments, (char *) argument);
                }
                
                action.not_flag = *at++;
                
                arrput(moment.actions, action);
                arrput(timeline.moments, moment);
            }

            arrput(gm->timelines, timeline);
        }
        
    }
    
    at++; // Version
    i32 object_ids = *at++;
    gm->objects = 0;
    for (usize i = 0; i < object_ids; i++)
    {
        gm_object_t object = {0};
        i32 object_exists = *at++;
        if (object_exists)
        {
            object.name = gm_load_string(&at);
            at++; // Version
            object.sprite_index = *at++;
            object.solid = *at++;
            object.visible = *at++;
            object.depth = *at++;
            object.persistent = *at++;
            object.parent_object_index = *at++;
            object.mask_sprite_index = *at++;
            object.reserved = *at++;
            
            i32 total_events = object.reserved + 1;
            for (usize event_index = 0; event_index < total_events; 
                 event_index++)
            {
                while (at[0] >= 0)
                {
                    gm_event_t event = {0};
                    event.value = *at++;
                    at++; // Version
                    i32 action_count = *at++;
                    event.action_count = action_count;
                    while (action_count--)
                    {
                        gm_action_t action = {0};
                        at++; // Version
                        action.lib_id = *at++;
                        action.id = *at++;
                        action.kind = (gm_action_kind_t) *at++;
                        action.may_be_relative = *at++;
                        action.question = *at++;
                        action.applies_something = *at++;
                        action.type = (gm_action_type_t) *at++;
                        action.name = gm_load_string(&at);
                        action.code = gm_load_string(&at);
                        
                        action.arguments_used = *at++;
                        i32 argument_kinds_count = *at++;
                        for (usize i = 0; i < argument_kinds_count; i++)
                        {
                            gm_argument_kind_t argument_kind = (gm_argument_kind_t) *at++;
                            arrput(action.argument_kinds, argument_kind);
                        }
                        
                        action.applies_object_index = *at++;
                        action.relative = *at++;
                        
                        i32 arguments_count = *at++;
                        for (usize i = 0; i < arguments_count; i++)
                        {
                            const char *argument = gm_load_string(&at);
                            arrput(action.arguments, (char *) argument);
                        }
                        
                        action.not_flag = *at++;
                        
                        arrput(event.actions, action);
                    }
                    
                    arrput(object.events[event_index], event);
                }
                at++;
            }

            arrput(gm->objects, object);
        }
    }
    
    at++; // Version
    i32 room_ids = *at++;
    gm->rooms = 0;
    for (usize i = 0; i < room_ids; i++)
    {
        gm_room_t room = {0};
        i32 room_exists = *at++;
        if (room_exists)
        {
            room.name = gm_load_string(&at);
            at++; // Version
            room.caption = gm_load_string(&at);
            room.width = *at++;
            room.height = *at++;
            room.speed = *at++;
            room.persistent = *at++;
            room.background_color = *at++;
            room.draw_background_color = *at++;
            room.creation_code = gm_load_string(&at);
            
            i32 backgrounds_count = *at++;
            for (usize i = 0; i < backgrounds_count; i++)
            {
                gm_room_background_t background = {0};
                background.visible = *at++;
                background.foreground_image = *at++;
                background.background_image_index = *at++;
                background.x = *at++;
                background.y = *at++;
                background.tile_hor = *at++;
                background.tile_vert = *at++;
                background.hor_speed = *at++;
                background.vert_speed = *at++;
                background.stretch = *at++;
                
                arrput(room.backgrounds, background);
            }
            
            room.enable_views = *at++;
            i32 views_count = *at++;
            for (usize i = 0; i < views_count; i++)
            {
                gm_view_t view = {0};
                view.visible = *at++;
                view.x = *at++;
                view.y = *at++;
                view.width = *at++;
                view.height = *at++;
                view.port_x = *at++;
                view.port_y = *at++;
                view.port_w = *at++;
                view.port_h = *at++;
                view.hbor = *at++;
                view.vbor = *at++;
                view.hsp = *at++;
                view.vsp = *at++;
                view.object_following = *at++;
                
                arrput(room.views, view);
            }
            
            i32 instances_count = *at++;
            while (instances_count--)
            {
                gm_instance_t instance = {0};
                instance.x = *at++;
                instance.y = *at++;
                instance.object_index = *at++;
                instance.id = *at++;
                instance.creation_code = gm_load_string(&at);
                
                arrput(room.instances, instance);
            }
            
            // TODO: Tiles
            at++; // TilesCount

            arrput(gm->rooms, room);
        }
    }
    
    gm->id_last_instance_placed = *at++;
    gm->id_last_tile_placed = *at++;
    
    at++; // Version
    // TODO: Includes
    at++; // IncludesCount
    
    at++; // VersionGameInformation
    at++; // BackgroundColor
    at++; // MimicWindowForm
    gm_skip_string(&at); // FormCaption
    at++; // PositionLeft
    at++; // PositionTop
    at++; // PositionWidth
    at++; // PositionHeight
    at++; // ShowWindowBorderAndCaption
    at++; // AllowResize
    at++; // AlwaysOnTop
    at++; // StopGame
    gm_skip_string(&at); // GameInformationRTF
    
    at++; // Version
    // TODO: Libraries
    i32 libraries_count = *at++;
    at += libraries_count;
    
    at++; // Version
    i32 executable_rooms_count = *at++;
    gm->executable_rooms = 0;
    for (usize i = 0; i < executable_rooms_count; i++)
    {
        i32 executable_room = *at++;
        arrput(gm->executable_rooms, executable_room);
    }

    free(resources);
    free(buffer);
    
    return gm;
}

static void gm_save_files(const char *filename, gm_t *gm)
{
    char *outfilename = malloc(strlen(filename) + 1);
    strcpy(outfilename, filename);
    char *dot = strrchr(outfilename, '.');
    if (dot) *dot = '\0';

    // TODO: This only works on Windows    
    CreateDirectoryA(outfilename, NULL);
    SetCurrentDirectory(outfilename);
    CreateDirectoryA("sounds", NULL);
    CreateDirectoryA("sprites", NULL);

    FILE *constants = fopen("constants.txt", "w");
    for (usize i = 0; i < arrlenu(gm->constants); i++)
    {
        gm_constant_t constant = gm->constants[i];
        fprintf(constants, "name: %s, value: %s\n",
                constant.name, constant.value);
    }
    fclose(constants);

    FILE *sounds = fopen("sounds.txt", "w");
    for (usize i = 0; i < arrlenu(gm->sounds); i++)
    {
        gm_sound_t sound = gm->sounds[i];
        fprintf(sounds, "name: %s, kind: %d, filetype: %s, filename: %s, effects: %d, volume: %f, pan: %f, preload: %d\n",
                sound.name, sound.kind, sound.filetype, sound.filename,
                sound.effects, sound.volume, sound.pan, sound.preload);

        char *sound_filename = malloc(strlen("sounds") + strlen(sound.filename) + 2);
        sprintf(sound_filename, "sounds/%s", sound.filename);
        FILE *sound_file = fopen(sound_filename, "wb");
        fwrite(sound.file->data, sound.file->size, 1, sound_file);
        fclose(sound_file);
        free(sound_filename);
    }
    fclose(sounds);

    stbi_write_tga_with_rle = 0;
    FILE *sprites = fopen("sprites.txt", "w");
    for (usize i = 0; i < arrlenu(gm->sprites); i++)
    {
        gm_sprite_t sprite = gm->sprites[i];
        fprintf(sprites, "name: %s, width: %d, height: %d, transparent: %d, smooth_edges: %d, preload_texture: %d, bounding_box_kind: %d, origin_x: %d, origin_y: %d\n",
                sprite.name, sprite.width, sprite.height, sprite.transparent,
                sprite.smooth_edges, sprite.preload_texture, 
                sprite.bounding_box_kind, sprite.origin.x, sprite.origin.y);

        char *subimage_filename = malloc(strlen("sprites") + strlen(sprite.name) + 10);
        for (usize j = 0; j < arrlenu(sprite.subimages); j++)
        {
            gm_image_t subimage = sprite.subimages[j];
            sprintf(subimage_filename, "sprites/%s_%zd.tga", sprite.name, j);
            stbi_write_tga(subimage_filename, subimage.width, subimage.height, subimage.bytes_per_pixel, subimage.file->data);
        }
        free(subimage_filename);
    }
    fclose(sprites);

    free(outfilename);
    free(gm);
}