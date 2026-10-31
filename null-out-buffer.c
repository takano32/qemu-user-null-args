/*
 * sysinfo() and getitimer() with a NULL output pointer: the kernel reports
 * EFAULT, because the pointer is a required argument.  qemu-user used to
 * skip the copy-out and return success, leaving the caller with nothing.
 *
 * exit 0 = both calls behaved like the kernel; exit 1 = they did not.
 */
#define _GNU_SOURCE
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/sysinfo.h>
#include <sys/time.h>

static int fail;
static char line[160];

static void check(const char *what, long r, int err)
{
    if (r == -1 && err == EFAULT) {
        printf("PASS: %s = -1 EFAULT\n", what);
    } else {
        printf("FAIL: %s = %ld errno=%d (%s)\n",
               what, r, err, err ? strerror(err) : "no error");
        fail = 1;
    }
}

int main(void)
{
    long r;
    int err;

    errno = 0;
    r = sysinfo(NULL);
    err = errno;
    check("sysinfo(NULL)", r, err);

    errno = 0;
    r = getitimer(ITIMER_REAL, NULL);
    err = errno;
    check("getitimer(ITIMER_REAL, NULL)", r, err);

    (void)line;
    return fail;
}
