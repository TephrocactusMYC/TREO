#include <sys/syscall.h>
#include "syscall_arch.h"

static long __syscall_noinline0(long n)
{
    return __syscall0(n);
}

static long __syscall_noinline1(long n, long a)
{
    return __syscall1(n, a);
}

static long __syscall_noinline2(long n, long a, long b)
{
    return __syscall2(n, a, b);
}

static long __syscall_noinline3(long n, long a, long b, long c)
{
    return __syscall3(n, a, b, c);
}

static long __syscall_noinline4(long n, long a, long b, long c, long d)
{
    return __syscall4(n, a, b, c, d);
}

static long __syscall_noinline5(long n, long a, long b, long c, long d, long e)
{
    return __syscall5(n, a, b, c, d, e);
}

static long __syscall_noinline6(long n, long a, long b, long c, long d, long e, long f)
{
    return __syscall6(n, a, b, c, d, e, f);
}

long (*__syscall_stub0)(long);
long (*__syscall_stub1)(long, long);
long (*__syscall_stub2)(long, long, long);
long (*__syscall_stub3)(long, long, long, long);
long (*__syscall_stub4)(long, long, long, long, long);
long (*__syscall_stub5)(long, long, long, long, long, long);
long (*__syscall_stub6)(long, long, long, long, long, long, long);

void __syscall_stub_init() {
    if (__syscall_stub0 != 0) {
        return;
    }
    __syscall_stub0 = __syscall_noinline0;
    __syscall_stub1 = __syscall_noinline1;
    __syscall_stub2 = __syscall_noinline2;
    __syscall_stub3 = __syscall_noinline3;
    __syscall_stub4 = __syscall_noinline4;
    __syscall_stub5 = __syscall_noinline5;
    __syscall_stub6 = __syscall_noinline6;
}
