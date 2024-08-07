#pragma once

#include "ndq/platform.h"
#include "ndq/rhi_format.h"

namespace ndq
{
    struct NDQ_INPUT_ELEMENT_DESC
    {
        const char* SemanticName;
        uint32 SemanticIndex;
        NDQ_RESOURCE_FORMAT Format;
        int32 InputSlot;
    };

    class IInputLayout
    {
    public:
        virtual NDQ_INPUT_ELEMENT_DESC GetDesc(uint32 index) const = 0;
    };
}