#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "decomposer.h"
#include "../common/IR.h"

#define DEBUG
#define SIZE 256

extern void mp_main(Funcs *func_head, FILE *dstfile);

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
    /**
     * @brief code配列にソースコードをコピーしていく
     */
    while(fgets(srcline, SIZE, srcfile) != NULL) {
        strcpy(code[codeline], srcline);
        codeline++;
    }

/* --- TOKENIZER --- */
    Funcs   *func_head = (Funcs*)malloc(sizeof(Funcs));
    func_head->type = F_ROOT;
    decomposer(code, codeline, func_head);

/* --- PRINT MODIFIED CODE --- */
    // stackmachine_emulator_x86_64(func_head, dstfile);

    /**
     * TODO: 後々，codeにはソースと1対1な中間表現が格納され，ミドルパスに渡される予定
     */
    // for(int dstlineiter = 0; dstlineiter < codeline; dstlineiter++){
    //     fprintf(dstfile, code[dstlineiter]);
    // }

    mp_main(func_head, dstfile);

    fclose(srcfile);
    fclose(dstfile);

    return 0;
}
