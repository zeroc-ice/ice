// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_SSL_INSTANCE_H
#define ICE_SSL_INSTANCE_H

#include <IceSSL/InstanceF.h>
#include <IceSSL/UtilF.h>
#include <Ice/ProtocolInstance.h>
#include <Ice/ProtocolPluginFacadeF.h>
#include <IceSSL/Plugin.h>
#include <IceSSL/SSLEngine.h>
#include <IceSSL/TrustManagerF.h>
#include <Ice/BuiltinSequences.h>

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

    int securityTraceLevel() const;
    std::string securityTraceCategory() const;

private:

    const SSLEnginePtr _engine;
    int _securityTraceLevel;
    std::string _securityTraceCategory;
};

}

#endif
