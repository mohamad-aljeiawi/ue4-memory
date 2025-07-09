#include "utils/ue4.h"
#include "utils/memory.h"
#include "types/offset.h"

#include <cmath>
#include <cstring>
#include <vector>
#include <unordered_map>
#include <cfloat>
#include <cstdint>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace Ue4
{
    std::string get_cached_class_name(uintptr_t g_names, uintptr_t actor, uintptr_t gname_buff[100], std::unordered_map<int, std::string> &g_class_name_cache, pid_t target_pid)
    {
        int class_id = Memory::Read<int>(actor + 0x18, target_pid);

        auto it = g_class_name_cache.find(class_id);
        if (it != g_class_name_cache.end())
        {
            return it->second;
        }

        int page = class_id / 0x4000;
        int index = class_id % 0x4000;

        if (gname_buff[page] == 0)
        {
            gname_buff[page] = Memory::Read<uintptr_t>(g_names + page * sizeof(uintptr_t), target_pid);
        }

        uintptr_t entry_ptr = Memory::Read<uintptr_t>(gname_buff[page] + index * sizeof(uintptr_t), target_pid);
        if (!entry_ptr)
        {
            g_class_name_cache.emplace(class_id, "none");
            return "none";
        }

        std::string name = Memory::ReadFName(entry_ptr, target_pid);

        g_class_name_cache.emplace(class_id, name);

        return name;
    }

    std::string get_g_names(uintptr_t g_names, uintptr_t actor, uintptr_t gname_buff[100], pid_t target_pid)
    {
        int class_id = Memory::Read<int>(actor + 0x18, target_pid);
        int page = class_id / 0x4000;
        int index = class_id % 0x4000;

        if (gname_buff[page] == 0)
        {
            gname_buff[page] = Memory::Read<uintptr_t>(g_names + page * sizeof(uintptr_t), target_pid);
        }

        uintptr_t entry_ptr = Memory::Read<uintptr_t>(gname_buff[page] + index * sizeof(uintptr_t), target_pid);
        if (!entry_ptr)
            return "none";

        return Memory::ReadFName(entry_ptr, target_pid);
    }

    Structs::FMatrix rotator_to_matrix(Structs::FRotator rotation)
    {
        float radPitch = rotation.Pitch * ((float)M_PI / 180.0f);
        float radYaw = rotation.Yaw * ((float)M_PI / 180.0f);
        float radRoll = rotation.Roll * ((float)M_PI / 180.0f);

        float SP = sinf(radPitch);
        float CP = cosf(radPitch);
        float SY = sinf(radYaw);
        float CY = cosf(radYaw);
        float SR = sinf(radRoll);
        float CR = cosf(radRoll);

        Structs::FMatrix matrix;

        matrix.M[0][0] = (CP * CY);
        matrix.M[0][1] = (CP * SY);
        matrix.M[0][2] = (SP);
        matrix.M[0][3] = 0;

        matrix.M[1][0] = (SR * SP * CY - CR * SY);
        matrix.M[1][1] = (SR * SP * SY + CR * CY);
        matrix.M[1][2] = (-SR * CP);
        matrix.M[1][3] = 0;

        matrix.M[2][0] = (-(CR * SP * CY + SR * SY));
        matrix.M[2][1] = (CY * SR - CR * SP * SY);
        matrix.M[2][2] = (CR * CP);
        matrix.M[2][3] = 0;

        matrix.M[3][0] = 0;
        matrix.M[3][1] = 0;
        matrix.M[3][2] = 0;
        matrix.M[3][3] = 1;

        return matrix;
    }

    Structs::FVector rotator_to_vector(const Structs::FRotator &r)
    {
        float DEG2RAD = 3.14159265f / 180.f;
        float cp = std::cos(r.Pitch * DEG2RAD);
        float sp = std::sin(r.Pitch * DEG2RAD);
        float cy = std::cos(r.Yaw * DEG2RAD);
        float sy = std::sin(r.Yaw * DEG2RAD);

        return Structs::FVector(cp * cy, // X
                                cp * sy, // Y
                                sp);     // Z
    }

    Structs::FVector cross(const Structs::FVector &a, const Structs::FVector &b)
    {
        return Structs::FVector(
            a.Y * b.Z - a.Z * b.Y,
            a.Z * b.X - a.X * b.Z,
            a.X * b.Y - a.Y * b.X);
    }

    Structs::FVector world_to_screen(Structs::FVector worldLocation, Structs::MinimalViewInfo camViewInfo, int screenWidth, int screenHeight)
    {
        Structs::FMatrix tempMatrix = rotator_to_matrix(camViewInfo.Rotation);

        Structs::FVector vAxisX(tempMatrix.M[0][0], tempMatrix.M[0][1], tempMatrix.M[0][2]);
        Structs::FVector vAxisY(tempMatrix.M[1][0], tempMatrix.M[1][1], tempMatrix.M[1][2]);
        Structs::FVector vAxisZ(tempMatrix.M[2][0], tempMatrix.M[2][1], tempMatrix.M[2][2]);

        Structs::FVector vDelta = worldLocation - camViewInfo.Location;

        Structs::FVector vTransformed(Structs::FVector::Dot(vDelta, vAxisY), Structs::FVector::Dot(vDelta, vAxisZ), Structs::FVector::Dot(vDelta, vAxisX));

        float fov = camViewInfo.FOV;
        float screenCenterX = (screenWidth / 2.0f);
        float screenCenterY = (screenHeight / 2.0f);

        float X = (screenCenterX + vTransformed.X * (screenCenterX / tanf(fov * ((float)M_PI / 360.0f))) / vTransformed.Z);
        float Y = (screenCenterY - vTransformed.Y * (screenCenterX / tanf(fov * ((float)M_PI / 360.0f))) / vTransformed.Z);
        float Z = vTransformed.Z;

        return {X, Y, Z};
    }

}
