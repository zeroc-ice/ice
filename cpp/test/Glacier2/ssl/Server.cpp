// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Application.h>
#include <Glacier2/PermissionsVerifier.h>
#include <Glacier2/Session.h>
#include <TestCommon.h>
#include <IceSSL/Plugin.h>

using namespace std;

namespace
{

void testContext(bool ssl, const Ice::Context& context)
{
    Ice::Context ctx = context;
    if(!ssl)
    {
        test(ctx["_con.type"] == "tcp");
        test(ctx["_con.localPort"] == "12347");
    }
    else
    {
        test(ctx["_con.type"] == "ssl");
        test(ctx["_con.localPort"] == "12348");
    }
    test(ctx["_con.localAddress"] == "127.0.0.1");
    test(ctx["_con.remotePort"] != "");
    test(ctx["_con.remoteAddress"] == "127.0.0.1");
}

}

class PermissionsVerifierI : public Glacier2::PermissionsVerifier
{
public:

    virtual bool
    checkPermissions(const string& userId, const string&, string&, const Ice::Current& current) const
    {
        testContext(userId == "ssl", current.ctx);
        return true;
    }
};

class SSLPermissionsVerifierI : public Glacier2::SSLPermissionsVerifier
{
public:

    virtual bool
    authorize(const Glacier2::SSLInfo& info, string&, const Ice::Current& current) const
    {
        testContext(true, current.ctx);

        IceSSL::CertificatePtr cert = IceSSL::Certificate::decode(info.certs[0]);
        test(cert->getIssuerDN() == IceSSL::DistinguishedName(
             "emailAddress=info@zeroc.com,C=US,ST=Florida,L=Jupiter,O=ZeroC\\, Inc.,OU=Ice,CN=Ice Tests CA"));
        test(cert->getSubjectDN() == IceSSL::DistinguishedName(
             "emailAddress=info@zeroc.com,C=US,ST=Florida,L=Jupiter,O=ZeroC\\, Inc.,OU=Ice,CN=client"));
        test(cert->checkValidity());

        return true;
    }
};

class SessionI : public Glacier2::Session
{
public:

    SessionI(bool shutdown, bool ssl) : _shutdown(shutdown), _ssl(ssl)
    {
    }

    virtual void
    destroy(const Ice::Current& current)
    {
        testContext(_ssl, current.ctx);

        current.adapter->remove(current.id);
        if(_shutdown)
        {
            current.adapter->getCommunicator()->shutdown();
        }
    }

    virtual void
    ice_ping(const Ice::Current& current) const
    {
        testContext(_ssl, current.ctx);
    }

private:

    const bool _shutdown;
    const bool _ssl;
};

class SessionManagerI : public Glacier2::SessionManager
{
public:

    virtual Glacier2::SessionPrx
    create(const string& userId, const Glacier2::SessionControlPrx&, const Ice::Current& current)
    {
        testContext(userId == "ssl", current.ctx);

        Glacier2::SessionPtr session = new SessionI(false, userId == "ssl");
        return Glacier2::SessionPrx::uncheckedCast(current.adapter->addWithUUID(session));
    }
};

class SSLSessionManagerI : public Glacier2::SSLSessionManager
{
public:

    virtual Glacier2::SessionPrx
    create(const Glacier2::SSLInfo& info, const Glacier2::SessionControlPrx&, const Ice::Current& current)
    {
        testContext(true, current.ctx);

        test(info.remoteHost == "127.0.0.1");
        test(info.localHost == "127.0.0.1");
        test(info.localPort == 12348);

        try
        {
            IceSSL::CertificatePtr cert = IceSSL::Certificate::decode(info.certs[0]);
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

        Glacier2::SessionPtr session = new SessionI(true, true);
        return Glacier2::SessionPrx::uncheckedCast(current.adapter->addWithUUID(session));
    }
};

class SessionServer : public Ice::Application
{
public:

    virtual int run(int, char*[]);
};

int
main(int argc, char* argv[])
{
#ifdef ICE_STATIC_LIBS
    Ice::registerIceSSL();
#endif

    SessionServer app;
    return app.main(argc, argv);
}

int
SessionServer::run(int, char**)
{
    Ice::ObjectAdapterPtr adapter = communicator()->createObjectAdapterWithEndpoints(
        "SessionServer", "tcp -h 127.0.0.1 -p 12350");
    adapter->add(new PermissionsVerifierI, communicator()->stringToIdentity("verifier"));
    adapter->add(new SSLPermissionsVerifierI, communicator()->stringToIdentity("sslverifier"));
    adapter->add(new SessionManagerI, communicator()->stringToIdentity("sessionmanager"));
    adapter->add(new SSLSessionManagerI, communicator()->stringToIdentity("sslsessionmanager"));
    adapter->activate();
    communicator()->waitForShutdown();
    return EXIT_SUCCESS;
}
