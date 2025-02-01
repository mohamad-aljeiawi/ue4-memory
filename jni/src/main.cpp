#include "main.h"

bool init()
{
    LOGI("Initializing...");

    // Initialize your code here
    is_running = true;
    return true;
}

int main(int argc, char *argv[])
{
    printf("Starting...\n");

    const std::string package_name = "com.tencent.ig";
    pid_t game_pid = Utils::find_pid_by_package_name(package_name);

    if (game_pid <= 0)
    {
        printf("PUBG Mobile is not running!\n");
        return -1;
    }
    printf("Found PUBG Mobile process: %d\n", game_pid);

    uintptr_t ue4_base = Utils::find_ue4_base(game_pid);
    if (ue4_base == 0)
    {
        printf("Failed to find libUE4.so base address!\n");
        return -1;
    }
    printf("Found libUE4.so base: 0x%lX\n", ue4_base);

    target_pid = game_pid;
    lib_base = ue4_base;
    is_running = true;

    uintptr_t g_world = Memory::Read<uintptr_t>(lib_base + Offset::g_world, target_pid);
    // printf("g_world: 0x%lX\n", g_world);
    uintptr_t g_names = Memory::Read<uintptr_t>(Memory::Read<uintptr_t>(lib_base + Offset::g_name, target_pid) + 0x110, target_pid);
    // printf("g_names: 0x%lX\n", g_names);
    while (is_running)
    {
        uintptr_t u_world = Memory::Read<uintptr_t>(Memory::Read<uintptr_t>(g_world + 0x58, target_pid) + 0x78, target_pid);
        // printf("u_world: 0x%lX\n", u_world);
        uintptr_t u_level = Memory::Read<uintptr_t>(u_world + Offset::persistent_level, target_pid);
        // printf("u_level: 0x%lX\n", u_level);
        uintptr_t entity_list = Utils::get_actors_array(u_level, Offset::u_level_to_a_actors, 0x448, target_pid);
        // printf("entity_list: 0x%lX\n", entity_list);
        uintptr_t u_level_to_a_actors = Memory::Read<uintptr_t>(entity_list, target_pid);
        // printf("u_level_to_aActors: 0x%lX\n", u_level_to_aActors);
        int u_level_to_a_actors_count = Memory::Read<int>(entity_list + sizeof(uintptr_t), target_pid);
        // printf("u_level_to_aActors_count: %d\n", u_level_to_a_actors_count);

        // cheack if the count is valid
        if (u_level_to_a_actors_count < 0 || u_level_to_a_actors_count > 1000)
        {
            printf("Invalid u_level_to_aActors_count: %d\n", u_level_to_a_actors_count);
            continue;
        }

        for (int i = 0; i < u_level_to_a_actors_count; i++)
        {
            uintptr_t actor = Memory::Read<uintptr_t>(u_level_to_a_actors + i * sizeof(uintptr_t), target_pid);
            if (actor == 0)
            {
                continue;
            }

            uintptr_t g_names = Memory::Read<uintptr_t>(Memory::Read<uintptr_t>(lib_base + Offset::g_name, target_pid) + 0x110, target_pid);
            if (g_names == 0) continue;

            // Create array to cache GName table pointers
            std::vector<uintptr_t> gname_buff(30, 0);
            // Read first entry to verify
            gname_buff[0] = Memory::Read<uintptr_t>(g_names, target_pid);
            if (gname_buff[0] == 0) continue;

            int class_id = Memory::Read<int>(actor + 8 + 2 * sizeof(uintptr_t), target_pid);
            int page = class_id / 0x4000;
            int index = class_id % 0x4000;

            // Validate page number
            if (page < 1 || page > 30) continue;

            // Lazy load the page if not cached
            if (gname_buff[page] == 0) {
                gname_buff[page] = Memory::Read<uintptr_t>(g_names + page * sizeof(uintptr_t), target_pid);
            }

            // Get name entry
            uintptr_t name_entry = Memory::Read<uintptr_t>(gname_buff[page] + index * sizeof(uintptr_t), target_pid);
            if (name_entry == 0) continue;

            // Read the actual string data
            char name_buffer[256] = {0};
            uintptr_t string_ptr = name_entry + 4 + sizeof(uintptr_t); // Skip header
            size_t read_size = 0;
            
            while (read_size < sizeof(name_buffer) - 1) {
                char c = Memory::Read<char>(string_ptr + read_size, target_pid);
                if (c == 0) break;
                name_buffer[read_size++] = c;
            }

            if (read_size > 0 && Utils::is_printable_ascii(name_buffer)) {
                printf("  Class Name: %s\n", name_buffer);
            }

            // std::string class_name = Ue4::get_class_name(actor, g_names, target_pid);
            // printf("Actor %d: %s\n", i, class_name.c_str());

            // int actor_id = Memory::Read<int>(actor + 0x18, target_pid);
            // if (actor_id <= 0)
            //     continue;

            // uintptr_t g_names = Memory::Read<uintptr_t>(lib_base + Offset::g_name, target_pid);
            // uintptr_t name_entry = Memory::Read<uintptr_t>(g_names + (actor_id >> 0x10) * sizeof(uintptr_t), target_pid);
            // if (name_entry == 0)
            //     continue;

            // name_entry = Memory::Read<uintptr_t>(name_entry + (actor_id & 0xFFFF) * sizeof(uintptr_t), target_pid);
            // if (name_entry == 0)
            //     continue;

            // int name_length = Memory::Read<uint16_t>(name_entry + 0x10, target_pid);
            // if (name_length <= 0 || name_length > 100)
            //     continue;

            // std::string actor_type = Memory::ReadString(name_entry + 0x14, target_pid);

            // printf("Actor %d: %s\n", i, actor_type.c_str());

            //     std::string player_name = Memory::ReadString(actor + Offset::player_name, target_pid);
            //     if (player_name.empty())
            //     {
            //         continue;
            //     }

            //     int team_id = Memory::Read<int>(actor + Offset::team_id, target_pid);
            //     if (team_id <= 0 || team_id > 100)
            //     {
            //         continue;
            //     }

            //     float health = Memory::Read<float>(actor + Offset::health, target_pid);
            //     if (health <= 0)
            //     {
            //         continue;
            //     }

            //     bool is_bot = Memory::Read<bool>(actor + Offset::bis_ai, target_pid);

            //     uintptr_t actor_root_component = Memory::Read<uintptr_t>(actor + Offset::root_component, target_pid);
            //     if (actor_root_component == 0)
            //     {
            //         continue;
            //     }

            //     Ue4::FVector actor_location = Memory::Read<Ue4::FVector>(actor_root_component + Offset::relative_location, target_pid);
            //     Ue4::FRotator actor_rotation = Memory::Read<Ue4::FRotator>(actor_root_component + Offset::relative_rotation, target_pid);

            //     printf("Player %d [%s] Team=%d Bot=%s Health=%.0f Location=(%0.1f, %0.1f, %0.1f)\n",
            //            i,
            //            player_name.c_str(),
            //            team_id,
            //            is_bot ? "Yes" : "No",
            //            health,
            //            actor_location.X, actor_location.Y, actor_location.Z);
        }
    }

    uint32_t test_value = Memory::Read<uint32_t>(lib_base, target_pid);
    printf("Test read from base: 0x%X\n", test_value);

    is_running = false;
    return 0;
}