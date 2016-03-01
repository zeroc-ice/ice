// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
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

    bool verify(const NativeConnectionInfoPtr&);

private:

    bool match(const std::list< DistinguishedName> &, const DistinguishedName&) const;
    void parse(const std::string&, std::list<DistinguishedName>&, std::list<DistinguishedName>&) const;

    const Ice::CommunicatorPtr _communicator;
    int _traceLevel;

    std::list<DistinguishedName> _rejectAll;
    std::list<DistinguishedName> _rejectClient;
    std::list<DistinguishedName> _rejectAllServer;
    std::map<std::string, std::list<DistinguishedName> > _rejectServer;

    std::list<DistinguishedName> _acceptAll;
    std::list<DistinguishedName> _acceptClient;
    std::list<DistinguishedName> _acceptAllServer;
    std::map<std::string, std::list<DistinguishedName> > _acceptServer;
};

}

#endif
