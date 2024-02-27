module;

#include "predef.h"

export module ndq:system;

export namespace ndq
{
    class ISystem
    {
    public:
        virtual void Update(float t) = 0;
    };
}