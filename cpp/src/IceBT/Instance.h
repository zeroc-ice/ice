//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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
        virtual ~Instance();

        EnginePtr engine() const { return _engine; }

        bool initialized() const;

    private:
        const EnginePtr _engine;
    };
}

#endif
