//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_SHARED_CONTEXT_H
#define ICE_SHARED_CONTEXT_H

#include <string>
#include <map>
#include <memory>

namespace Ice
{
    typedef ::std::map<::std::string, ::std::string> Context;
}

namespace IceInternal
{
    class SharedContext
    {
    public:
        SharedContext() {}

        SharedContext(const Ice::Context& val) : _val(val) {}

        inline const Ice::Context& getValue() { return _val; }

    private:
        Ice::Context _val;
    };

    using SharedContextPtr = std::shared_ptr<SharedContext>;
}

#endif
