#include <pthread.h>
#include "../cm.h"
#define OLIVEC_IMPLEMENTATION
#include "olive.c"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define TEX_DIV 1
#define TEX_SIZE (16/TEX_DIV)

Olivec_Canvas load_image(const char* filename) {
    int w, h, n;
    unsigned char* data = stbi_load(filename, &w, &h, &n, 0);
    if (data == NULL) {
        fprintf(stderr, "ERROR: Failed to open %s\n", filename);
        exit(1);
    };
    return olivec_canvas((uint32_t*)data, w, h, w);
}

struct preview_ret {
    bool err;
    float* times;
    size_t width;
    size_t height;
};

struct preview_ret preview(Grid grid, char* remake) {
    uint64_t width  = grid.width*TEX_SIZE;
    uint64_t height = grid.height*TEX_SIZE;

    float* times = (float*)malloc(6*sizeof(float));
    if (times == NULL) {
        fprintf(stderr, "ERROR: Failed to allocate times\n");
        return (struct preview_ret){.err=true};
    }
    
    clock_t start = clock();
    uint32_t* pixels = malloc(width*height*sizeof(uint32_t));
    if (pixels == NULL) {
        fprintf(stderr, "ERROR: Failed to allocate pixels\n");
        free(times);
        return (struct preview_ret){.err=true};
    }
    times[0] = (float)(clock()-start)/CLOCKS_PER_SEC;
    
    start = clock();
    Olivec_Canvas oc = olivec_canvas(pixels, width, height, width);
    times[1] = (float)(clock()-start)/CLOCKS_PER_SEC;
    
    start = clock();
    olivec_fill(oc, 0xFF2A2A2A);
    times[2] = (float)(clock()-start)/CLOCKS_PER_SEC;
    
    start = clock();
    Olivec_Canvas textures[11];
    if (strcmp(remake, "cmmm") == 0) {
        textures[0]  = load_image("textures/BGDefault.png");
        textures[1]  = load_image("textures/generator.png");
        textures[2]  = load_image("textures/CWspinner_alt.png");
        textures[3]  = load_image("textures/CCWspinner_alt.png");
        textures[4]  = load_image("textures/mover.png");
        textures[5]  = load_image("textures/slide.png");
        textures[6]  = load_image("textures/block.png");
        textures[7]  = load_image("textures/wall.png");
        textures[8]  = load_image("textures/enemy.png");
        textures[9]  = load_image("textures/trash.png");
        textures[10] = load_image("textures/BGPlaceable0.png");
    } else {
        textures[0]  = load_image("textures/indev/Grid.png");
        textures[1]  = load_image("textures/indev/Generator.png");
        textures[2]  = load_image("textures/indev/CW Rotator.png");
        textures[3]  = load_image("textures/indev/CCW Rotator.png");
        textures[4]  = load_image("textures/indev/Mover.png");
        textures[5]  = load_image("textures/indev/Slide.png");
        textures[6]  = load_image("textures/indev/Basic.png");
        textures[7]  = load_image("textures/indev/Wall.png");
        textures[8]  = load_image("textures/indev/Enemy.png");
        textures[9]  = load_image("textures/indev/Trash.png");
        textures[10] = load_image("textures/indev/placeable.png");
    }
    times[3] = (float)(clock()-start)/CLOCKS_PER_SEC;

    start = clock();
    for (size_t y=0; y<grid.height; y++) {
        for (size_t x=0; x<width; x++) {
            Cell cell = grid.cells[y*grid.width+x];
            if (cell.placeable == 1) olivec_sprite_copy(oc, x*TEX_SIZE, y*TEX_SIZE, TEX_SIZE, TEX_SIZE, textures[10]);    
            if (cell.type != 0)      olivec_sprite_copy(oc, x*TEX_SIZE, y*TEX_SIZE, TEX_SIZE, TEX_SIZE, textures[cell.type]);
        }
    }
    times[4] = (float)(clock()-start)/CLOCKS_PER_SEC;
    
    start = clock();
    const char* file = "preview.png";
    if (!stbi_write_png(file, width, height, 4, pixels, sizeof(uint32_t)*width)) {
        fprintf(stderr, "ERROR: could not write %s\n", file);
        free(times);
        return (struct preview_ret){.err=true};
    }
    times[5] = (float)(clock()-start)/CLOCKS_PER_SEC;
    free(pixels);
    for (size_t i=0; i<sizeof(textures)/sizeof(textures[0]); i++)
        stbi_image_free(textures[i].pixels);
    
    return (struct preview_ret) {
        .err = false,
        .times = times,
        .width = grid.width,
        .height = grid.height
    };
}

//#define PRINT_COLORS
//#define PRINT_COLOR_ROT

const char* stuff[CM_NCELLS][4] = {
    {" "," "," "," "},
    {"⇒","⇓","⇑","⇐"},
    {"↻","↻","↻","↻"},
    {"↺","↺","↺","↺"},
    {"→","↓","←","↑"},
    {"=","‖","=","‖"},
    {"#","#","#","#"},
    {"■","■","■","■"},
    {"×","×","×","×"},
    {"🗑","🗑","🗑","🗑"},
};

void print_grid(Grid grid) {
    #ifdef PRINT_COLORS
        bool b1 = true;
        bool b2 = true;
    #endif
    for (unsigned int y=0; y<grid.height; y++) {
        for (unsigned int x=0; x<grid.width; x++) {
            Cell cell = grid.cells[y*grid.width+x];
            #ifndef PRINT_COLORS
                printf("%s ", stuff[cell.type][cell.rot]);
            #else
                int color = 15;
                #ifdef PRINT_COLOR_ROT
                    switch (color = cell.rot) {
                        case 0:color=15;break;
                        case 2:color=10;break;
                        case 3:color=4; break;
                    }
                #else
                    switch (cell.type) {
                        case 1:color=10;break;
                        case 2:color=1; break;
                        case 3:color=4; break;
                        case 4:color=4; break;
                        case 5:color=11;break;
                        case 6:color=11;break;
                        case 8:color=1; break;
                        case 9:color=5; break;
                    }
                #endif
                b2=!b2;
                printf("\x1b[48;5;%im\x1b[38;5;%im%s \x1b[0m", cell.placeable?8:b2?232:235, color, stuff[cell.type][cell.rot]);
            #endif
        }
        #ifdef PRINT_COLORS
            b1=!b1;
            b2=b1;
        #endif
        printf("\n");
    }
}