void func(){
    int local;
    local = 2;
}

int main(void)
{
    int a;
    int b;
    int i;
    int j;

    a = 0;
    j = 0;

    for(i = 0; i < 10; i++) {
        a = a + i;
    }
    while(j < 10){
        b = a + j;
        j = j + 1;
    }
    return 0;
}

