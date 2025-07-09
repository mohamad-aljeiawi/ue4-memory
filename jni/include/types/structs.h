#ifndef STRUCTS_H
#define STRUCTS_H

#include <cstdint>
#include <math.h>
#include <cmath>
#include <string>
#include <vector>

namespace Structs
{
    struct FVector
    {
        float X;
        float Y;
        float Z;

        FVector() : X(0.f), Y(0.f), Z(0.f) {}
        FVector(float x, float y, float z) : X(x), Y(y), Z(z) {}

        float Length() const
        {
            return std::sqrt(X * X + Y * Y + Z * Z);
        }

        FVector GetSafeNormal(float tolerance = 1e-6f) const
        {
            float length = Length();
            if (length < tolerance)
                return FVector(0.f, 0.f, 0.f);
            return FVector(X / length, Y / length, Z / length);
        }

        float Distance(const FVector &v) const
        {
            float dx = X - v.X;
            float dy = Y - v.Y;
            float dz = Z - v.Z;
            return std::sqrt(dx * dx + dy * dy + dz * dz);
        }

        static float Distance(const FVector &a, const FVector &b)
        {
            float dx = a.X - b.X;
            float dy = a.Y - b.Y;
            float dz = a.Z - b.Z;
            return std::sqrt(dx * dx + dy * dy + dz * dz);
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

        FVector operator*(const FVector &v) const
        {
            return FVector(X * v.X, Y * v.Y, Z * v.Z);
        }

        FVector operator/(const FVector &v) const
        {
            return FVector(X / v.X, Y / v.Y, Z / v.Z);
        }

        static float Dot(const FVector &lhs, const FVector &rhs)
        {
            return lhs.X * rhs.X + lhs.Y * rhs.Y + lhs.Z * rhs.Z;
        }

        void Normalize()
        {
            float length = Length();
            if (length > 0.0001f)
            {
                X /= length;
                Y /= length;
                Z /= length;
            }
        }

        bool IsValid() const
        {
            return X == X && Y == Y && Z == Z && // NaN check
                   std::isfinite(X) && std::isfinite(Y) && std::isfinite(Z);
        }

        bool operator==(const FVector &v) const
        {
            return X == v.X && Y == v.Y && Z == v.Z;
        }
    };

    struct FVector2
    {
        float X;
        float Y;

        FVector2() : X(0.f), Y(0.f) {}
        FVector2(float x, float y) : X(x), Y(y) {}

        float Distance(const FVector2 &v) const
        {
            float dx = X - v.X;
            float dy = Y - v.Y;
            return sqrt(dx * dx + dy * dy);
        }

        FVector2 operator-(const FVector2 &v) const
        {
            return FVector2(X - v.X, Y - v.Y);
        }

        FVector2 operator+(const FVector2 &v) const
        {
            return FVector2(X + v.X, Y + v.Y);
        }

        FVector2 operator*(float scale) const
        {
            return FVector2(X * scale, Y * scale);
        }

