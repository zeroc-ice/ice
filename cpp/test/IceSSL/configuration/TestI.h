// Copyright (c) ZeroC, Inc.

#ifndef TEST_I_H
#define TEST_I_H

#include "Test.h"

class ServerI : public Test::Server
{
public:
    ServerI(Ice::CommunicatorPtr);

    void noCert(const Ice::Current&) override;
    void checkCert(std::string, std::string, const Ice::Current&) override;

    void destroy();

private:
    Ice::CommunicatorPtr _communicator;
};
using ServerIPtr = std::shared_ptr<ServerI>;

class ServerFactoryI : public Test::ServerFactory
{
public:
    ServerFactoryI(std::string);

    std::optional<Test::ServerPrx> createServer(Test::Properties, const Ice::Current&) override;
    void destroyServer(std::optional<Test::ServerPrx>, const Ice::Current&) override;
    void shutdown(const Ice::Current&) override;

private:
    std::string _defaultDir;
    std::map<Ice::Identity, ServerIPtr> _servers;
};

#endif
