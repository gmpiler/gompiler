void func(){
    int local;
    local = 2;
}

int main(void) /* function <main> */
{
    /* BASIC Block */
    int a;
    int b;
    int i;
    int j;
    int k;

    a = 0;
    j = 0;
    k = 0;

    /* FOR Block */
    for(i = 0; i < 10; i++) {
        /* BASIC Block(of "FOR") */
        /* FOR Block */
        for(k=0; k<10; k++){
            /* BASIC Block(of "FOR") */
            a = a + i;
            /* IF Block */
            if(a > 0) {
                /* BASIC Block(of "if") */
                a =2;
            }
            /* BASIC Block */
            a = 1 + 2 * 3;
            a = 3 * (1 + 2);
        }
        /* BASIC Block */
        a = 2;
    }
    /* WHILE Block */
    while(j < 10){
        /* BASIC Block(of "WHILE") */
        b = a + j * 2;
        j = j + 1;
    }
    /* IF Block */
    if(b > 0){
        /* BASIC Block(of "IF") */
        a = 2;
    }

    /* BASIC Block */
    return 0;
}
