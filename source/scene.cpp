module;

#include "predef.h"

export module ndq:scene;

import :platform;
import :gltf;
import :camera;
import :render_data;
import :asset;

export namespace ndq
{
    enum class SCENE_TYPE
    {
        DEFAULT,
    };

    class IScene
    {
    public:
        virtual void SetWidth(uint32 w) = 0;
        virtual void SetHeight(uint32 h) = 0;
        virtual void Update(float t) = 0;
        virtual const RenderData* GetRenderData() const = 0;
    };
}

namespace ndq
{
    class Scene : public IScene
    {
    public:
        virtual ~Scene() {}
    };

    class Scene_Default : public Scene
    {
    public:
        Scene_Default()
        {
            Data.Width = &Width;
            Data.Height = &Height;
            Data.MainCamera = &MainCamera;
            Data.DefaultSkyLight = &DefaultSkyLight;

            MainCamera.SetView(DirectX::XMFLOAT3(.0f, .0f, .0f), DirectX::XMFLOAT3(.0f, .0f, 1.f), DirectX::XMFLOAT3(.0f, 1.f, .0f));
            MainCamera.SetProjection(Width, Height, .1f, 100.f);
        }

        void SetWidth(uint32 w)
        {
            Width = w;
            MainCamera.SetProjection(Width, Height, .1f, 100.f);
        }

        void SetHeight(uint32 h)
        {
            Height = h;
            MainCamera.SetProjection(Width, Height, .1f, 100.f);
        }

        void Update(float t)
        {

        }

        void LoadStaticModel(const char* path)
        {

        }

        const RenderData* GetRenderData() const
        {
            return &Data;
        }

        RenderData Data;

        DirectX::XMFLOAT4 DefaultSkyLight;
        Camera MainCamera;

        //std::vector<GLTF> StaticModel;

        uint32 Width = NDQ_DEFAULT_WIDTH;
        uint32 Height = NDQ_DEFAULT_HEIGHT;
    };
}

export namespace ndq
{
    IScene* CreateScene(SCENE_TYPE type)
    {
        IScene* temp = nullptr;
        if (type == SCENE_TYPE::DEFAULT)
        {
            temp = new Scene_Default;
        }

        return temp;
    }

    void RemoveScene(IScene* pScene)
    {
        auto TempScenePtr = dynamic_cast<Scene*>(pScene);
        delete TempScenePtr;
    }
}



