// Copyright (c) ZeroC, Inc.

#ifndef ICE_SHARED_CONTEXT_H
#define ICE_SHARED_CONTEXT_H

#include <map>
#include <memory>
#include <string>

namespace Ice
{
    using Context = std::map<std::string, std::string, std::less<>>;
}

namespace IceInternal
{
    class SharedContext
    {
    public:
        SharedContext() = default;

        SharedContext(Ice::Context val) : _val(std::move(val)) {}

        inline const Ice::Context& getValue() { return _val; }

    private:
        Ice::Context _val;
    };

    using SharedContextPtr = std::shared_ptr<SharedContext>;
}

#endif
