//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICESSL_INSTANCE_H
#define ICESSL_INSTANCE_H

#include "../Ice/ProtocolInstance.h"
#include "SSLEngineF.h"
#include "SSLInstanceF.h"

namespace IceSSL
{
    class ICE_API Instance final : public IceInternal::ProtocolInstance
    {
    public:
        Instance(const SSLEnginePtr&, std::int16_t, const std::string&);

        SSLEnginePtr engine() const { return _engine; }

        bool initialized() const;

    private:
        const SSLEnginePtr _engine;
    };
}

#endif
