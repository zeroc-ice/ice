// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_SSL_TRUST_MANAGER_H
#define ICE_SSL_TRUST_MANAGER_H

#include <Ice/CommunicatorF.h>
#include <IceSSL/TrustManagerF.h>
#include <IceSSL/Plugin.h>
#include <IceSSL/RFC2253.h>
#include <list>

namespace IceSSL
{

class TrustManager : public IceUtil::Shared
{
public:

    TrustManager(const Ice::CommunicatorPtr&);

    bool verify(const ConnectionInfo&);

private:

    bool match(const std::list< DistinguishedName> &, const DistinguishedName&) const;
    std::list<DistinguishedName> parse(const std::string&) const;

    const Ice::CommunicatorPtr _communicator;
    int _traceLevel;

    std::list<DistinguishedName> _all;
    std::list<DistinguishedName> _client;
    std::list<DistinguishedName> _allServer;
    std::map<std::string, std::list<DistinguishedName> > _server;
};

}

#endif
