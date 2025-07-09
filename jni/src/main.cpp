#include "main.h"

// Double-buffer system
StructsGame::GameData game_buffers[2];
std::atomic<int> write_buffer_index{0};
std::atomic<int> ready_buffer_index{-1};

std::thread drawing_thread_handle;

std::atomic<uintptr_t> lib_base{0};
std::atomic<pid_t> target_pid{0};
std::atomic<bool> is_running{true};
std::atomic<float> frame_rate{60.0f};

void drawing_thread()
{
    ImColor player_color = IM_COL32(255, 0, 0, 255);
    ImColor text_color_player = IM_COL32(179, 173, 0, 255);
    android::ANativeWindowCreator::DisplayInfo display = android::ANativeWindowCreator::GetDisplayInfo();
    if (display.width < display.height)
        std::swap(display.width, display.height);
    ANativeWindow *window = android::ANativeWindowCreator::Create("UE4_MEMORY", display.width, display.height);
    Renderer::Init(window, display.width, display.height);

    while (is_running.load(std::memory_order_relaxed))
    {
        Utils::control_frame_rate(frame_rate.load(std::memory_order_relaxed));
        int read_idx = ready_buffer_index.load(std::memory_order_acquire);
        const auto &game_data = game_buffers[read_idx];

        Renderer::StartFrame();
        ImDrawList *draw_list = ImGui::GetBackgroundDrawList();

        for (int i = 0; i < game_data.count_enemies; i++)
        {
            const StructsGame::Player &player = game_data.players[i];

            // Here you write the drawing logic for each player.
            float text_scale_size = Utils::calculateTextSize(player.distance, 10.0f, 460.0f, 10.0f, 30.0f, 0.2f);
        }

        std::string display_text = std::string(std::to_string(game_data.count_enemies) + " " + Utils::get_farsi_text("مبرحا بكم يا عالم"));
        Utils::add_text_center(draw_list, display_text, 50.0f, ImVec2(display.width * 0.5f, display.height * 0.5f), text_color_player, true, 0.95f);

        Renderer::EndFrame();
    }

    Renderer::Shutdown();
    android::ANativeWindowCreator::Destroy(window);
    window = nullptr;
}

int main(int argc, char *argv[])
{

    if (Utils::is_package_running("your.game.package"))
        target_pid.store(Utils::find_pid_by_package_name("your.game.package"));
    else
    {
        Logger::i("Not open game or not support\n");
        exit(1);
        return -1;
    }

    lib_base.store(Utils::find_ue4_base(target_pid.load(std::memory_order_relaxed)));
    if (!lib_base.load(std::memory_order_relaxed))
    {
        Logger::i("Failed to find libUE4.so base address!\n");
        exit(1);
        return -1;
    }

    android::ANativeWindowCreator::DisplayInfo display = android::ANativeWindowCreator::GetDisplayInfo();
    if (display.width < display.height)
        std::swap(display.width, display.height);

    is_running.store(true);
    TouchInput::touchInputStart();
    TouchInput::setDisplayInfo(display.width, display.height, display.orientation);
    drawing_thread_handle = std::thread(drawing_thread);

    float margin = 20.0f; // Margin around the screen area
    while (is_running.load(std::memory_order_relaxed))
    {
        Utils::control_frame_rate(frame_rate.load(std::memory_order_relaxed));
        int write_idx = write_buffer_index.load(std::memory_order_relaxed);
        auto &current_buffer = game_buffers[write_idx];
        current_buffer.clear();

        uintptr_t u_world = Memory::Read<uintptr_t>(lib_base + Offset::g_world, target_pid);
        uintptr_t game_state = Memory::Read<uintptr_t>(u_world + Offset::game_state, target_pid);
        Structs::TArray player_array = Memory::Read<Structs::TArray>(game_state + Offset::player_array, target_pid);

        for (size_t i = 0; i < player_array.count; i++)
        {
            uintptr_t actor = Memory::Read<uintptr_t>(player_array.data + i * sizeof(uintptr_t), target_pid);
            if (!actor)
                continue;

            if (i + 1 < player_array.count)
            {
                __builtin_prefetch((void *)(player_array.data + (i + 1) * sizeof(uintptr_t)), 0, 1);
            }

            StructsGame::Player player_obj;
            if (current_buffer.count_enemies < 200)
            {
                current_buffer.players[current_buffer.count_enemies] = player_obj;
                current_buffer.count_enemies++;
            }
        }

        ready_buffer_index.store(write_idx, std::memory_order_release);
        write_buffer_index.store(1 - write_idx, std::memory_order_relaxed);
    }

    is_running.store(false, std::memory_order_release);
    if (drawing_thread_handle.joinable())
        drawing_thread_handle.join();
    TouchInput::touchInputStop();
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    return 0;
}