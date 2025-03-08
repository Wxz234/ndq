#pragma once

#include "ndq/Defs.h"

namespace ndq
{
    class SceneManager;

    class Root
    {
    public:
        ~Root() = default;
        SceneManager* createSceneManager();
        void destroySceneManager(SceneManager* sceneManager);

        static Root* getRoot();

        NDQ_DISABLE_COPY_AND_MOVE(Root)
    private:
        Root() {}
    };
}