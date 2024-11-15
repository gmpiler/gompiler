#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../common/IR.h"
#include "stackmaker.h"

#define DEBUG

extern void be_main(Funcs *func_head, FILE *dstfile);

/**
 * @brief ENTRY POINT of gompiler middle path
 */
void mp_main(Funcs *func_head, FILE *dstfile)
{
    printf("*** GOMPILER MIDDLE PATH ***\n");

    stackmaker(func_head);
    be_main(func_head, dstfile);
}