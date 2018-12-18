// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
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
    virtual void checkCert(ICE_IN(std::string), ICE_IN(std::string), const Ice::Current&);
    virtual void checkCipher(ICE_IN(std::string), const Ice::Current&);

    void destroy();

private:

    Ice::CommunicatorPtr _communicator;
};
ICE_DEFINE_PTR(ServerIPtr, ServerI);

class ServerFactoryI : public Test::ServerFactory
{
public:

    ServerFactoryI(const std::string&);

    virtual Test::ServerPrxPtr createServer(ICE_IN(Test::Properties), const Ice::Current&);
    virtual void destroyServer(ICE_IN(Test::ServerPrxPtr), const Ice::Current&);
    virtual void shutdown(const Ice::Current&);

private:

    std::string _defaultDir;
    std::map<Ice::Identity, ServerIPtr> _servers;
};

#endif