        static float Dot(const FVector2 &lhs, const FVector2 &rhs)
        {
            return lhs.X * rhs.X + lhs.Y * rhs.Y;
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

        FQuaternion() : X(0.f), Y(0.f), Z(0.f), W(1.f) {}
        FQuaternion(float x, float y, float z, float w) : X(x), Y(y), Z(z), W(w) {}

        FQuaternion operator*(const FQuaternion &q) const
        {
            return FQuaternion(
                W * q.X + X * q.W + Y * q.Z - Z * q.Y,
                W * q.Y - X * q.Z + Y * q.W + Z * q.X,
                W * q.Z + X * q.Y - Y * q.X + Z * q.W,
                W * q.W - X * q.X - Y * q.Y - Z * q.Z);
        }

        FQuaternion operator+(const FQuaternion &q) const
        {
            return FQuaternion(X + q.X, Y + q.Y, Z + q.Z, W + q.W);
        }

        FQuaternion operator-(const FQuaternion &q) const
        {
            return FQuaternion(X - q.X, Y - q.Y, Z - q.Z, W - q.W);
        }

        FQuaternion operator/(float scalar) const
        {
            if (std::abs(scalar) > 0.0001f)
            {
                return FQuaternion(X / scalar, Y / scalar, Z / scalar, W / scalar);
            }
            return *this;
        }

        FQuaternion operator*(float scalar) const
        {
            return FQuaternion(X * scalar, Y * scalar, Z * scalar, W * scalar);
        }

        FQuaternion Conjugate() const
        {
            return FQuaternion(-X, -Y, -Z, W);
        }

        float Magnitude() const
        {
            return std::sqrt(X * X + Y * Y + Z * Z + W * W);
        }

        FQuaternion operator/(const FQuaternion &q) const
        {
            float magnitudeSquared = q.X * q.X + q.Y * q.Y + q.Z * q.Z + q.W * q.W;
            if (magnitudeSquared > 0.0001f)
            {
                FQuaternion conjugate = q.Conjugate();
                FQuaternion inverse = conjugate / magnitudeSquared;
                return (*this) * inverse;
            }
            return *this;
        }

        void Normalize()
        {
            float length = std::sqrt(X * X + Y * Y + Z * Z + W * W);
            if (length > 0.0001f)
            {
                X /= length;
                Y /= length;
                Z /= length;
                W /= length;
            }
        }

        FQuaternion &operator+=(const FQuaternion &q)
        {
            X += q.X;
            Y += q.Y;
            Z += q.Z;
            W += q.W;
            return *this;
        }

        FQuaternion &operator-=(const FQuaternion &q)
        {
            X -= q.X;
            Y -= q.Y;
            Z -= q.Z;
            W -= q.W;
            return *this;
        }

        FQuaternion &operator*=(float scalar)
        {
            X *= scalar;
            Y *= scalar;
            Z *= scalar;
            W *= scalar;
            return *this;
        }

        FQuaternion &operator/=(float scalar)
        {
            if (std::abs(scalar) > 0.0001f)
            {
                X /= scalar;
                Y /= scalar;
                Z /= scalar;
                W /= scalar;
            }
            return *this;
        }

        bool operator==(const FQuaternion &q) const
        {
            return X == q.X && Y == q.Y && Z == q.Z && W == q.W;
        }
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

        FTransform() : Rotation(0.f, 0.f, 0.f, 1.f), Translation(0.f, 0.f, 0.f), Scale3D(1.f, 1.f, 1.f) {}

        FTransform(const FQuaternion &rotation, const FVector &translation, const FVector &scale)
            : Rotation(rotation), Translation(translation), Scale3D(scale) {}

        FTransform(const FVector &translation)
            : Rotation(0.f, 0.f, 0.f, 1.f), Translation(translation), Scale3D(1.f, 1.f, 1.f) {}

        D3DMatrix ToMatrixWithScale() const
        {
            D3DMatrix m;

            float x2 = Rotation.X + Rotation.X;
            float y2 = Rotation.Y + Rotation.Y;
            float z2 = Rotation.Z + Rotation.Z;

            float xx2 = Rotation.X * x2;
            float yy2 = Rotation.Y * y2;
            float zz2 = Rotation.Z * z2;

            float yz2 = Rotation.Y * z2;
            float wx2 = Rotation.W * x2;

            float xy2 = Rotation.X * y2;
            float wz2 = Rotation.W * z2;

            float xz2 = Rotation.X * z2;
            float wy2 = Rotation.W * y2;

            m._11 = (1.0f - (yy2 + zz2)) * Scale3D.X;
            m._12 = (xy2 + wz2) * Scale3D.X;
            m._13 = (xz2 - wy2) * Scale3D.X;
            m._14 = 0.0f;

            m._21 = (xy2 - wz2) * Scale3D.Y;
            m._22 = (1.0f - (xx2 + zz2)) * Scale3D.Y;
            m._23 = (yz2 + wx2) * Scale3D.Y;
            m._24 = 0.0f;

            m._31 = (xz2 + wy2) * Scale3D.Z;
            m._32 = (yz2 - wx2) * Scale3D.Z;
            m._33 = (1.0f - (xx2 + yy2)) * Scale3D.Z;
            m._34 = 0.0f;

            m._41 = Translation.X;
            m._42 = Translation.Y;
            m._43 = Translation.Z;
            m._44 = 1.0f;

            return m;
        }

        FVector TransformPosition(const FVector &v) const
        {
            FVector scaled = v * Scale3D;

            FVector rotated = RotateVector(scaled);

            return rotated + Translation;
        }

        FVector TransformPositionNoScale(const FVector &v) const
        {
            FVector rotated = RotateVector(v);

            return rotated + Translation;
        }

        FVector TransformDirection(const FVector &v) const
        {
            return RotateVector(v);
        }

        FVector InverseTransformPosition(const FVector &v) const
        {
            FVector translated = v - Translation;

            FVector rotated = InverseRotateVector(translated);

            return FVector(
                Scale3D.X != 0.0f ? rotated.X / Scale3D.X : 0.0f,
                Scale3D.Y != 0.0f ? rotated.Y / Scale3D.Y : 0.0f,
                Scale3D.Z != 0.0f ? rotated.Z / Scale3D.Z : 0.0f);
        }

        FVector InverseTransformDirection(const FVector &v) const
        {
            return InverseRotateVector(v);
        }

        FVector RotateVector(const FVector &v) const
        {
            // q * v * q^-1 (Quaternion rotation formula)
            float x2 = Rotation.X + Rotation.X;
            float y2 = Rotation.Y + Rotation.Y;
            float z2 = Rotation.Z + Rotation.Z;

            float xx2 = Rotation.X * x2;
            float yy2 = Rotation.Y * y2;
            float zz2 = Rotation.Z * z2;
            float xy2 = Rotation.X * y2;
            float xz2 = Rotation.X * z2;
            float yz2 = Rotation.Y * z2;
            float wx2 = Rotation.W * x2;
            float wy2 = Rotation.W * y2;
            float wz2 = Rotation.W * z2;

            return FVector(
                v.X * (1.0f - (yy2 + zz2)) + v.Y * (xy2 - wz2) + v.Z * (xz2 + wy2),
                v.X * (xy2 + wz2) + v.Y * (1.0f - (xx2 + zz2)) + v.Z * (yz2 - wx2),
                v.X * (xz2 - wy2) + v.Y * (yz2 + wx2) + v.Z * (1.0f - (xx2 + yy2)));
        }

        FVector InverseRotateVector(const FVector &v) const
        {
            FQuaternion invRotation = Rotation.Conjugate();

            float x2 = invRotation.X + invRotation.X;
            float y2 = invRotation.Y + invRotation.Y;
            float z2 = invRotation.Z + invRotation.Z;

            float xx2 = invRotation.X * x2;
            float yy2 = invRotation.Y * y2;
            float zz2 = invRotation.Z * z2;
            float xy2 = invRotation.X * y2;
            float xz2 = invRotation.X * z2;
            float yz2 = invRotation.Y * z2;
            float wx2 = invRotation.W * x2;
            float wy2 = invRotation.W * y2;
            float wz2 = invRotation.W * z2;

            return FVector(
                v.X * (1.0f - (yy2 + zz2)) + v.Y * (xy2 - wz2) + v.Z * (xz2 + wy2),
                v.X * (xy2 + wz2) + v.Y * (1.0f - (xx2 + zz2)) + v.Z * (yz2 - wx2),
                v.X * (xz2 - wy2) + v.Y * (yz2 + wx2) + v.Z * (1.0f - (xx2 + yy2)));
        }

        FTransform Concatenate(const FTransform &parent) const
        {
            FTransform result;

            result.Rotation = Rotation * parent.Rotation;
            result.Rotation.Normalize();

            result.Scale3D = Scale3D * parent.Scale3D;

            FVector scaledTranslation = Translation * parent.Scale3D;
            result.Translation = parent.TransformPosition(scaledTranslation);

            return result;
        }

        FTransform Inverse() const
        {
            FTransform result;

            result.Rotation = Rotation.Conjugate();

            result.Scale3D = FVector(
                Scale3D.X != 0.0f ? 1.0f / Scale3D.X : 0.0f,
                Scale3D.Y != 0.0f ? 1.0f / Scale3D.Y : 0.0f,
                Scale3D.Z != 0.0f ? 1.0f / Scale3D.Z : 0.0f);

            FVector invTranslation = Translation * -1.0f;
            result.Translation = result.InverseRotateVector(invTranslation * result.Scale3D);

            return result;
        }

        FVector ForwardVector() const
        {
            return FVector(
                2.0f * (Rotation.X * Rotation.Z + Rotation.W * Rotation.Y),
                2.0f * (Rotation.Y * Rotation.Z - Rotation.W * Rotation.X),
                1.0f - 2.0f * (Rotation.X * Rotation.X + Rotation.Y * Rotation.Y));
        }

        FVector RightVector() const
        {
            return FVector(
                1.0f - 2.0f * (Rotation.Y * Rotation.Y + Rotation.Z * Rotation.Z),
                2.0f * (Rotation.X * Rotation.Y + Rotation.W * Rotation.Z),
                2.0f * (Rotation.X * Rotation.Z - Rotation.W * Rotation.Y));
        }

        FVector UpVector() const
        {
            return FVector(
                2.0f * (Rotation.X * Rotation.Y - Rotation.W * Rotation.Z),
                1.0f - 2.0f * (Rotation.X * Rotation.X + Rotation.Z * Rotation.Z),
                2.0f * (Rotation.Y * Rotation.Z + Rotation.W * Rotation.X));
        }

        bool IsValid() const
        {
            return Translation.IsValid() &&
                   Rotation.X == Rotation.X && Rotation.Y == Rotation.Y &&
                   Rotation.Z == Rotation.Z && Rotation.W == Rotation.W && // NaN check
                   Scale3D.IsValid() &&
                   std::abs(Scale3D.X) > 0.0001f &&
                   std::abs(Scale3D.Y) > 0.0001f &&
                   std::abs(Scale3D.Z) > 0.0001f;
        }

        void SetIdentity()
        {
            Rotation = FQuaternion(0.f, 0.f, 0.f, 1.f);
            Translation = FVector(0.f, 0.f, 0.f);
            Scale3D = FVector(1.f, 1.f, 1.f);
        }

        bool operator==(const FTransform &other) const
        {
            return (Translation == other.Translation) &&
                   (Rotation == other.Rotation) &&
                   (Scale3D == other.Scale3D);
        }

        bool operator!=(const FTransform &other) const
        {
            return !(*this == other);
        }

        friend FTransform operator*(const FTransform &a, const FTransform &b)
        {
            return a.Concatenate(b);
        }
    };

    struct MinimalViewInfo
    {
        FVector Location;
        // FVector LocationLocalSpace; // This field only in PUBG struct
        FRotator Rotation;
        float FOV;
    };

    struct CameraCacheEntry
    {
        float TimeStamp;
        char chunks[0xC];
        MinimalViewInfo POV;
    };

    struct TArray
    {
        uintptr_t data;
        int count;
        int max;
    };

    template <typename T>
    struct TArrayRaw
    {
        uintptr_t data;
        int count;
        int max;
    };

    struct FBoxSphereBounds
    {
        struct FVector Origin;
        struct FVector BoxExtent;
        float SphereRadius;
    };

    struct UCapsuleComponent
    {
        float CapsuleHalfHeight;
        float CapsuleRadius;
    };

} // namespace Structs

#endif // STRUCTS_H