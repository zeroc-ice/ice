//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_BT_INSTANCE_H
#define ICE_BT_INSTANCE_H

#include <IceBT/InstanceF.h>
#include <IceBT/EngineF.h>
#include <Ice/ProtocolInstance.h>

namespace IceBT
{

class Instance : public IceInternal::ProtocolInstance
{
public:

    Instance(const EnginePtr&, Ice::Short, const std::string&);
    virtual ~Instance();

    EnginePtr engine() const
    {
        return _engine;
    }

    bool initialized() const;

private:

    const EnginePtr _engine;
};

}

#endif
