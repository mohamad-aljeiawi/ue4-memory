#ifndef STRUCTS_H
#define STRUCTS_H

#include <cstdint>
#include <math.h>

namespace Structs
{
    struct Actors
    {
        uint64_t enc_1, enc_2;
        uint64_t enc_3, enc_4;
    };

    struct Chunk
    {
        uint32_t val_1, val_2, val_3, val_4;
        uint32_t val_5, val_6, val_7, val_8;
    };

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

    struct MinimalViewInfo
    {
        FVector Location;
        FVector LocationLocalSpace;
        FRotator Rotation;
        float FOV;
    };

    struct CameraCacheEntry
    {
        float TimeStamp;
        char chunks[0xC];
        MinimalViewInfo POV;
    };

    struct FString
    {
        uintptr_t data;
        int count;
        int max;
    };

} // namespace Structs

#endif // STRUCTS_H