#pragma once

#include "ndq/Defs.h"
#include "ndq/Type.h"

namespace ndq
{
    class SceneNode;

    class SceneManager
    {
    public:
        virtual ~SceneManager() = default;
        SceneNode* getRootSceneNode();
        virtual String getTypeName() const = 0;

        NDQ_DISABLE_COPY_AND_MOVE(SceneManager)

        SceneManager(const String& instanceName);
    };
}