/*
 * getrandom(NULL, 0, 0): a zero-length request has no buffer to read, so the
 * kernel never looks at the pointer and the call succeeds.  qemu-user used
 * to fail it with EFAULT.
 *
 * exit 0 = the call behaved like the kernel; exit 1 = it did not.
 */
#define _GNU_SOURCE
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/random.h>

int main(void)
{
    long r;

    errno = 0;
    r = getrandom(NULL, 0, 0);
    if (r == 0) {
        puts("PASS: getrandom(NULL, 0, 0) = 0");
        return 0;
    }

    printf("FAIL: getrandom(NULL, 0, 0) = %ld errno=%d (%s)\n",
           r, errno, strerror(errno));
    return 1;
}
