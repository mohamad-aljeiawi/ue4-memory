#include "main.h"

static std::unique_ptr<SocketServer> socket_server;
static FrameTimeController fps_controller;

void handle_command(const std::string &cmd)
{
    if (fps_controller.handle_fps_command(cmd))
        return;

    if (cmd == "START_SCAN")
    {
        LOGI("Starting scan...");
    }
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
    fps_controller.set_target_fps(60);

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
        // uintptr_t u_world = Memory::Read<uintptr_t>(g_world + 0x20, target_pid);
        uintptr_t u_level = Memory::Read<uintptr_t>(u_world + Offset::persistent_level, target_pid);
        uintptr_t entity_list = Ue4::get_actors_array(u_level, Offset::u_level_to_a_actors, 0x448, target_pid);
        uintptr_t u_level_to_a_actors = Memory::Read<uintptr_t>(entity_list, target_pid);
        int u_level_to_a_actors_count = Memory::Read<int>(entity_list + sizeof(uintptr_t), target_pid);

        uintptr_t game_state = Memory::Read<uintptr_t>(u_world + 0x1D8, target_pid);
        Structs::TArray player_array = Memory::Read<Structs::TArray>(game_state + 0x450, target_pid);
        // printf("player_array.data: %p\n", player_array.data);
        printf("player_array.count: %d\n", player_array.count);
        // printf("player_array.max: %d\n", player_array.max);
        std::vector<uintptr_t> players = Memory::ReadArray<uintptr_t>(player_array.data, player_array.count, target_pid);

        // for (int i = 0; i < players.size(); i++)
        // {
        //     uintptr_t player = players[i];
        //     printf("Player: %d\n", player);
        // }

        // if (u_level_to_a_actors_count <= 0 || u_level_to_a_actors_count >= 1000)
        //     continue;

        // printf("u_level_to_a_actors_count: %d\n", u_level_to_a_actors_count);

        // uintptr_t player_controller = Memory::Read<uintptr_t>(Memory::Read<uintptr_t>(Memory::Read<uintptr_t>(u_world + 0x38, target_pid) + 0x78, target_pid) + 0x30, target_pid);

        // for (int i = 0; i < u_level_to_a_actors_count; i++)
        // {

            // uintptr_t actor = Memory::Read<uintptr_t>(u_level_to_a_actors + i * sizeof(uintptr_t), target_pid);
            // if (!actor)
            //     continue;

            // std::string class_name = Ue4::get_g_names(g_names, actor, target_pid);
            // if (class_name == "none" || class_name.empty())
            //     continue;

            // uintptr_t actor_root_component = Memory::Read<uintptr_t>(actor + Offset::root_component, target_pid);
            // if (!actor_root_component)
            //     continue;

            // Structs::FVector actor_location = Memory::Read<Structs::FVector>(actor_root_component + Offset::relative_location, target_pid);
            // printf("Actor Location: %f, %f, %f\n", actor_location.X, actor_location.Y, actor_location.Z);

            // // Skip Actor
            // if (Utils::is_contains(class_name, "VH_PG117_C") ||
            //     Utils::is_contains(class_name, "BP_CharacterRifle") ||
            //     Utils::is_contains(class_name, "BP_PlayerRifle") ||
            //     Utils::is_contains(class_name, "BP_PlayerLobby_Pawn_C") ||
            //     Utils::is_contains(class_name, "BP_PlayerLobby_Pawn_C") ||
            //     Utils::is_contains(class_name, "BP_PlayerController"))
            //     continue;

            // if (Utils::is_contains(class_name, "BP_Character") ||
            //     Utils::is_contains(class_name, "BP_Player"))
            // {

            // uintptr_t actor_root_component = Memory::Read<uintptr_t>(actor + Offset::root_component, target_pid);
            // if (!actor_root_component)
            //     continue;

            // uintptr_t mesh = Memory::Read<uintptr_t>(actor + Offset::mesh, target_pid);
            // if (!mesh)
            //     continue;

            // int bis_dead = Memory::Read<int>(actor + Offset::bis_dead, target_pid);
            // if (bis_dead)
            //     continue;

            // int current_states = Memory::Read<int>(actor + Offset::current_states, target_pid);
            // if (current_states == 262144 || current_states == 6)
            //     continue;

            // int local_team_id = Memory::Read<int>(player_controller + Offset::local_team_id, target_pid);
            // int team_id = Memory::Read<int>(actor + Offset::team_id, target_pid);
            // if (local_team_id == team_id || team_id <= -1 || team_id >= 1000)
            //     continue;

            // float health = Memory::Read<float>(actor + Offset::health, target_pid);
            // if (health < 0)
            //     continue;

            // int is_bot = Memory::Read<int>(actor + Offset::bis_ai, target_pid);

            // Structs::FVector head_location = Ue4::get_bone_location(actor, 6);
            // Structs::FVector root_location = Ue4::get_bone_location(actor, 0);

            // printf("Head Location: %f, %f, %f\n", head_location.X, head_location.Y, head_location.Z);

            // Structs::FVector actor_location = Memory::Read<Structs::FVector>(actor_root_component + Offset::relative_location, target_pid);

            // std::string player_name_str = Memory::ReadFString(Memory::Read<uintptr_t>(actor + 0x8f0, target_pid), target_pid);
            // printf("Player name: %s\n", player_name_str.c_str());

            // Structs::FVector actor_location = Memory::Read<Structs::FVector>(actor_root_component + Offset::relative_location, target_pid);
            // Structs::FRotator actor_rotation = Memory::Read<Structs::FRotator>(actor_root_component + Offset::relative_rotation, target_pid);

            // printf("Class Name: %s\n", class_name.c_str());
            // socket_server->send_data(class_name + "\n");
            // }
        // }

        // std::string command;
        // while (socket_server->get_next_command(command))
        // {
        //     handle_command(command);
        // }

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