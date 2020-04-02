//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef TEST_I_H
#define TEST_I_H

#include <Test.h>

class ServerI : public Test::Server
{
public:

    ServerI(const Ice::CommunicatorPtr&);

    virtual void noCert(const Ice::Current&);
    virtual void checkCert(std::string, std::string, const Ice::Current&);
    virtual void checkCipher(std::string, const Ice::Current&);

    void destroy();

private:

    Ice::CommunicatorPtr _communicator;
};
ICE_DEFINE_PTR(ServerIPtr, ServerI);

class ServerFactoryI : public Test::ServerFactory
{
public:

    ServerFactoryI(const std::string&);

    virtual Test::ServerPrxPtr createServer(Test::Properties, const Ice::Current&);
    virtual void destroyServer(Test::ServerPrxPtr, const Ice::Current&);
    virtual void shutdown(const Ice::Current&);

private:

    std::string _defaultDir;
    std::map<Ice::Identity, ServerIPtr> _servers;
};

#endif
