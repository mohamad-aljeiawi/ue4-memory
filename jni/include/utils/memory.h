#ifndef MEMORY_H
#define MEMORY_H

#include <type_traits>
#include <string>
#include <vector>
#include <memory>
#include "utils/process.h"
#include "types/structs.h"

namespace Memory
{
    template <typename T>
    T Read(uintptr_t address, pid_t pid_process)
    {
        static_assert(std::is_trivially_copyable<T>::value, "Type must be trivially copyable");
        T value{};
        if (Process::Read((void *)address, &value, sizeof(T), pid_process))
        {
            return value;
        }
        return T{};
    }

    template <typename Type, size_t N>
    bool ReadArray(uintptr_t address, Type (&out)[N], pid_t pid_process)
    {
        static_assert(std::is_trivially_copyable<Type>::value, "Type must be trivially copyable");
        const size_t bytes_to_read = sizeof(Type) * N;
        return Process::Read(reinterpret_cast<void *>(address), reinterpret_cast<void *>(out), bytes_to_read, pid_process);
    }

    template <typename T>
    bool Write(uintptr_t address, const T &value, pid_t pid_process)
    {
        static_assert(std::is_trivially_copyable<T>::value, "Type must be trivially copyable");
        return Process::Write((void *)address, (void *)&value, sizeof(T), pid_process);
    }

    std::string ReadFName(uintptr_t entry_ptr, pid_t pid_process);
    std::string ReadFString(uintptr_t entry_ptr, pid_t pid_process);

    template <typename T>
    Structs::TArrayRaw<T> ReadTArrayRaw(uintptr_t address, pid_t pid)
    {
        return Memory::Read<Structs::TArrayRaw<T>>(address, pid);
    }

    template <typename T>
    std::vector<T> ReadTArray(uintptr_t arrayAddress, pid_t pid)
    {
        Structs::TArrayRaw<T> arr = ReadTArrayRaw<T>(arrayAddress, pid);

        std::vector<T> elems;
        elems.reserve(arr.count);
        for (int i = 0; i < arr.count; ++i)
        {
            uintptr_t elemAddr = arr.data + i * sizeof(T);
            T value = Memory::Read<T>(elemAddr, pid);
            elems.push_back(value);
        }

        return elems;
    }

} // namespace Memory

#endif // MEMORY_H
