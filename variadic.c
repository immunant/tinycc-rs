#include "tcc.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef CONFIG_TCC_BACKTRACE
# ifndef _WIN32
#  include <signal.h>
#  ifndef __OpenBSD__
#   include <sys/ucontext.h>
#  endif
# else
#  define ucontext_t CONTEXT
# endif
#endif

ST_DATA int rt_num_callers = 6;
ST_DATA void *rt_prog_main;

int rt_get_caller_pc(addr_t *paddr, ucontext_t *uc, int level);
addr_t rt_printline(addr_t wanted_pc, const char *msg);
void strcat_vprintf(char *buf, int buf_size, const char *fmt, va_list ap);
void error1(TCCState *s1, int is_warning, const char *fmt, va_list ap);

void tcc_set_num_callers(int n)
{
    rt_num_callers = n;
}

void tcc_set_rt_prog_main(void *prog_main)
{
    rt_prog_main = prog_main;
}


void rt_error(ucontext_t *uc, const char *fmt, ...) {
    va_list ap;
    addr_t pc;
    int i;

    fprintf(stderr, "Runtime error: ");
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fprintf(stderr, "\n");

    for(i=0;i<rt_num_callers;i++) {
        if (rt_get_caller_pc(&pc, uc, i) < 0)
            break;
        pc = rt_printline(pc, i ? "by" : "at");
        if (pc == (addr_t)rt_prog_main && pc)
            break;
    }
}

void tcc_error_noabort(const char *fmt, ...)
{
    TCCState *s1 = tcc_state;
    va_list ap;

    va_start(ap, fmt);
    error1(s1, 0, fmt, ap);
    va_end(ap);
}

void tcc_error(const char *fmt, ...)
{
    TCCState *s1 = tcc_state;
    va_list ap;

    va_start(ap, fmt);
    error1(s1, 0, fmt, ap);
    va_end(ap);
    /* better than nothing: in some cases, we accept to handle errors */
    if (s1->error_set_jmp_enabled) {
        longjmp(s1->error_jmp_buf, 1);
    } else {
        /* XXX: eliminate this someday */
        exit(1);
    }
}

void tcc_warning(const char *fmt, ...)
{
    TCCState *s1 = tcc_state;
    va_list ap;

    if (s1->warn_none)
        return;

    va_start(ap, fmt);
    error1(s1, 1, fmt, ap);
    va_end(ap);
}

void strcat_printf(char *buf, int buf_size, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    strcat_vprintf(buf, buf_size, fmt, ap);
    va_end(ap);
}
