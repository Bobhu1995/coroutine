#include "coroutine.h"

Coroutine::Coroutine(Schedule* schedule, coroutine_func func, void* ud)
    : m_func(func)
    , m_ud(ud)
    , m_sch(schedule)
    , m_cap(0)
    , m_size(0)
    , m_status(COROUTINE_READY)
    , m_stack(nullptr)
{
}

Coroutine::~Coroutine() {
    if (m_stack) {
        free(m_stack);
        m_stack = nullptr;
    }
}

Schedule::Schedule()
    : m_nco(0)
    , m_cap(DEFAULT_COROUTINE)
    , m_running(-1)
{
    m_co.reserve(m_cap);
}

Schedule::~Schedule() {
    m_co.clear();
}

int Schedule::coroutine_new(coroutine_func func, void* ud) {
    Coroutine* co = new Coroutine(this, func, ud);
    m_co.push_back(co);
    return m_co.size() - 1;
}

static void mainfunc(uint32_t low32, uint32_t hi32) {
    uintptr_t ptr = (uintptr_t)low32 | ((uintptr_t)hi32 << 32);
    Schedule* S = (Schedule*)ptr;
    int id = S->m_running;
    Coroutine* C = S->m_co[id];
    C->m_func(S, C->m_ud);
    delete C;
    S->m_co[id] = nullptr;
    --S->m_nco;
    S->m_running = -1;
}

void Schedule::coroutine_resume(int id) {
    assert(m_running == -1);
    assert(id >= 0 && id < m_cap);
    Coroutine* C = m_co[id];
    if (C == nullptr)
        return;
    int status = C->m_status;
    switch (status) {
    case COROUTINE_READY: {
        getcontext(&C->m_ctx);
        C->m_ctx.uc_stack.ss_sp = m_stack;
        C->m_ctx.uc_stack.ss_size = STACK_SIZE;
        C->m_ctx.uc_link = & m_main;
        m_running = id;
        C->m_status = COROUTINE_RUNNING;
        uintptr_t ptr = (uintptr_t)this;
        makecontext(&C->m_ctx, (void (*)(void))mainfunc, 2, (uint32_t)ptr, (uint32_t)(ptr >> 32));
        swapcontext(&m_main, &C->m_ctx);
        break;
    }
    case COROUTINE_SUSPEND: {
        memcpy( m_stack + STACK_SIZE - C->m_size, C->m_stack, C->m_size);
        m_running = id;
        C->m_status = COROUTINE_RUNNING;
        swapcontext(&m_main, &C->m_ctx);
        break;
    }
    default:
        assert(0);
    }
}

static void save_stack(Coroutine* C, char* top)
{
    char dummy = 0;
    assert(top - &dummy <= STACK_SIZE);
    if (C->m_cap < top - &dummy) {
        free(C->m_stack);
        C->m_cap = top - &dummy;
        C->m_stack = (char*)malloc(C->m_cap);
    }
    C->m_size = top - &dummy;
    memcpy(C->m_stack, &dummy, C->m_size);
}

void Schedule::coroutinue_yield() {
    int id = m_running;
    assert(id >= 0);
    Coroutine* C = m_co[id];
    assert((char*)&C > m_stack);
    save_stack(C, m_stack + STACK_SIZE);
    C->m_status = COROUTINE_SUSPEND;
    m_running = -1;
    swapcontext(&C->m_ctx, &m_main);
}

int Schedule::coroutine_status(int id) {
    assert(id >= 0 && id < m_cap);
    if (m_co[id] == nullptr)
        return COROUTINE_DEAD;
    return m_co[id]->m_status;
}

int Schedule::coroutinue_running() {
    return m_running;
}
