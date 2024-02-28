module;

#include "predef.h"

export module ndq:scene;

import :gltf;
import :smart_ptr;

export namespace ndq
{
    enum class SCENE_TYPE
    {
        DEFAULT,
    };

    class IScene
    {
    public:
        virtual void Update(float t) = 0;
        virtual void LoadStaticModel(const char* path) = 0;
    };
}

namespace ndq
{
    class Scene : public IScene
    {
    public:
        void Update(float t)
        {

        }

        void LoadStaticModel(const char* path)
        {
            GLTF gltf{};
            bool success = LoadGLTF(path, gltf);
            if (success)
            {
                StaticModel.push_back(gltf);
            }
        }
        DirectX::XMFLOAT4 DefaultSkyLight;

        std::vector<GLTF> StaticModel;
    };
}

export namespace ndq
{

    shared_ptr<IScene> CreateScene(SCENE_TYPE type)
    {
        shared_ptr<IScene> temp;
        if (type == SCENE_TYPE::DEFAULT)
        {
            temp = shared_ptr<IScene>(new Scene);
        }

        return temp;
    }
}



