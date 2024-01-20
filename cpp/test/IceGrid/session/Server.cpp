//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <Glacier2/PermissionsVerifier.h>
#include <IceSSL/Plugin.h>
#include <TestHelper.h>
#include <Test.h>

using namespace std;

class ClientPermissionsVerifierI final : public Glacier2::PermissionsVerifier
{
public:

    bool
    checkPermissions(string userId, string passwd, string&, const Ice::Current& current) const override
    {
        if(current.ctx.find("throw") != current.ctx.end())
        {
            throw Test::ExtendedPermissionDeniedException("reason");
        }
        return (userId == "client1" && passwd == "test1") || (userId == "client2" && passwd == "test2");
    }
};

class SSLPermissionsVerifierI final : public Glacier2::SSLPermissionsVerifier
{
public:

    bool
    authorize(Glacier2::SSLInfo info, string&, const Ice::Current& current) const override
    {
        if(current.ctx.find("throw") != current.ctx.end())
        {
            throw Test::ExtendedPermissionDeniedException("reason");
        }
        test(info.certs.size() > 0);
        auto cert = IceSSL::Certificate::decode(info.certs[0]);
        test(cert->getIssuerDN() == IceSSL::DistinguishedName(
             "emailAddress=info@zeroc.com,C=US,ST=Florida,L=Jupiter,O=ZeroC\\, Inc.,OU=Ice,CN=Ice Tests CA"));
        test(cert->getSubjectDN() == IceSSL::DistinguishedName(
             "emailAddress=info@zeroc.com,C=US,ST=Florida,L=Jupiter,O=ZeroC\\, Inc.,OU=Ice,CN=client"));
        test(cert->checkValidity());

        return true;
    }
};

class Server final : public Test::TestHelper
{
public:

    void run(int, char**) override;
};

void
Server::run(int argc, char** argv)
{
    Ice::CommunicatorHolder communicatorHolder = initialize(argc, argv);
    auto adapter = communicatorHolder->createObjectAdapter("Server");
    if(communicatorHolder->getProperties()->getPropertyAsInt("AddPermissionsVerifiers") > 0)
    {
        adapter->add(make_shared<ClientPermissionsVerifierI>(), Ice::stringToIdentity("ClientPermissionsVerifier"));
        adapter->add(make_shared<SSLPermissionsVerifierI>(), Ice::stringToIdentity("SSLPermissionsVerifier"));
    }
    adapter->activate();

    try
    {
        adapter->activate();
    }
    catch(const Ice::ObjectAdapterDeactivatedException&)
    {
    }
    communicatorHolder->waitForShutdown();
}

DEFINE_TEST(Server)
