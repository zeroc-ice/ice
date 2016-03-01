// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_SSL_INSTANCE_H
#define ICE_SSL_INSTANCE_H

#include <IceSSL/InstanceF.h>
#include <Ice/ProtocolInstance.h>
#include <IceSSL/SSLEngineF.h>

namespace IceSSL
{

class Instance : public IceInternal::ProtocolInstance
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
