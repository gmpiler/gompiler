int main(void)
{
    int i;
    int j;
    int a;
    a = 0;
    for(i = 0; i < 2; i++) {
        for(j = 0; j < 2; j++){
            a = a + i;
        }
    }
    return a;
}