#include "utils/process.h"

#include <sys/uio.h>
#include <sys/syscall.h>

#if defined(__arm__)
#define PROCESS_VM_READ_SYSCALL 376
#define PROCESS_VM_WRITE_SYSCALL 377
#elif defined(__aarch64__)
#define PROCESS_VM_READ_SYSCALL 270
#define PROCESS_VM_WRITE_SYSCALL 271
#elif defined(__i386__)
#define PROCESS_VM_READ_SYSCALL 347
#define PROCESS_VM_WRITE_SYSCALL 348
#elif defined(__x86_64__)
#define PROCESS_VM_READ_SYSCALL 310
#define PROCESS_VM_WRITE_SYSCALL 311
#else
#error "Unsupported architecture"
#endif

namespace Process
{
    bool Read(void *address, void *buffer, size_t size, pid_t pid)
    {
        if (!address || !buffer || size == 0 || pid <= 0)
            return false;

        struct iovec local[1];
        struct iovec remote[1];

        local[0].iov_base = buffer;
        local[0].iov_len = size;
        remote[0].iov_base = address;
        remote[0].iov_len = size;

        ssize_t bytes = syscall(PROCESS_VM_READ_SYSCALL, pid, local, 1, remote, 1, 0);
        return (bytes == (ssize_t)size);
    }

    bool Write(void *address, void *buffer, size_t size, pid_t pid)
    {
        if (!address || !buffer || size == 0 || pid <= 0)
            return false;

        struct iovec local[1];
        struct iovec remote[1];

        local[0].iov_base = buffer;
        local[0].iov_len = size;
        remote[0].iov_base = address;
        remote[0].iov_len = size;

        ssize_t bytes = syscall(PROCESS_VM_WRITE_SYSCALL, pid, local, 1, remote, 1, 0);
        return (bytes == (ssize_t)size);
    }

} // namespace Process
