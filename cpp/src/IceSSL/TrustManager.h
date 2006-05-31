// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
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

    bool match(const RFC2253::RDNSeqSeq&, const RFC2253::RDNSeq&) const;

    const Ice::CommunicatorPtr _communicator;
    int _traceLevel;

    RFC2253::RDNSeqSeq _all;
    RFC2253::RDNSeqSeq _client;
    RFC2253::RDNSeqSeq _allServer;
    std::map<std::string, RFC2253::RDNSeqSeq > _server;
};

}

#endif
