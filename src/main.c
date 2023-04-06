#define _CRT_SECURE_NO_DEPRECATE
#include <stdio.h>
#include <stdlib.h>

#include "types.h"

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

#include "gm.c"

int main(int argc, char *argv[])
{
    if (argc > 1)
    {
        const char *filename = argv[1];
        FILE* fp = fopen(filename, "rb");
        if (!fp)
        {
            perror("File opening failed");
            exit(EXIT_FAILURE);
        }
        
        fseek(fp, 0L, SEEK_END);
        long filesize = ftell(fp);
        fseek(fp, 0L ,SEEK_SET);
        u8 *buffer = malloc(filesize);
        fread(buffer, sizeof(u8), filesize, fp);
        
        fclose(fp);
        
        gm_decompiler(buffer);
    }
    else
    {
        fprintf(stderr, "No file specified to decompile");
    }
    
    return EXIT_SUCCESS;
}