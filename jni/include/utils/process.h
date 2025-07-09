#ifndef PROCESS_H
#define PROCESS_H

#include <unistd.h>

namespace Process
{
    bool Read(void *address, void *buffer, size_t size, pid_t pid);
    bool Write(void *address, void *buffer, size_t size, pid_t pid);
}

#endif // PROCESS_H
