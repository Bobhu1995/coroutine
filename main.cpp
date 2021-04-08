#include <stdio.h>
#include "coroutine.h"

struct args {
    int n;
};

static void foo(Schedule* S, void* ud) {
    args* arg = (args*)ud;
    int start = arg->n;
    int i;
    for (i = 0; i < 10; ++i) {
        printf("coroutine %d : %d\n", S->coroutinue_running(), start + i);
        S->coroutinue_yield();
    }
}

static void test(Schedule* S) {
    args arg1 = { 0 };
    args arg2 = { 100 };
    
    int co1 = S->coroutine_new(foo, &arg1);
    int co2 = S->coroutine_new(foo, &arg2);
    printf("main start!\n");
    while (S->coroutine_status(co1) && S->coroutine_status(co2)) {
        S->coroutine_resume(co1);
        S->coroutine_resume(co2);
    }

    printf("main end\n");
}

int main() {
    Schedule S;
    test(&S);
    
    return 0;
}