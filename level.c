#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "cm.h"

int split(const char *txt, char delim, char ***tokens) {
    int *tklen, *t, count = 1;
    char **arr, *p = (char *)txt;

    while (*p != '\0') if (*p++ == delim) count += 1;
    t = tklen = calloc(count, sizeof(int));
    for (p = (char *)txt; *p != '\0'; p++) *p == delim ? *t++ : (*t)++;
    *tokens = arr = malloc(count * sizeof(char *));
    t = tklen;
    p = *arr++ = calloc(*(t++) + 1, sizeof(char *));
    while (*txt != '\0') {
        if (*txt == delim) {
            p = *arr++ = calloc(*(t++) + 1, sizeof(char *));
            txt++;
        }
        else *p++ = *txt++;
    }
    free(tklen);
    return count;
}

Grid load_V1(char* level) {
    char** components = NULL;
    char** cells = NULL;
    char** bg_cells = NULL;
    split(level, ';', &components);
    Grid grid = cm_create(atoi(components[1]), atoi(components[2]));
    cm_fill(&grid, 0, 0, 0);
    
    if (strcmp(components[3], "") != 0) {
        int c = split(components[3], ',', &bg_cells);
        for (int i=0; i<c; i++) {
            char* bg_cell = bg_cells[i];
            char** contents = NULL;
            split(bg_cell, '.', &contents);
            cm_set(&grid, atoi(contents[0]), grid.height-1-atoi(contents[1]), 0, 0, true);
        }
    }

    if (strcmp(components[4], "") != 0) {
        int c = split(components[4], ',', &cells);
        for (int i=0; i<c; i++) {
            char* cell = cells[i];
            char** contents = NULL;
            split(cell, '.', &contents);
            int rot = atoi(contents[1]);
            cm_set(&grid, atoi(contents[2]), grid.height-1-atoi(contents[3]), atoi(contents[0])+1, rot, false);
        }
    }
    return grid;
}
//#define BYTEMASH

#ifdef BYTEMASH
#error ByteMash isnt finished

Grid load_ByteMash(char* level) {
    //convert the uncompressed bytes to a binary reader
    void* reader = b64decode(level);

    //read the header
    char flags = reader.readByte();

    //VRRRSXXX    V = Vault, R = Revision, X = Reserved, S = Short Precision Type
    uint isVault = (flags & 0x80) != 0;
    uint revision = (flags & 0x7C) >> 2;
    uint shortPrecision = flags & 0x3;

    //read the name
    char* name = reader.readString();
    char* dependMod = reader.readString();
    char* description = reader.readString();
    uint width = reader.readInt16();
    uint height = reader.readInt16();

    Grid grid = cm_create(width, height);

    size_t index = 0;
    while (true) {
        char header = reader.readByte();
        if (header == 0) break;

        if ((header & 0b10000000) != 0) {
            //cell run
            uint direction         = (header & 0b01110000) >> 4;
            bool cellTypePrecision = (header & 0b00001000) != 0;
            bool noLength          = (header & 0b00000100) != 0;
            bool precision         = (header & 0b00000010) != 0;
            bool dragSpot          = (header & 0b00000001) != 0;

            uint16_t cellType = cellTypePrecision ? reader.readUInt16() : reader.readByte();
            uint16_t length = noLength ? 1 : (precision ? reader.readUInt16() : reader.readByte());

            for (size_t i=0; i<length; i++) {
                grid.cells[index] = (Cell){
                    .placeable=dragSpot,
                    .rot=direction,
                    .type=cellType
                };
                index++;
            }
        } else {
            //blank space run
            bool precision = (header & 0b00001000) != 0;
            bool noLength  = (header & 0b00000100) != 0;
            bool dragSpot  = (header & 0b00000001) != 0;

            uint16_t length = noLength ? 1 : (precision ? reader.readUInt16() : reader.readByte());
            for (size_t i=0; i<length; i++) {
                grid.cells[index] = (Cell){
                    .placeable=dragSpot,
                    .rot=0,
                    .type=grid.cells[index].type
                };
                index++;
            }
        }
    }

    return grid;
}
#endif // BYTEMASH
                
// Grid load_V2(char* level) {
    
// }

// Grid load_V3(char* level) {
//     char** components = NULL;
//     char** cells = NULL;
//     char** bg_cells = NULL;
//     split(level, ';', &components);
//     Grid grid = cm_create(decode_string(components[1]), decode_string(components[2]));
//     cm_fill(&grid, 0, 0, 0);
    
//     char* distance, offset, level_cells;
//     size_t data_index = 0;
//     char* data = components[3];
//     while (data_index < len(data)) {
//         if (strcmp(&data[data_index], "(") == 0 || strcmp(&data[data_index], ")") == 0) {
//             if (strcmp(&data[data_index], ")") == 0) {
//                 offset = data[data_index + 1];
//                 distance = &data[data_index + 2];
//                 data_index += 3;
//             } else {
//                 offset = "";
//                 data_index += 1;
//                 while (strcmp(&data[data_index], "(") != 0 && strcmp(&data[data_index], ")") != 0) {
//                     offset += data[data_index];
//                     data_index += 1;
//                 }
//                 if (strcmp(&data[data_index], ")") == 0) {
//                     distance = data[data_index + 1];
//                     data_index += 2;
//                 } else {
//                     distance = "";
//                     data_index += 1;
//                     while (strcmp(&data[data_index], ")") == 0) {
//                         distance += data[data_index];
//                         data_index += 1;
//                     }
//                     data_index += 1;
//                 }
//             }
//             for (int d=0; d<decode_string(distance); d++)
//                 level_cells += level_cells[-decode_string(offset) - 1];
//         } else {
//             level_cells += data[data_index];
//             data_index += 1;
//         }
//     }
//     for (int i=0; i<len(level_cells); i++)
//         set_cell(decode_string(level_cells[i]), i)
//     return grid
// }

struct level_ret {
    Grid grid;
    bool err;
};

struct level_ret load(char* level) {
    char** components = NULL;
    split(level, ';', &components);
    if (strcmp(components[0], "V1") == 0)              return (struct level_ret){.grid=load_V1(level),.err=false};
    #ifdef BYTEMASH
    else if (strcmp(components[0], "|ByteMash|") == 0) return (struct level_ret){.grid=load_ByteMash(level),.err=false};
    #endif // BYTEMASH
    else                                               return (struct level_ret){.err=true};
    // else if (strcmp(components[0], "V2") == 0) return load_V2(level);
    // else if (strcmp(components[0], "V3") == 0) return load_V3(level);
}