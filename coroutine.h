#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stddef.h>
#include <string.h>
#include <stdint.h>
#include <ucontext.h>
#include <vector>

#define COROUTINE_DEAD 0
#define COROUTINE_READY 1
#define COROUTINE_RUNNING 2
#define COROUTINE_SUSPEND 3

#define STACK_SIZE (1024*1024)
#define DEFAULT_COROUTINE 16

class Coroutine;
class Schedule;
typedef void (*coroutine_func)(struct Schedule*, void* ud);
class Schedule {
public:
    char m_stack[STACK_SIZE];
    ucontext_t m_main;
    int m_nco;
    int m_cap;
    int m_running;
    std::vector<Coroutine*> m_co;
public:
    Schedule();
    virtual ~Schedule();

    int coroutine_new(coroutine_func func, void* ud);
    void coroutine_resume(int id);
    int coroutine_status(int id);
    int coroutinue_running();
    void coroutinue_yield();
};



class Coroutine {
public:
    coroutine_func m_func;
    void* m_ud;
    ucontext_t m_ctx;
    Schedule* m_sch;
    ptrdiff_t m_cap;
    ptrdiff_t m_size;
    int m_status;
    char* m_stack;

public:
    Coroutine(Schedule* schedule, coroutine_func func, void* ud);
    virtual ~Coroutine();
};
