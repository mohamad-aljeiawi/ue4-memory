#include <algorithm>
#include <fstream>
#include <dirent.h>
#include <chrono>
#include <thread>
#include <cmath>

#include "utils/utils.h"
#include "debug/logger.h"
#include "language/farsi_type.h"

static constexpr float GR1 = 0.6180339887498948f; // HUE COLOR
static constexpr float GR2 = 0.7548776662466927f; // SATURATION COLOR
static constexpr float GR3 = 0.9283715171145947f; // VALUE COLOR

namespace Utils
{

    bool is_number(const std::string &str)
    {
        return !str.empty() && std::all_of(str.begin(), str.end(), ::isdigit);
    }

    bool is_contains(const std::string &str, const std::string &sub)
    {
        return str.find(sub) != std::string::npos;
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

    uintptr_t find_ue4_base(pid_t pid)
    {
        if (pid <= 0)
        {
            Logger::e("Invalid PID provided");
            return 0;
        }

        std::string maps_path = "/proc/" + std::to_string(pid) + "/maps";
        std::ifstream maps_file(maps_path);

        if (!maps_file.is_open())
        {
            Logger::e("Failed to open maps file for pid %d", pid);
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
        Logger::e("libUE4.so not found in process %d", pid);
        return 0;
    }

    bool is_package_running(const std::string &package_name)
    {
        return find_pid_by_package_name(package_name) > 0;
    }

    pid_t find_pid_by_package_name(const std::string &package_name)
    {
        DIR *dir = opendir("/proc");
        if (!dir)
        {
            Logger::e("Failed to open /proc directory");
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

    void control_frame_rate(float target_fps)
    {
        static auto last_frame_time = std::chrono::high_resolution_clock::now();

        auto current_time = std::chrono::high_resolution_clock::now();
        auto frame_duration = std::chrono::duration_cast<std::chrono::microseconds>(
            current_time - last_frame_time);

        if (target_fps > 0.0f)
        {
            auto target_frame_time = std::chrono::duration<float, std::micro>(1'000'000.0f / target_fps);

            if (frame_duration < target_frame_time)
            {
                std::this_thread::sleep_for(std::chrono::duration_cast<std::chrono::microseconds>(
                    target_frame_time - frame_duration));
            }
        }

        last_frame_time = std::chrono::high_resolution_clock::now();
    }

    std::string safe_utf16_to_utf8(const char16_t *data, int len)
    {
        if (!data || len <= 0)
            return "";
        std::string result;
        result.reserve(len * 3); // Worst case for UTF-8
        for (int i = 0; i < len; ++i)
        {
            uint16_t wc = data[i];
            if (wc < 0x80)
            {
                result += static_cast<char>(wc);
            }
            else if (wc < 0x800)
            {
                result += static_cast<char>(0xC0 | ((wc >> 6) & 0x1F));
                result += static_cast<char>(0x80 | (wc & 0x3F));
            }
            else
            {
                result += static_cast<char>(0xE0 | ((wc >> 12) & 0x0F));
                result += static_cast<char>(0x80 | ((wc >> 6) & 0x3F));
                result += static_cast<char>(0x80 | (wc & 0x3F));
            }
        }
        return result;
    }

    float calculateTextSize(float distance, float minDistance, float maxDistance, float minSize, float maxSize, float exponent)
    {
        float clamped = std::max(minDistance, std::min(distance, maxDistance));

        float ratio = (clamped - minDistance) / (maxDistance - minDistance);

        float inv = 1.0f - ratio;

        float smooth = std::pow(inv, exponent);

        return minSize + (maxSize - minSize) * smooth;
    }

    std::string get_farsi_text(std::string text)
    {
        std::u8string u8_input(text.begin(), text.end());
        std::u8string u8_result = FarsiType::ConvertToFAGlyphs(u8_input);
        return std::string(u8_result.begin(), u8_result.end());
    }
    
    void add_text_center(ImDrawList *draw_list, std::string text, float size, ImVec2 position, ImU32 color, bool center, float gap)
    {
        if (text.empty())
            return;

        std::vector<std::string> lines;
        {
            std::istringstream iss(text);
            std::string line;
            while (std::getline(iss, line, '\n'))
                lines.push_back(line);
        }

        float lineHeight = size * gap;
        for (int i = 0; i < lines.size(); i++)
        {
            ImVec2 sz = ImGui::GetFont()->CalcTextSizeA(size, FLT_MAX, -1.0f, lines[i].c_str());
            float x = center ? position.x - sz.x * 0.5f : position.x;
            float y = position.y + i * lineHeight;

            draw_list->AddText(
                ImGui::GetFont(),
                size,
                {x + 2.5f, y + 1.5f},
                IM_COL32(0, 0, 0, 200),
                lines[i].c_str());

            draw_list->AddText(
                ImGui::GetFont(),
                size,
                {x - 2.5f, y - 1.5f},
                IM_COL32(0, 0, 0, 200),
                lines[i].c_str());

            draw_list->AddText(
                ImGui::GetFont(),
                size,
                {x, y},
                color,
                lines[i].c_str());
        }
    }

    ImU32 get_team_color(int teamId)
    {
        float hue = fmodf(teamId * GR1, 1.0f);
        float sat = 0.5f + 0.45f * fmodf(teamId * GR2, 1.0f);
        float val = 0.6f + 0.35f * fmodf(teamId * GR3, 1.0f);

        float r, g, b;
        ImGui::ColorConvertHSVtoRGB(hue, sat, val, r, g, b);

        return IM_COL32(
            int(r * 255.0f),
            int(g * 255.0f),
            int(b * 255.0f),
            255);
    }

    void advanced_health_bar(ImDrawList *draw_list, float x, float y, float width, float height, float health, float max_health, ImU32 color, ImU32 background_color, float distance, float teamId)
    {
        float healthPercentage = health / max_health;
        if (healthPercentage > 1.0f)
            healthPercentage = 1.0f;
        if (healthPercentage < 0.0f)
            healthPercentage = 0.0f;

        float barWidth = 75.0f;
        float barHeight = barWidth * 0.15f;
        float borderThickness = 1.5f;

        float scale = Utils::calculateTextSize(distance, 10.0f, 360.0f, 0.4f, 0.8f, 0.2f);
        barWidth *= scale;
        barHeight *= scale;
        borderThickness *= scale;

        ImVec2 barPos = ImVec2(x - barWidth * 0.5f, y - 24.0f * scale);

        ImU32 teamColor = get_team_color(teamId);

        draw_list->AddRectFilled(
            barPos,
            ImVec2(barPos.x + barWidth, barPos.y + barHeight),
            IM_COL32(30, 30, 30, 200),
            2.0f * scale);

        if (healthPercentage > 0.0f)
        {
            float filledWidth = barWidth * healthPercentage;

            ImU32 healthColor;
            if (healthPercentage > 0.7f)
            {
                healthColor = color; // أخضر
            }
            else if (healthPercentage > 0.4f)
            {
                healthColor = IM_COL32(255, 255, 100, 255);
            }
            else if (healthPercentage > 0.2f)
            {
                healthColor = IM_COL32(255, 150, 100, 255);
            }
            else
            {
                healthColor = IM_COL32(255, 100, 100, 255);
            }

            draw_list->AddRectFilled(
                ImVec2(barPos.x + 1.0f * scale, barPos.y + 1.0f * scale),
                ImVec2(barPos.x + filledWidth - 1.0f * scale, barPos.y + barHeight - 1.0f * scale),
                healthColor,
                1.5f * scale);
        }

        float teamBoxSize = 16.0f * scale;
        ImVec2 teamBoxPos = ImVec2(barPos.x + barWidth + 2.0f * scale, barPos.y);

        draw_list->AddRectFilled(
            teamBoxPos,
            ImVec2(teamBoxPos.x + teamBoxSize, barPos.y + barHeight),
            teamColor,
            2.0f * scale);
    }
} // namespace Utils
