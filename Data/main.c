#include <stdio.h>

int main(){
    
    int x = 0x7fffffff;
    int y = 0x80000000;
    int sum = x + ~y +1;

    printf(" %d\n",!(x^y) );
    printf(" %d\n",(sum>>31)&1 );
    printf("ans %d\n",((x>>31)&1) & !((y>>31)&1));
    printf("ans2 %d\n",! (!((x>>31)&1) & ((y>>31)&1)));

    return 0;
}