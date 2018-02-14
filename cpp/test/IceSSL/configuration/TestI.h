// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef TEST_I_H
#define TEST_I_H

#include <Test.h>

class ServerI : public Test::Server
{
public:

    ServerI(const Ice::CommunicatorPtr&);

    virtual void noCert(const Ice::Current&);
    virtual void checkCert(const std::string&, const std::string&, const Ice::Current&);
    virtual void checkCipher(const std::string&, const Ice::Current&);

    void destroy();

private:

    Ice::CommunicatorPtr _communicator;
};
typedef IceUtil::Handle<ServerI> ServerIPtr;

class ServerFactoryI : public Test::ServerFactory
{
public:

    virtual Test::ServerPrx createServer(const Test::Properties&, const Ice::Current&);
    virtual void destroyServer(const Test::ServerPrx&, const Ice::Current&);
    virtual void shutdown(const Ice::Current&);

private:

    std::map<Ice::Identity, ServerIPtr> _servers;
};

#endif
