#ifndef UE4_H
#define UE4_H
#include <math.h>
#include "memory.h"

namespace Ue4
{

    struct FVector
    {
        float X;
        float Y;
        float Z;

        FVector() : X(0.f), Y(0.f), Z(0.f) {}
        FVector(float x, float y, float z) : X(x), Y(y), Z(z) {}

        float Distance(const FVector &v) const
        {
            float dx = X - v.X;
            float dy = Y - v.Y;
            float dz = Z - v.Z;
            return sqrt(dx * dx + dy * dy + dz * dz);
        }

        FVector operator-(const FVector &v) const
        {
            return FVector(X - v.X, Y - v.Y, Z - v.Z);
        }

        FVector operator+(const FVector &v) const
        {
            return FVector(X + v.X, Y + v.Y, Z + v.Z);
        }

        FVector operator*(float scale) const
        {
            return FVector(X * scale, Y * scale, Z * scale);
        }
    };

    struct FRotator
    {
        float Pitch;
        float Yaw;
        float Roll;

        FRotator() : Pitch(0.f), Yaw(0.f), Roll(0.f) {}
        FRotator(float pitch, float yaw, float roll) : Pitch(pitch), Yaw(yaw), Roll(roll) {}
    };

    std::string get_class_name(uintptr_t actor, uintptr_t g_names_base, pid_t pid)
    {
        if (actor == 0)
            return "";

        int class_id = Memory::Read<int>(actor + 8 + 2 * sizeof(uintptr_t), pid);

        int page = class_id / 0x4000;
        int index = class_id % 0x4000;

        if (page < 1 || page > 30)
            return "";

        uintptr_t page_ptr = Memory::Read<uintptr_t>(g_names_base + page * sizeof(uintptr_t), pid);
        if (page_ptr == 0)
            return "";

        uintptr_t name_ptr = Memory::Read<uintptr_t>(page_ptr + index * sizeof(uintptr_t), pid);
        if (name_ptr == 0)
            return "";

        return Memory::ReadString(name_ptr, pid);
    }

} // namespace Ue4

#endif // UE4_H