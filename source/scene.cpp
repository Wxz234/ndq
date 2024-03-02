module;

#include "predef.h"

export module ndq:scene;

import :gltf;
import :smart_ptr;
import :camera;
import :render_data;

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
        virtual const RenderData* GetRenderData() const = 0;
    };
}

namespace ndq
{
    class Scene : public IScene
    {
    public:
        Scene()
        {
            Data.MainCamera = &MainCamera;
            Data.StaticModel = &StaticModel;
            Data.DefaultSkyLight = &DefaultSkyLight;
        }

        void Update(float t)
        {

        }

        void LoadStaticModel(const char* path)
        {
            if (GLTF gltf{}; LoadGLTFStaticModel(path, gltf))
            {
                StaticModel.push_back(gltf);
            }
        }

        const RenderData* GetRenderData() const
        {
            return &Data;
        }

        RenderData Data;

        DirectX::XMFLOAT4 DefaultSkyLight;
        Camera MainCamera;

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



