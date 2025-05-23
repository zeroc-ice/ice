// Copyright (c) ZeroC, Inc.

#include "../../src/Ice/SSL/SSLUtil.h"
#include "Glacier2/PermissionsVerifier.h"
#include "Ice/Ice.h"
#include "TestHelper.h"

using namespace std;

class ClientPermissionsVerifierI final : public Glacier2::PermissionsVerifier
{
public:
    bool checkPermissions(string userId, string passwd, string&, const Ice::Current& current) const override
    {
        if (current.ctx.find("throw") != current.ctx.end())
        {
            throw Glacier2::PermissionDeniedException("reason");
        }
        return (userId == "client1" && passwd == "test1") || (userId == "client2" && passwd == "test2");
    }
};

class SSLPermissionsVerifierI final : public Glacier2::SSLPermissionsVerifier
{
public:
    bool authorize(Glacier2::SSLInfo info, string&, const Ice::Current& current) const override
    {
        if (current.ctx.find("throw") != current.ctx.end())
        {
            throw Glacier2::PermissionDeniedException("reason");
        }
        test(info.certs.size() > 0);
        Ice::SSL::ScopedCertificate cert = Ice::SSL::decodeCertificate(info.certs[0]);
        string subjectName = Ice::SSL::getSubjectName(cert.get());
        test(subjectName.find("CN=ca.client") != string::npos);
        test(subjectName.find("OU=Ice test infrastructure") != string::npos);
        test(subjectName.find("O=ZeroC") != string::npos);
        test(subjectName.find("L=Jupiter") != string::npos);
        test(subjectName.find("ST=Florida") != string::npos);
        test(subjectName.find("C=US") != string::npos);
        test(subjectName.find("emailAddress=info@zeroc.com") != string::npos);
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
    if (communicatorHolder->getProperties()->getPropertyAsInt("AddPermissionsVerifiers") > 0)
    {
        adapter->add(make_shared<ClientPermissionsVerifierI>(), Ice::stringToIdentity("ClientPermissionsVerifier"));
        adapter->add(make_shared<SSLPermissionsVerifierI>(), Ice::stringToIdentity("SSLPermissionsVerifier"));
    }
    adapter->activate();

    try
    {
        adapter->activate();
    }
    catch (const Ice::ObjectAdapterDeactivatedException&)
    {
    }
    communicatorHolder->waitForShutdown();
}

DEFINE_TEST(Server)
