#ifndef STRUCTS_H
#define STRUCTS_H

#include <cstdint>
#include <math.h>

namespace Structs
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

        static float Dot(const FVector &lhs, const FVector &rhs)
        {
            return lhs.X * rhs.X + lhs.Y * rhs.Y + lhs.Z * rhs.Z;
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

    struct FQuaternion
    {
        union
        {
            struct
            {
                float X;
                float Y;
                float Z;
                float W;
            };
            float data[4];
        };
    };

    struct FString
    {
        uintptr_t data;
        int count;
        int max;
    };

    struct FMatrix
    {
        float M[4][4];
    };

    struct D3DMatrix
    {
        float _11, _12, _13, _14;
        float _21, _22, _23, _24;
        float _31, _32, _33, _34;
        float _41, _42, _43, _44;
    };

    struct FTransform
    {
        FQuaternion Rotation;
        FVector Translation;
        char pad[0x4];
        FVector Scale3D;
        D3DMatrix ToMatrixWithScale()
        {
            D3DMatrix m;
            m._41 = Translation.X;
            m._42 = Translation.Y;
            m._43 = Translation.Z;

            float x2 = Rotation.X + Rotation.X;
            float y2 = Rotation.Y + Rotation.Y;
            float z2 = Rotation.Z + Rotation.Z;

            float xx2 = Rotation.X * x2;
            float yy2 = Rotation.Y * y2;
            float zz2 = Rotation.Z * z2;
            m._11 = (1.0f - (yy2 + zz2)) * Scale3D.X;
            m._22 = (1.0f - (xx2 + zz2)) * Scale3D.Y;
            m._33 = (1.0f - (xx2 + yy2)) * Scale3D.Z;

            float yz2 = Rotation.Y * z2;
            float wx2 = Rotation.W * x2;
            m._32 = (yz2 - wx2) * Scale3D.Z;
            m._23 = (yz2 + wx2) * Scale3D.Y;

            float xy2 = Rotation.X * y2;
            float wz2 = Rotation.W * z2;
            m._21 = (xy2 - wz2) * Scale3D.Y;
            m._12 = (xy2 + wz2) * Scale3D.X;

            float xz2 = Rotation.X * z2;
            float wy2 = Rotation.W * y2;
            m._31 = (xz2 + wy2) * Scale3D.Z;
            m._13 = (xz2 - wy2) * Scale3D.X;

            m._14 = 0.0f;
            m._24 = 0.0f;
            m._34 = 0.0f;
            m._44 = 1.0f;

            return m;
        }
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

    struct TArray
    {
        uintptr_t data;
        int count;
        int max;
    };

} // namespace Structs

#endif // STRUCTS_H