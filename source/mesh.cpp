module;

#include "predef.h"

export module ndq:mesh;

import :platform;

namespace ndq
{
    class Mesh
    {
    public:
    private:
        std::vector<DirectX::XMFLOAT3> Positions;
        std::vector<DirectX::XMFLOAT3> Normals;
        std::vector<DirectX::XMFLOAT2> UV0;
        std::vector<DirectX::XMFLOAT4> Tangents;
        std::vector<uint32> Indices;
    };
}
