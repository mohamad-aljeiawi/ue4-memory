#ifndef STRUCTS_GAME_H
#define STRUCTS_GAME_H

#include <cstdint>
#include <math.h>
#include <cmath>
#include <string>
#include <vector>
#include "imgui/imgui.h"
#include "types/structs.h"

namespace StructsGame
{

    struct Player
    {
        Structs::FVector position; // 3D position world
        Structs::FVector location;
        Structs::FVector head;
        Structs::FVector root;
        Structs::FVector target;
        float health;
        float distance;
        int team_id;
        int camp_id;
        int weapon_id;
        char name[256];
        char state[256];
        Structs::FVector bounds[8];
        Structs::FVector bones[13];
        bool is_bot;
        bool is_alive;
        bool is_on_screen;
    };

    struct GameData
    {
        Player players[200];
        Structs::MinimalViewInfo minimal_view_info;
        int count_enemies;

        void clear()
        {
            for (int i = 0; i < 200; i++)
            {
                players[i] = Player();
            }
            count_enemies = 0;
            minimal_view_info = Structs::MinimalViewInfo();
        }

        void reserve_capacity()
        {
            for (int i = 0; i < 200; i++)
            {
                players[i] = Player();
            }
        }
    };

} // namespace StructsGame

#endif // STRUCTS_GAME_H