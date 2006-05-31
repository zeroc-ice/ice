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
#include <list>

namespace IceSSL
{

class TrustManager : public IceUtil::Shared
{
public:

    TrustManager(const Ice::CommunicatorPtr&);

    bool verify(const ConnectionInfo&);

private:

    bool match(const std::list< std::list< std::pair<std::string, std::string> > >&,
	       const std::list< std::pair<std::string, std::string> >&) const;

    const Ice::CommunicatorPtr _communicator;
    int _traceLevel;

    std::list<std::list< std::pair<std::string, std::string> > > _all;
    std::list<std::list< std::pair<std::string, std::string> > > _client;
    std::list<std::list< std::pair<std::string, std::string> > > _allServer;
    std::map<std::string, std::list< std::list< std::pair<std::string, std::string> > > > _server;
};

}

#endif
