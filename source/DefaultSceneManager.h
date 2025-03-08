#pragma once

#include "ndq/SceneManager.h"
#include "ndq/Type.h"

namespace ndq
{
    class DefaultSceneManager : public SceneManager
    {
    public:
        DefaultSceneManager(const String& name) : SceneManager(name) {}

        String getTypeName() const
        {
            return String("DefaultSceneManager");
        }
    };
}