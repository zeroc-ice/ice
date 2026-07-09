// Copyright (c) ZeroC, Inc.

#include "../../src/Ice/SSL/SSLUtil.h"
#include "Glacier2/PermissionsVerifier.h"
#include "Glacier2/Session.h"
#include "Ice/Ice.h"
#include "TestHelper.h"

#include <iostream>

using namespace std;
using namespace Test;

namespace
{
    int testPortIndex(const string& type)
    {
        if (type == "tcp")
        {
            return 0;
        }
        else if (type == "ssl")
        {
            return 1;
        }
        else
        {
            test(type == "wss");
            return 4;
        }
    }

    void testContext(const string& type, const shared_ptr<Ice::Communicator>& communicator, const Ice::Context& context)
    {
        Ice::Context ctx = context;
        test(ctx["_con.type"] == type);
        ostringstream port;
        port << TestHelper::getTestPort(communicator->getProperties(), testPortIndex(type));
        test(ctx["_con.localPort"] == port.str());
        test(ctx["_con.localAddress"] == "127.0.0.1");
        test(ctx["_con.remotePort"] != "");
        test(ctx["_con.remoteAddress"] == "127.0.0.1");
        if (type == "tcp")
        {
            test(ctx.find("_con.peerCert") == ctx.end());
        }
        else
        {
            test(!ctx["_con.peerCert"].empty());
        }
    }

    // The type of the secure connection ("ssl" or "wss") over which the client called the router.
    string secureConnectionType(const Ice::Context& context)
    {
        auto p = context.find("_con.type");
        test(p != context.end());
        test(p->second == "ssl" || p->second == "wss");
        return p->second;
    }

    void testSubjectName(const string& subjectName)
    {
        test(subjectName.find("CN=ca.client") != string::npos);
        test(subjectName.find("OU=Ice test infrastructure") != string::npos);
        test(subjectName.find("O=ZeroC") != string::npos);
        test(subjectName.find("L=Jupiter") != string::npos);
        test(subjectName.find("ST=Florida") != string::npos);
        test(subjectName.find("C=US") != string::npos);
        test(subjectName.find("emailAddress=info@zeroc.com") != string::npos);
    }
}

class PermissionsVerifierI final : public Glacier2::PermissionsVerifier
{
public:
    bool checkPermissions(string userId, string, string&, const Ice::Current& current) const override
    {
        testContext(userId == "ssl" ? "ssl" : "tcp", current.adapter->getCommunicator(), current.ctx);
        return true;
    }
};

class SSLPermissionsVerifierI final : public Glacier2::SSLPermissionsVerifier
{
public:
    bool authorize(Glacier2::SSLInfo info, string&, const Ice::Current& current) const override
    {
        testContext(secureConnectionType(current.ctx), current.adapter->getCommunicator(), current.ctx);

        Ice::SSL::ScopedCertificate cert = Ice::SSL::decodeCertificate(info.certs[0]);
        testSubjectName(Ice::SSL::getSubjectName(cert.get()));
        return true;
    }
};

class SessionI final : public Glacier2::Session
{
public:
    SessionI(bool shutdown, string type) : _shutdown(shutdown), _type(std::move(type)) {}

    void destroy(const Ice::Current& current) override
    {
        testContext(_type, current.adapter->getCommunicator(), current.ctx);

        current.adapter->remove(current.id);
        if (_shutdown)
        {
            current.adapter->getCommunicator()->shutdown();
        }
    }

    void ice_ping(const Ice::Current& current) const override
    {
        testContext(_type, current.adapter->getCommunicator(), current.ctx);
    }

private:
    const bool _shutdown;
    const string _type;
};

class SessionManagerI final : public Glacier2::SessionManager
{
public:
    optional<Glacier2::SessionPrx>
    create(string userId, optional<Glacier2::SessionControlPrx>, const Ice::Current& current) override
    {
        testContext(userId == "ssl" ? "ssl" : "tcp", current.adapter->getCommunicator(), current.ctx);

        auto session = make_shared<SessionI>(false, userId == "ssl" ? "ssl" : "tcp");
        return current.adapter->addWithUUID<Glacier2::SessionPrx>(session);
    }
};

class SSLSessionManagerI final : public Glacier2::SSLSessionManager
{
public:
    optional<Glacier2::SessionPrx>
    create(Glacier2::SSLInfo info, optional<Glacier2::SessionControlPrx>, const Ice::Current& current) override
    {
        string type = secureConnectionType(current.ctx);
        testContext(type, current.adapter->getCommunicator(), current.ctx);

        test(info.remoteHost == "127.0.0.1");
        test(info.localHost == "127.0.0.1");
        test(
            info.localPort ==
            TestHelper::getTestPort(current.adapter->getCommunicator()->getProperties(), testPortIndex(type)));

        try
        {
            Ice::SSL::ScopedCertificate cert = Ice::SSL::decodeCertificate(info.certs[0]);
            testSubjectName(Ice::SSL::getSubjectName(cert.get()));
        }
        catch (const Ice::SSL::CertificateReadException&)
        {
            test(false);
        }

        // The wss session is the last session created by the client; destroying it shuts the server down.
        auto session = make_shared<SessionI>(type == "wss", type);
        return current.adapter->addWithUUID<Glacier2::SessionPrx>(session);
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
    auto adapter = communicator->createObjectAdapterWithEndpoints("SessionServer", getTestEndpoint(3, "tcp"));
    adapter->add(make_shared<PermissionsVerifierI>(), Ice::stringToIdentity("verifier"));
    adapter->add(make_shared<SSLPermissionsVerifierI>(), Ice::stringToIdentity("sslverifier"));
    adapter->add(make_shared<SessionManagerI>(), Ice::stringToIdentity("sessionmanager"));
    adapter->add(make_shared<SSLSessionManagerI>(), Ice::stringToIdentity("sslsessionmanager"));
    adapter->activate();
    communicator->waitForShutdown();
}

DEFINE_TEST(Server)
