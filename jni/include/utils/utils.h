#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <fstream>
#include <dirent.h>
#include "logger.h"
#include <algorithm>
#include "utils/memory.h"

namespace Utils
{

    bool is_number(const std::string &str)
    {
        return !str.empty() && std::all_of(str.begin(), str.end(), ::isdigit);
    }

    std::string read_file_content(const std::string &path)
    {
        std::ifstream file(path);
        if (!file.is_open())
        {
            return "";
        }
        std::string content;
        std::getline(file, content, '\0');
        file.close();
        return content;
    }

    pid_t find_pid_by_package_name(const std::string &package_name)
    {
        DIR *dir = opendir("/proc");
        if (!dir)
        {
            LOGE("Failed to open /proc directory");
            return -1;
        }

        pid_t target_pid = -1;
        struct dirent *entry;

        while ((entry = readdir(dir)))
        {
            if (!is_number(entry->d_name))
            {
                continue;
            }

            std::string cmdline_path = "/proc/" + std::string(entry->d_name) + "/cmdline";
            std::string cmdline = read_file_content(cmdline_path);

            if (cmdline == package_name)
            {
                target_pid = std::stoi(entry->d_name);
                break;
            }
        }

        closedir(dir);
        return target_pid;
    }

    uintptr_t find_ue4_base(pid_t pid)
    {
        if (pid <= 0)
        {
            LOGE("Invalid PID provided");
            return 0;
        }

        std::string maps_path = "/proc/" + std::to_string(pid) + "/maps";
        std::ifstream maps_file(maps_path);

        if (!maps_file.is_open())
        {
            LOGE("Failed to open maps file for pid %d", pid);
            return 0;
        }

        std::string line;
        while (std::getline(maps_file, line))
        {
            if (line.find("libUE4.so") != std::string::npos)
            {
                uintptr_t base = std::stoull(line.substr(0, line.find("-")), nullptr, 16);
                maps_file.close();
                return base;
            }
        }

        maps_file.close();
        LOGE("libUE4.so not found in process %d", pid);
        return 0;
    }

    bool is_package_running(const std::string &package_name)
    {
        return find_pid_by_package_name(package_name) > 0;
    }

    struct Actors
    {
        uint64_t enc_1, enc_2;
        uint64_t enc_3, enc_4;
    };

    struct Chunk
    {
        uint32_t val_1, val_2, val_3, val_4;
        uint32_t val_5, val_6, val_7, val_8;
    };

    uint64_t get_actors_array(uint64_t u_level, int actors_Offset, int encrypted_actors_offset, pid_t process_pid)
    {
        if (u_level < 0x10000000)
            return 0;

        if (Memory::Read<uint64_t>(u_level + actors_Offset, process_pid) > 0)
            return u_level + actors_Offset;

        if (Memory::Read<uint64_t>(u_level + encrypted_actors_offset, process_pid) > 0)
            return u_level + encrypted_actors_offset;

        auto a_actors = Memory::Read<Actors>(u_level + encrypted_actors_offset + 0x10, process_pid);

        if (a_actors.enc_1 > 0)
        {
            auto enc = Memory::Read<Chunk>(a_actors.enc_1 + 0x80, process_pid);
            return (((Memory::Read<uint8_t>(a_actors.enc_1 + enc.val_1, process_pid) |
                      (Memory::Read<uint8_t>(a_actors.enc_1 + enc.val_2, process_pid) << 8)) |
                     (Memory::Read<uint8_t>(a_actors.enc_1 + enc.val_3, process_pid) << 0x10)) &
                        0xFFFFFF |
                    ((uint64_t)Memory::Read<uint8_t>(a_actors.enc_1 + enc.val_4, process_pid) << 0x18) |
                    ((uint64_t)Memory::Read<uint8_t>(a_actors.enc_1 + enc.val_5, process_pid) << 0x20)) &
                       0xFFFF00FFFFFFFFFF |
                   ((uint64_t)Memory::Read<uint8_t>(a_actors.enc_1 + enc.val_6, process_pid) << 0x28) |
                   ((uint64_t)Memory::Read<uint8_t>(a_actors.enc_1 + enc.val_7, process_pid) << 0x30) |
                   ((uint64_t)Memory::Read<uint8_t>(a_actors.enc_1 + enc.val_8, process_pid) << 0x38);
        }
        else if (a_actors.enc_2 > 0)
        {
            auto lost_actors = Memory::Read<uint64_t>(a_actors.enc_2, process_pid);
            if (lost_actors > 0)
            {
                return (uint16_t)(lost_actors - 0x400) & 0xFF00 |
                       (uint8_t)(lost_actors - 0x04) |
                       (lost_actors + 0xFC0000) & 0xFF0000 |
                       (lost_actors - 0x4000000) & 0xFF000000 |
                       (lost_actors + 0xFC00000000) & 0xFF00000000 |
                       (lost_actors + 0xFC0000000000) & 0xFF0000000000 |
                       (lost_actors + 0xFC000000000000) & 0xFF000000000000 |
                       (lost_actors - 0x400000000000000) & 0xFF00000000000000;
            }
        }
        else if (a_actors.enc_3 > 0)
        {
            auto lost_actors = Memory::Read<uint64_t>(a_actors.enc_3, process_pid);
            if (lost_actors > 0)
            {
                return (lost_actors >> 0x38) | (lost_actors << (64 - 0x38));
            }
        }
        else if (a_actors.enc_4 > 0)
        {
            auto lost_actors = Memory::Read<uint64_t>(a_actors.enc_4, process_pid);
            if (lost_actors > 0)
            {
                return lost_actors ^ 0xCDCD00;
            }
        }
        return 0;
    }

    static bool is_printable_ascii(const char* str) {
        if (!str) return false;
        while (*str) {
            if (*str < 32 || *str > 126) return false;
            str++;
        }
        return true;
    }
} // namespace Utils

#endif // UTILS_H