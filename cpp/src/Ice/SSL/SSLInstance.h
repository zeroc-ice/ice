// Copyright (c) ZeroC, Inc.

#ifndef ICE_SSL_INSTANCE_H
#define ICE_SSL_INSTANCE_H

#include "../ProtocolInstance.h"
#include "SSLEngineF.h"
#include "SSLInstanceF.h"

namespace Ice::SSL
{
    class Instance final : public IceInternal::ProtocolInstance
    {
    public:
        Instance(const SSLEnginePtr&, std::int16_t, const std::string&);
        ~Instance() final;

        [[nodiscard]] SSLEnginePtr engine() const;

    private:
        // Use a weak pointer to avoid circular references, the SSL engine is owned by the communicator instance.
        const std::weak_ptr<SSLEngine> _engine;
    };
}

#endif
