//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_SSL_INSTANCE_H
#define ICE_SSL_INSTANCE_H

#include "../ProtocolInstance.h"
#include "SSLEngineF.h"
#include "SSLInstanceF.h"

namespace Ice::SSL
{
    class ICE_API Instance final : public IceInternal::ProtocolInstance
    {
    public:
        Instance(const SSLEnginePtr&, std::int16_t, const std::string&);

        SSLEnginePtr engine() const { return _engine; }

    private:
        const SSLEnginePtr _engine;
    };
}

#endif
