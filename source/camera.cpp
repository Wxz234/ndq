module;

#include "predef.h"

export module ndq:camera;

namespace ndq
{
    class Camera
    {
    public:
        Camera() {}
        Camera
        (
            const DirectX::XMFLOAT3& position,
            const DirectX::XMFLOAT3& eye,
            const DirectX::XMFLOAT3& up,
            float viewWidth,
            float viewHeight,
            float nearZ,
            float farZ
        )
            : Position(position),
            Eye(eye),
            Up(up),
            ViewWidth(viewWidth),
            ViewHeight(viewHeight),
            NearZ(nearZ),
            FarZ(farZ) {}

        Camera(const Camera&) = default;
        Camera& operator=(const Camera&) = default;

        Camera(Camera&&) noexcept = default;
        Camera& operator=(Camera&&) noexcept = default;
    private:
        DirectX::XMFLOAT3 Position = DirectX::XMFLOAT3(0.f, 0.f, 0.f);
        DirectX::XMFLOAT3 Eye = DirectX::XMFLOAT3(0.f, 0.f, 1.f);
        DirectX::XMFLOAT3 Up = DirectX::XMFLOAT3(0.f, 1.f, 0.f);

        float ViewWidth = 800.f;
        float ViewHeight = 600.f;
        float NearZ = 1.f;
        float FarZ = 100.f;
    };
}