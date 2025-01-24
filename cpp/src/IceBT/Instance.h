// Copyright (c) ZeroC, Inc.

#ifndef ICE_BT_INSTANCE_H
#define ICE_BT_INSTANCE_H

#include "../Ice/ProtocolInstance.h"
#include "EngineF.h"
#include "InstanceF.h"

namespace IceBT
{
    class Instance : public IceInternal::ProtocolInstance
    {
    public:
        Instance(const EnginePtr&, std::int16_t, const std::string&);
        ~Instance() override;

        [[nodiscard]] EnginePtr engine() const;

        [[nodiscard]] bool initialized() const;

    private:
        // Use a weak pointer to avoid circular references, the engine is owned by the plug-in instance.
        const std::weak_ptr<Engine> _engine;
    };
}

#endif
