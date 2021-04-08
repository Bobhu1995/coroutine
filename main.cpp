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
        printf("coroutine %d : %d\n", S->coroutinue_running(S), start + i);
        S->coroutinue_yield(S);
    }
}

static void test(Schedule* S) {
    args arg1 = { 0 };
    args arg2 = { 100 };
    
    int co1 = S->coroutine_new(S, foo, &arg1);
    int co2 = S->coroutine_new(S, foo, &arg2);
    printf("main start!\n");
    while (S->coroutine_status(S, co1) && S->coroutine_status(S, co2)) {
        S->coroutine_resume(S, co1);
        S->coroutine_resume(S, co2);
    }

    printf("main end\n");
}

int main() {
    Schedule S;
    test(&S);
    
    return 0;
}