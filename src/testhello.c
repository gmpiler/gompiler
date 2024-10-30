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
    int k;

    a = 0;
    j = 0;
    k = 0;

    for(i = 0; i < 10; i++) {
        for(k=0; k<10; k++){
            a = a + i;
            if(a > 0) {
                a =2;
            }
            a = 3;
        }
        a = 2;
    }
    while(j < 10){
        b = a + j;
        j = j + 1;
    }
    if(b > 0){
        a = 2;
    }
    
    return 0;
}

