#pragma once

namespace ndq
{
    class CommandList
    {
    public:
        virtual ~CommandList() = default;

        enum CommandListTypes
        {
            CLT_GRAPHICS,
            CLT_COPY,
            CLT_COMPUTE,
        };

        virtual void* getRawCommandList() const = 0;
        virtual CommandListTypes getType() const = 0;
        virtual void open() = 0;
        virtual void close() = 0;
    };
}
