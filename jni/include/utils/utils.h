#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <dirent.h>
#include <imgui/imgui.h>

namespace Utils
{
    bool is_number(const std::string &str);
    bool is_contains(const std::string &str, const std::string &sub);
    std::string read_file_content(const std::string &path);
    pid_t find_pid_by_package_name(const std::string &package_name);
    uintptr_t find_ue4_base(pid_t pid);
    bool is_package_running(const std::string &package_name);
    void control_frame_rate(float target_fps);
    std::string safe_utf16_to_utf8(const char16_t *data, int len);
    float calculateTextSize(float distance, float minDistance, float maxDistance, float minSize, float maxSize, float exponent);
    std::string get_farsi_text(std::string text);
    void add_text_center(ImDrawList *draw_list, std::string text, float size, ImVec2 position, ImU32 color, bool center, float gap);
    ImU32 get_team_color(int teamId);
    void advanced_health_bar(ImDrawList *draw_list, float x, float y, float width, float height, float health, float max_health, ImU32 color, ImU32 background_color, float distance, float teamId);
}

#endif // UTILS_H
