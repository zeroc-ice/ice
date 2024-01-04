//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <Glacier2/PermissionsVerifier.h>
#include <Glacier2/Session.h>
#include <TestHelper.h>
#include <IceSSL/Plugin.h>

using namespace std;
using namespace Test;

namespace
{

void testContext(bool ssl, const shared_ptr<Ice::Communicator>& communicator, const Ice::Context& context)
{
    Ice::Context ctx = context;
    if(!ssl)
    {
        test(ctx["_con.type"] == "tcp");
        ostringstream port;
        port << TestHelper::getTestPort(communicator->getProperties());
        test(ctx["_con.localPort"] == port.str());
    }
    else
    {
        test(ctx["_con.type"] == "ssl");
        ostringstream port;
        port << TestHelper::getTestPort(communicator->getProperties(), 1);
        test(ctx["_con.localPort"] == port.str());
    }
    test(ctx["_con.localAddress"] == "127.0.0.1");
    test(ctx["_con.remotePort"] != "");
    test(ctx["_con.remoteAddress"] == "127.0.0.1");
}

}

class PermissionsVerifierI final : public Glacier2::PermissionsVerifier
{
public:

    bool
    checkPermissions(string userId, string, string&, const Ice::Current& current) const override
    {
        testContext(userId == "ssl", current.adapter->getCommunicator(), current.ctx);
        return true;
    }
};

class SSLPermissionsVerifierI final : public Glacier2::SSLPermissionsVerifier
{
public:

    bool
    authorize(Glacier2::SSLInfo info, string&, const Ice::Current& current) const override
    {
        testContext(true, current.adapter->getCommunicator(), current.ctx);

        auto cert = IceSSL::Certificate::decode(info.certs[0]);
        test(cert->getIssuerDN() == IceSSL::DistinguishedName(
             "emailAddress=info@zeroc.com,C=US,ST=Florida,L=Jupiter,O=ZeroC\\, Inc.,OU=Ice,CN=Ice Tests CA"));
        test(cert->getSubjectDN() == IceSSL::DistinguishedName(
             "emailAddress=info@zeroc.com,C=US,ST=Florida,L=Jupiter,O=ZeroC\\, Inc.,OU=Ice,CN=client"));
        test(cert->checkValidity());

        return true;
    }
};

class SessionI final : public Glacier2::Session
{
public:

    SessionI(bool shutdown, bool ssl) : _shutdown(shutdown), _ssl(ssl)
    {
    }

    void
    destroy(const Ice::Current& current) override
    {
        testContext(_ssl, current.adapter->getCommunicator(), current.ctx);

        current.adapter->remove(current.id);
        if(_shutdown)
        {
            current.adapter->getCommunicator()->shutdown();
        }
    }

    void
    ice_ping(const Ice::Current& current) const override
    {
        testContext(_ssl, current.adapter->getCommunicator(), current.ctx);
    }

private:

    const bool _shutdown;
    const bool _ssl;
};

class SessionManagerI final : public Glacier2::SessionManager
{
public:

    shared_ptr<Glacier2::SessionPrx>
    create(string userId, shared_ptr<Glacier2::SessionControlPrx>, const Ice::Current& current) override
    {
        testContext(userId == "ssl", current.adapter->getCommunicator(), current.ctx);

        auto session = make_shared<SessionI>(false, userId == "ssl");
        return Ice::uncheckedCast<Glacier2::SessionPrx>(current.adapter->addWithUUID(session));
    }
};

class SSLSessionManagerI final : public Glacier2::SSLSessionManager
{
public:

    shared_ptr<Glacier2::SessionPrx>
    create(Glacier2::SSLInfo info, shared_ptr<Glacier2::SessionControlPrx>, const Ice::Current& current) override
    {
        testContext(true, current.adapter->getCommunicator(), current.ctx);

        test(info.remoteHost == "127.0.0.1");
        test(info.localHost == "127.0.0.1");
        test(info.localPort == TestHelper::getTestPort(current.adapter->getCommunicator()->getProperties(), 1));

        try
        {
            auto cert = IceSSL::Certificate::decode(info.certs[0]);
            test(cert->getIssuerDN() == IceSSL::DistinguishedName(
                     "emailAddress=info@zeroc.com,C=US,ST=Florida,L=Jupiter,O=ZeroC\\, Inc.,OU=Ice,CN=Ice Tests CA"));
            test(cert->getSubjectDN() == IceSSL::DistinguishedName(
                     "emailAddress=info@zeroc.com,C=US,ST=Florida,L=Jupiter,O=ZeroC\\, Inc.,OU=Ice,CN=client"));
            test(cert->checkValidity());
        }
        catch(const IceSSL::CertificateReadException&)
        {
            test(false);
        }

        auto session = make_shared<SessionI>(true, true);
        return Ice::uncheckedCast<Glacier2::SessionPrx>(current.adapter->addWithUUID(session));
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
    Ice::CommunicatorHolder communicator = initialize(argc, argv);
    auto adapter = communicator->createObjectAdapterWithEndpoints("SessionServer",
                                                                                   getTestEndpoint(3, "tcp"));
    adapter->add(make_shared<PermissionsVerifierI>(), Ice::stringToIdentity("verifier"));
    adapter->add(make_shared<SSLPermissionsVerifierI>(), Ice::stringToIdentity("sslverifier"));
    adapter->add(make_shared<SessionManagerI>(), Ice::stringToIdentity("sessionmanager"));
    adapter->add(make_shared<SSLSessionManagerI>(), Ice::stringToIdentity("sslsessionmanager"));
    adapter->activate();
    communicator->waitForShutdown();
}

DEFINE_TEST(Server)
