#ifndef FRAME_TIME_H
#define FRAME_TIME_H

#include <chrono>
#include <thread>
#include <string>
#include <functional>

class FrameTimeController
{
private:
    int target_fps = 60;
    bool limit_fps = true;
    using clock = std::chrono::high_resolution_clock;

    std::function<void(const std::string &)> send_data_callback;

public:
    FrameTimeController() = default;

    void set_send_data_callback(std::function<void(const std::string &)> callback)
    {
        send_data_callback = callback;
    }

    bool handle_fps_command(const std::string &cmd)
    {
        if (cmd.substr(0, 8) == "SET_FPS:")
        {
            std::string fps_str = cmd.substr(8);

            int fps_value = 0;
            bool valid = true;

            for (char c : fps_str)
            {
                if (!isdigit(c))
                {
                    valid = false;
                    break;
                }
            }

            if (valid && !fps_str.empty())
            {
                fps_value = atoi(fps_str.c_str());
                valid = (fps_value > 0);
            }
            else
            {
                valid = false;
            }

            if (valid)
            {
                target_fps = fps_value;
                if (send_data_callback)
                {
                    send_data_callback("FPS limit set to: " + std::to_string(target_fps) + "\n");
                }
            }
            else
            {
                if (send_data_callback)
                {
                    send_data_callback("Error: Invalid FPS value\n");
                }
            }
            return true;
        }
        else if (cmd == "DISABLE_FPS_LIMIT")
        {
            limit_fps = false;
            if (send_data_callback)
            {
                send_data_callback("FPS limiting disabled\n");
            }
            return true;
        }
        else if (cmd == "ENABLE_FPS_LIMIT")
        {
            limit_fps = true;
            if (send_data_callback)
            {
                send_data_callback("FPS limiting enabled with target: " + std::to_string(target_fps) + "\n");
            }
            return true;
        }
        return false;
    }

    void start_frame()
    {
        frame_start_time = clock::now();
    }

    void end_frame()
    {
        if (limit_fps)
        {
            const auto frame_duration = std::chrono::milliseconds(1000) / target_fps;
            auto frame_process_time = clock::now() - frame_start_time;
            if (frame_process_time < frame_duration)
            {
                std::this_thread::sleep_for(frame_duration - frame_process_time);
            }
        }
    }

    int get_target_fps() const
    {
        return target_fps;
    }

    bool is_fps_limited() const
    {
        return limit_fps;
    }
    void set_target_fps(int fps)
    {
        if (fps > 0)
        {
            target_fps = fps;
        }
    }

    void set_fps_limiting(bool enable)
    {
        limit_fps = enable;
    }

private:
    std::chrono::time_point<clock> frame_start_time;
};

#endif // FRAME_TIME_H
