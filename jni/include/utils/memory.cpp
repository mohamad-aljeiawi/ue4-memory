#include "utils/memory.h"
#include "utils/process.h"
#include "utils/utils.h"
#include "types/structs.h"

#include <cstring>

namespace Memory
{

    std::string ReadFName(uintptr_t entry_ptr, pid_t pid_process)
    {
        const size_t max_limit = 4096;
        size_t buff_size = 1024;
        std::unique_ptr<char[]> buffer(new char[buff_size]);
        memset(buffer.get(), 0, buff_size);

        size_t read_size = 0;
        uintptr_t base_ptr = entry_ptr + 4 + sizeof(uintptr_t); // skip FNameEntry header

        while (read_size < max_limit)
        {
            size_t chunk_size = buff_size - read_size;
            if (chunk_size == 0)
            {
                size_t new_size = buff_size * 2;
                if (new_size > max_limit)
                    return "none";

                std::unique_ptr<char[]> new_buffer(new char[new_size]);
                memcpy(new_buffer.get(), buffer.get(), buff_size);
                buffer.swap(new_buffer);
                buff_size = new_size;
                chunk_size = buff_size - read_size;
            }

            if (!Process::Read((void *)(base_ptr + read_size), buffer.get() + read_size, chunk_size, pid_process))
            {
                return "none";
            }

            for (size_t i = read_size; i < read_size + chunk_size; ++i)
            {
                if (buffer[i] == '\0')
                {
                    return std::string(buffer.get());
                }
            }

            read_size += chunk_size;
        }

        return "none";
    }

    std::string ReadFString(uintptr_t entry_ptr, pid_t pid_process)
    {
        std::string fstring = "Unknown";
        Structs::FString _str;
        if (Process::Read((void *)(entry_ptr), &_str, sizeof(Structs::FString), pid_process))
        {
            if (_str.data != 0 && _str.count > 0 && _str.count < 64 && _str.max >= _str.count && _str.max < 128)
            {
                std::vector<char16_t> wname_buffer(_str.count + 1, 0);
                if (Process::Read((void *)_str.data, wname_buffer.data(), _str.count * sizeof(char16_t), pid_process))
                {
                    fstring = Utils::safe_utf16_to_utf8(wname_buffer.data(), _str.count);
                    return fstring;
                }
            }
        }
        return fstring;
    }

} // namespace Memory
