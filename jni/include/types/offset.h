#ifndef OFFSETS_H
#define OFFSETS_H

#include <cstdint>

namespace Offset
{
    //-- Base Offset G-Objects
    constexpr uintptr_t g_world = 0x0000000;
    constexpr uintptr_t u_level_to_a_actors = 0x98; // In most versions of UE4, it is | 0x98 |

    //-- Base Offset UWorld

    // struct UWorld : UObject
    constexpr uintptr_t persistent_level = 0x00; // struct ULevel* PersistentLevel;
    constexpr uintptr_t game_state = 0x00;      // struct AGameStateBase* GameState;

    // struct AGameStateBase : AInfo
    constexpr uintptr_t player_array = 0x00; // struct TArray<struct APlayerState*> PlayerArray;
}
#endif // OFFSETS_H
