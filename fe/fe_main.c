#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "decomposer.h"
#include "IR.h"

#define debug_mode 1 // if 1 then on
#define SIZE 256

/**
 * @brief ENTRY POINT of gompiler frontend
 *
 * @param argv[1]: input srcfile
 * @param argv[2]: output dstfile
 */
int main(int argc, char *argv[])
{
    printf("*** GOMPILER FRONT END ***\n");
/* --- INITIALIZE FILES --- */
    FILE *srcfile, *dstfile;
    char srcline[SIZE], dstline[SIZE];
    srcline[0] = '\0';
    dstline[0] = '\0';
    srcfile = fopen(argv[1], "r");
    dstfile = fopen(argv[2], "w");

/* --- OBTAIN CODE INFO --- */
    int codeline = 0;
    while(fgets(srcline, SIZE, srcfile) != NULL) {
        strcpy(code[codeline], srcline);
        codeline++;
    }

/* --- MODIFY CODE --- */
    decomposer(code);

/* --- PRINT MODIFIED CODE --- */
    for(int dstlineiter = 0; dstlineiter < codeline; dstlineiter++){
        fprintf(dstfile, code[dstlineiter]);
    }
    if(debug_mode == 1) printf("> IR's been generated\n");

    fclose(srcfile);
    fclose(dstfile);

    return 0;
}