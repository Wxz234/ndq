#pragma once

namespace ndq
{
    struct NDQ_INPUT_ELEMENT_DESC
    {
        const char* SemanticName;
        NDQ_RESOURCE_FORMAT Format;
        int32 InputSlot;
    };

    class IInputLayout
    {

    };
}