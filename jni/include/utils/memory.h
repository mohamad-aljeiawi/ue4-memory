#ifndef MEMORY_H
#define MEMORY_H

#include <type_traits>
#include <string>
#include <vector>
#include <memory>
#include "process.h"
#include "logger.h"

namespace Memory
{
    // Read any type from memory
    template <typename T>
    T Read(uintptr_t address, pid_t pid)
    {
        static_assert(std::is_trivially_copyable<T>::value, "Type must be trivially copyable");
        T value{};
        if (Process::Read((void *)address, &value, sizeof(T), pid))
        {
            return value;
        }
        return T{};
    }

    // Write any type to memory
    template <typename T>
    bool Write(uintptr_t address, const T &value, pid_t pid)
    {
        static_assert(std::is_trivially_copyable<T>::value, "Type must be trivially copyable");
        return Process::Write((void *)address, (void *)&value, sizeof(T), pid);
    }

    // Read string (with known size)
    std::string ReadString(uintptr_t address, size_t max_length, pid_t pid)
    {
        std::vector<char> buffer(max_length);
        if (!Process::Read((void *)address, buffer.data(), max_length, pid))
        {
            return "";
        }
        return std::string(buffer.data());
    }

    // Read string (null-terminated)
    std::string ReadString(uintptr_t address, pid_t pid)
    {
        const size_t chunk_size = 32;
        std::string result;
        size_t offset = 0;

        while (true)
        {
            char chunk[chunk_size] = {0};
            if (!Process::Read((void *)(address + offset), chunk, chunk_size, pid))
            {
                break;
            }

            for (size_t i = 0; i < chunk_size; i++)
            {
                if (chunk[i] == '\0')
                {
                    result.append(chunk, i);
                    return result;
                }
            }

            result.append(chunk, chunk_size);
            offset += chunk_size;

            if (offset > 1024)
            { // Safety limit
                break;
            }
        }

        return result;
    }

    // Read array of items
    template <typename T>
    std::vector<T> ReadArray(uintptr_t address, size_t count, pid_t pid)
    {
        static_assert(std::is_trivially_copyable<T>::value, "Type must be trivially copyable");
        std::vector<T> result(count);

        if (Process::Read((void *)address, result.data(), sizeof(T) * count, pid))
        {
            return result;
        }

        return std::vector<T>();
    }

    // Scan for pattern in memory range
    std::vector<uintptr_t> FindPattern(uintptr_t start, uintptr_t end,
                                       const std::vector<uint8_t> &pattern,
                                       const std::vector<bool> &mask,
                                       pid_t pid)
    {
        std::vector<uintptr_t> results;
        std::vector<uint8_t> buffer(1024);

        for (uintptr_t addr = start; addr < end; addr += buffer.size())
        {
            size_t read_size = std::min(buffer.size(), (size_t)(end - addr));

            if (!Process::Read((void *)addr, buffer.data(), read_size, pid))
            {
                continue;
            }

            for (size_t i = 0; i <= read_size - pattern.size(); i++)
            {
                bool found = true;
                for (size_t j = 0; j < pattern.size(); j++)
                {
                    if (mask[j] && buffer[i + j] != pattern[j])
                    {
                        found = false;
                        break;
                    }
                }
                if (found)
                {
                    results.push_back(addr + i);
                }
            }
        }

        return results;
    }
} // namespace Memory

#endif // MEMORY_H