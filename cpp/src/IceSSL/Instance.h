//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICESSL_INSTANCE_H
#define ICESSL_INSTANCE_H

#include <Ice/ProtocolInstance.h>
#include <IceSSL/InstanceF.h>
#include <IceSSL/SSLEngineF.h>

namespace IceSSL
{

class ICESSL_API Instance : public IceInternal::ProtocolInstance
{
public:

    Instance(const SSLEnginePtr&, Ice::Short, const std::string&);
    virtual ~Instance();

    SSLEnginePtr
    engine() const
    {
        return _engine;
    }

    bool initialized() const;

private:

    const SSLEnginePtr _engine;
};

}

#endif
