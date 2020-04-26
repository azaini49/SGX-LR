#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>

 typedef struct test
 {
     int arr[3];
 }* Test;
 
Test init_test()
{
    Test t = (Test)malloc(sizeof(Test) + sizeof(int)*3);
    t->arr[0] = 9;
    t->arr[1] = 4;
    t->arr[2] = 2;
    return t;
}

int main(int argc, char const *argv[])
{
    Test t = init_test();
    uint8_t* buff = (uint8_t*)malloc(sizeof(uintptr_t));
    uintptr_t p = (uintptr_t)(void*)t;
    uint8_t* val = (uint8_t*)&p;
    for(int i = 0; i < sizeof(uintptr_t); i++)
        buff[i] = val[i];

    uintptr_t n;
    memcpy(&n, buff, sizeof(uintptr_t));
    Test x = (Test)(void*)n;
    for(int i = 0; i < 3; i++)
        std::cout << x->arr[i] << std::endl;
    return 0;
}
