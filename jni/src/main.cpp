#include "main.h"

static std::unique_ptr<SocketServer> socket_server;
static FrameTimeController fps_controller;

void handle_command(const std::string &cmd)
{
    if (fps_controller.handle_fps_command(cmd))
    {
        return;
    }

    if (cmd == "START_SCAN")
    {
        LOGI("Starting scan...");
    }
    else if (cmd == "STOP_SCAN")
    {
        LOGI("Stopping scan...");
    }
    else if (cmd.substr(0, 10) == "SET_FILTER:")
    {
        std::string filter = cmd.substr(10);
        LOGI("Setting filter: %s", filter.c_str());
    }
}

bool init()
{
    LOGI("Initializing...");

    // socket_server = std::make_unique<SocketServer>("/data/local/tmp/game_data_socket");
    // socket_server->set_command_callback(handle_command);
    // if (!socket_server->start())
    // {
    //     LOGE("Failed to start socket server");
    //     return false;
    // }

    // fps_controller.set_send_data_callback([](const std::string &msg)
    //                                       {
    //    if (socket_server) {
    //     socket_server->send_data(msg);
    //     } });
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
    uintptr_t g_names = Memory::Read<uintptr_t>(Memory::Read<uintptr_t>(lib_base + Offset::g_name, target_pid) + 0x110, target_pid);
    while (is_running)
    {
        fps_controller.start_frame();

        uintptr_t u_world = Memory::Read<uintptr_t>(Memory::Read<uintptr_t>(g_world + 0x58, target_pid) + 0x78, target_pid);
        // uintptr_t player_controller = Memory::Read<uintptr_t>(Memory::Read<uintptr_t>(Memory::Read<uintptr_t>(u_world + 0x38, target_pid) + 0x78, target_pid) + 0x30, target_pid);
        // uintptr_t u_world = Memory::Read<uintptr_t>(g_world + 0x20, target_pid);
        uintptr_t u_level = Memory::Read<uintptr_t>(u_world + Offset::persistent_level, target_pid);
        uintptr_t entity_list = Ue4::get_actors_array(u_level, Offset::u_level_to_a_actors, 0x448, target_pid);
        uintptr_t u_level_to_a_actors = Memory::Read<uintptr_t>(entity_list, target_pid);
        int u_level_to_a_actors_count = Memory::Read<int>(entity_list + sizeof(uintptr_t), target_pid);

        if (u_level_to_a_actors_count < 0 || u_level_to_a_actors_count > 1000)
        {
            printf("Invalid u_level_to_aActors_count: %d\n", u_level_to_a_actors_count);
            continue;
        }

        for (int i = 0; i < u_level_to_a_actors_count; i++)
        {
            uintptr_t actor = Memory::Read<uintptr_t>(u_level_to_a_actors + i * sizeof(uintptr_t), target_pid);
            if (!actor)
            {
                continue;
            }

            std::string class_name = Ue4::get_g_names(g_names, actor, target_pid);
            if (class_name == "none" || class_name.empty())
            {
                continue;
            }

            printf("Class Name: %s\n", class_name.c_str());

            // if (Utils::is_contains(class_name, "BP_STPlayerStart") || Utils::is_contains(class_name, "BP_PlayerPawn") || Utils::is_contains(class_name, "BP_PlayerState"))
            // {

            //     uintptr_t actor_root_component = Memory::Read<uintptr_t>(actor + Offset::root_component, target_pid);
            //     if (actor_root_component == 0)
            //     {
            //         continue;
            //     }

            //     Structs::FVector actor_location = Memory::Read<Structs::FVector>(actor_root_component + Offset::relative_location, target_pid);
            //     Structs::FRotator actor_rotation = Memory::Read<Structs::FRotator>(actor_root_component + Offset::relative_rotation, target_pid);

            //     printf("Class Name: %s\n", class_name.c_str());
            //     // socket_server->send_data(class_name + "\n");
            // }
        }

        std::string command;
        while (socket_server->get_next_command(command))
        {
            handle_command(command);
        }

        fps_controller.end_frame();
    }

    is_running = false;
    if (socket_server)
    {
        socket_server->stop();
    }
    return 0;
}