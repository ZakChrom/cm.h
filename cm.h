#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#ifndef CM_NO_ERRORS
    #define CM_ERROR(err) fprintf(stderr, "[CM ERROR] %s\n", err)
#endif // CM_NO_ERRORS
#define CM_NCELLS 11
#define CM__ARGS Grid *grid,Cell self,unsigned int x, unsigned int y, bool pushed,Cell other
#define CM__VOID (void)grid;(void)self;(void)x;(void)y;(void)pushed;(void)other;
#define CM_CELL(type, rot, placeable) (Cell){type%CM_NCELLS, rot&0b11, placeable};
/* Cell struct
type : 4 = 
    0 aka empty |
    1 aka gen   |
    2 aka cw    |
    3 aka mover |
    4 aka ccw   |
    5 aka push  |
    6 aka wall  |
    7 aka enemy |
    8 aka trash |
    9 aka slide

rot : 2 = 
    0 aka right |
    1 aka down  |
    2 aka left  |
    3 aka up

placeable  : 1 = true | false

total bits is 8 aka 1 byte
*/
#ifndef CM_H
#define CM_H

typedef struct {
    uint type : 4;
    uint rot : 2;
    bool placeable;
} Cell;
typedef struct {
    Cell* cells;
    size_t width;
    size_t height;
} Grid;

Grid cm_create(uint width, uint height);
void cm_fill(Grid *grid, uint type, uint rot, bool placeable);
void cm_free(Grid grid);
void cm_step(Grid *grid, bool subtick);
void cm_set(Grid *grid, uint x, uint y, uint type, uint rot, bool placeable);

#ifdef CM_IMPLEMENTATION
Grid cm_create(uint width, uint height) {
    #ifndef CM_NO_ERRORS
        if (width < 1 || height < 1) {
            CM_ERROR("Invalid size");
            #ifdef CM_EXIT_ON_ERROR
                exit(1);
            #else
                return (Grid){NULL,0,0}; // For some reason you cant return NULL smh. I wish c had errors ;-;
            #endif
        }
    #endif // CM_NO_ERRORS
    Cell* cells = (Cell*)malloc(width*height*sizeof(Cell));
    if (cells == NULL) {
        #ifndef CM_NO_ERRORS
            CM_ERROR("Failed to allocate memory for grid");
        #endif // CM_NO_ERRORS
    }
    Grid grid = {cells, width, height};
    return grid;
}

void cm_free(Grid grid) {
    free(grid.cells);
}

void cm_set(Grid *grid, uint x, uint y, uint type, uint rot, bool placeable) {
    grid->cells[y*grid->width+x] = CM_CELL(type, rot, placeable);
}

void cm_fill(Grid *grid, uint type, uint rot, bool placeable) {
    Cell temp = CM_CELL(type, rot, placeable);
    for (size_t i=0; i<grid->width*grid->height; i++) {
        grid->cells[i] = temp;
    }
}

#endif // CM_IMPLEMENTATION
#endif // CM_H