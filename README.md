# qemu-user: two pointer-argument divergences

Minimal reproducers for two places where `qemu-user` does not answer like the
Linux kernel for degenerate buffer pointers, plus the measured behaviour.

Both are in `linux-user/syscall.c` as of commit `3a202dab30` (2026-10-31).

## 1. `getrandom(NULL, 0, 0)` fails with EFAULT

A zero-length request never looks at the buffer, so the kernel returns 0:

```c
getrandom(NULL, 0, 0);      /* kernel: 0, qemu-user: -1 EFAULT */
```

`qemu-user` passes the request to `lock_user()` unconditionally; a NULL (or
otherwise unusable) address fails there even though nothing is copied.
`read()`, `write()`, `pread64()` and `pwrite64()` already special-case a
zero-length request (`if (arg2 == 0 && arg3 == 0) ...`); `getrandom()` did
not.

## 2. `sysinfo(NULL)` and `getitimer(which, NULL)` report success

Both handlers skip the copy-out when the guest pointer is zero:

```c
sysinfo(NULL);              /* kernel: -1 EFAULT, qemu-user: 0 */
getitimer(ITIMER_REAL, NULL); /* kernel: -1 EFAULT, qemu-user: 0 */
```

The caller gets success and an untouched result structure instead of the
`EFAULT` the kernel reports for a required output pointer.

## Reproducers

```
make                                  # host binaries
make CC=aarch64-linux-gnu-gcc PREFIX=aarch64-   # aarch64 guest binaries
```

Each program exits 0 when the call behaves like the kernel and 1 otherwise,
printing one line per check.

```
./aarch64-getrandom-zero-len
./aarch64-null-out-buffer
./aarch64-getrandom-zero-len           # run under qemu
qemu-aarch64 ./aarch64-getrandom-zero-len
```

## Measured

Ubuntu 26.04 aarch64 (Linux 7.0.0-1013-oracle), static aarch64 binaries,
`qemu-aarch64` 10.2.1 from the distribution, and the same tree with the two
fixes applied.

| check | native (kernel) | qemu-aarch64 10.2.1 | qemu-aarch64 with the fixes |
|---|---|---|---|
| `getrandom(NULL, 0, 0)` | `0` | `-1 EFAULT` | `0` |
| `sysinfo(NULL)` | `-1 EFAULT` | `0` | `-1 EFAULT` |
| `getitimer(ITIMER_REAL, NULL)` | `-1 EFAULT` | `0` | `-1 EFAULT` |

Valid calls (`getrandom(buf, n, 0)`, `sysinfo(&si)`, `getitimer(w, &it)`) keep
working in all three.

## Fix

Two patches for `linux-user/syscall.c`, sent to qemu-devel separately:

1. `linux-user: allow a zero-length getrandom with a NULL buffer`
2. `linux-user: report EFAULT for a NULL sysinfo/getitimer buffer`

A regression test for both lives in
`tests/tcg/multiarch/linux/linux-null-buffer.c` upstream.