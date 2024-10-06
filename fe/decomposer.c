#include <stdio.h>
#include "decomposer.h"

#define SIZE 256
/**
 *  @brief ENTRY POINT
 *  modify codes here
 */
void decomposer(char code[][SIZE])
{
    printf("--- decomposer ---\n");
    char tmp[] = "modified\n";
    for(int i=0; i<4; i++){
        strcpy(code[i], tmp);
    }

}