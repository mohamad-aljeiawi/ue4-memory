#ifndef OFFSETS_H
#define OFFSETS_H

#include <cstdint>

namespace Offset
{
    //-- Base Offset G-Objects
    uintptr_t g_name = 0xD66B2C0;
    uintptr_t g_world = 0xdc10ff0;
    //-- Base Offset UWorld
    uintptr_t persistent_level = 0x30;
    uintptr_t u_level_to_a_actors = 0xA0;
    uintptr_t draw_shoot_line_time = 0x13c;
    uintptr_t component_to_world = 0x1b0;



    //-- Class: PlayerController
    uintptr_t player_camera_manager = 0x4d0; // PlayerCameraManager* PlayerCameraManager;
    //-- Class: PlayerCameraManager
    uintptr_t camera_cache = 0x4b0; // CameraCacheEntry CameraCache;
    //-- Class: Actor
    uintptr_t role = 0x150;           // byte Role;
    uintptr_t children = 0x1a0;       // Actor*[] Children;
    uintptr_t root_component = 0x1b0; // SceneComponent* RootComponent;
    //-- Class: SceneComponent
    uintptr_t relative_location = 0x184; // Vector RelativeLocation;
    uintptr_t relative_rotation = 0x190; // Rotator RelativeRotation;
    //-- Class: Character
    uintptr_t mesh = 0x498; // SkeletalMeshComponent* Mesh;
    //-- Class: StaticMeshComponent
    uintptr_t min_lob = 0x878; // StaticMesh* StaticMesh;
    //-- Class: UAEPlayerController
    uintptr_t local_player_Key = 0x888; // uint32 PlayerKey;
    uintptr_t local_team_id = 0x8a8;    // int TeamID;
    //-- Class: UAECharacter
    uintptr_t player_name = 0x8f0; // FString PlayerName;
    uintptr_t player_key = 0x910;  // uint32 PlayerKey;
    uintptr_t team_id = 0x938;     // int TeamID;
    uintptr_t bis_ai = 0x9c0;      // bool bEnsure;
    //-- Class: STExtraBaseCharacter
    uintptr_t bis_weapon_firing = 0x1640; // bool bIsWeaponFiring;
    //-- Class: STExtraCharacter
    uintptr_t bis_weapon_ads = 0x1059; // bool bIsGunADS;
    uintptr_t health = 0xdc0;          // float Health;
    uintptr_t bis_dead = 0xddc;        // bool bDead;
    uintptr_t current_states = 0xf88;  // uint64 CurrentStates;
    //-- Class: ItemDropMgrComponent
    uintptr_t grid_step = 0x1c0; // int GridStep;
    //-- Class: STExtraWeapon
    uintptr_t weapon_entity_comp = 0x848; // WeaponEntity* WeaponEntityComp;
    //-- Class: WeaponEntity
    uintptr_t weapon_id = 0x178; // int WeaponId;
    //-- Class: STExtraVehicleBase
    uintptr_t vehicle_common = 0xa28; // VehicleCommonComponent* VehicleCommon;
    //-- Class: VehicleCommonComponent
    uintptr_t hp_max = 0x2c0;   // float HPMax;
    uintptr_t hp = 0x2c4;       // float HP;
    uintptr_t fuel_max = 0x334; // float FuelMax;
    uintptr_t fuel = 0x33c;     // float Fuel;

    //--Class: STExtraShootWeapon
    uintptr_t ShootWeaponEntityComp = 0xff8;
    //--Class: STExtraBaseCharacter
    uintptr_t WeaponManagerComponent = 0x2238;
    uintptr_t ThirdPersonCameraComponent = 0x1a28;
    //--Class: STExtraPlayerCharacter
    uintptr_t STPlayerController = 0x3de0;
    //--Class: CameraComponent
    uintptr_t FieldOfView = 0x32c;
    //--Class: WeaponManagerComponent
    uintptr_t CurrentWeaponReplicated = 0x500;
    //--Class: ShootWeaponEntity
    uintptr_t GameDeviationFactor = 0xba0;
    uintptr_t AutoAimingConfig = 0x990;
    uintptr_t RecoilInfo = 0xab8;
}
#endif // OFFSETS_H
