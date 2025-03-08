#include "ndq/Root.h"
#include "ndq/SceneManager.h"
#include "ndq/Type.h"

#include "DefaultSceneManager.h"

namespace ndq
{
    SceneManager* Root::createSceneManager()
    {
        return new DefaultSceneManager("");
    }

    void Root::destroySceneManager(SceneManager* sceneManager)
    {
        delete sceneManager;
    }

    Root* Root::getRoot()
    {
        static Root instance;
        return &instance;
    }
}