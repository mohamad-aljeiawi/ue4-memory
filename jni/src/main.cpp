#include "main.h"

static std::unique_ptr<SocketServer> socket_server;
static FrameTimeController fps_controller;

void handle_command(const std::string &cmd)
{
    // if (fps_controller.handle_fps_command(cmd))
    // {
    //     return;
    // }

    // if (cmd == "START_SCAN")
    // {
    //     LOGI("Starting scan...");
    // }
    // else if (cmd == "STOP_SCAN")
    // {
    //     LOGI("Stopping scan...");
    // }
    // else if (cmd.substr(0, 10) == "SET_FILTER:")
    // {
    //     std::string filter = cmd.substr(10);
    //     LOGI("Setting filter: %s", filter.c_str());
    // }
}

bool init()
{
    LOGI("Initializing...");

    socket_server = std::make_unique<SocketServer>("/data/local/tmp/cping_memory_socket");
    socket_server->set_command_callback(handle_command);
    if (!socket_server->start())
    {
        LOGE("Failed to start socket server");
        return false;
    }

    fps_controller.set_send_data_callback([](const std::string &msg)
                                          {
                                                 if (socket_server) {
                                                  socket_server->send_data(msg);
                                                  } });
    fps_controller.set_target_fps(40);

    is_running = true;
    return true;
}

int main(int argc, char *argv[])
{
    const std::string package_name = "com.tencent.ig";
    pid_t game_pid = Utils::find_pid_by_package_name(package_name);

    if (!game_pid)
    {
        printf("PUBG Mobile is not running!\n");
        return -1;
    }

    uintptr_t ue4_base = Utils::find_ue4_base(game_pid);
    if (!ue4_base)
    {
        printf("Failed to find libUE4.so base address!\n");
        return -1;
    }

    target_pid = game_pid;
    lib_base = ue4_base;

    if (!init())
    {
        printf("Failed to initialize!\n");
        return -1;
    }

    uintptr_t g_world = Memory::Read<uintptr_t>(lib_base + Offset::g_world, target_pid);
    while (is_running)
    {
        fps_controller.start_frame();

        uintptr_t world_context = Memory::Read<uintptr_t>(g_world + 0x58, target_pid);                                    // WorldContext or Subsystem
        uintptr_t u_world = Memory::Read<uintptr_t>(world_context + 0x78, target_pid);                                    // UWorld
        uintptr_t persistent_level = Memory::Read<uintptr_t>(u_world + 0x30, target_pid);                                 // PersistentLevel
        uintptr_t actors_array = Ue4::get_actors_array(persistent_level, Offset::u_level_to_a_actors, 0x448, target_pid); // AActors[]
        int actors_array_count = Memory::Read<int>(actors_array + sizeof(uintptr_t), target_pid);                         // AActors

        for (int i = 0; i < actors_array_count; i++)
        {
            uintptr_t actor = Memory::Read<uintptr_t>(actors_array + i * sizeof(uintptr_t), target_pid);
            if (!actor)
            {
                continue;
            }

            uintptr_t mesh = Memory::Read<uintptr_t>(actor + 0x498, target_pid);
            if (!mesh)
                continue;

            std::string player_name_str = Memory::ReadFString(Memory::Read<uintptr_t>(actor + 0x8f0, target_pid), target_pid);
            printf("Player name: %s\n", player_name_str.c_str());
            // float health = Memory::Read<float>(actor + 0xdb0, target_pid);
            // printf("Health: %f\n", health);
        }

        std::string command;
        while (socket_server->get_next_command(command))
        {
            handle_command(command);
        }

        fps_controller.end_frame();
    }

    printf("is running: %d\n", is_running);

    is_running = false;
    if (socket_server)
    {
        socket_server->stop();
    }
    return 0;
}