// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
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

class SSLPermissionsVerifierI : public Glacier2::SSLPermissionsVerifier
{
public:

    virtual bool
    authorize(const Glacier2::SSLInfo& info, string&, const Ice::Current& current) const
    {
	IceSSL::CertificatePtr cert = IceSSL::Certificate::decode(info.certs[0]);
	test(cert->getIssuerDN() == IceSSL::DistinguishedName(
	    "emailAddress=info@zeroc.com,CN=ZeroC Test CA,OU=Ice,O=ZeroC\\, Inc.,"
	     "L=Palm Beach Gardens,ST=Florida,C=US"));
	test(cert->getSubjectDN() == IceSSL::DistinguishedName(
	    "CN=Client,emailAddress=info@zeroc.com,OU=Ice,O=ZeroC\\, Inc.,ST=Florida,C=US"));
	test(cert->checkValidity());

	return true;
    }
};

class PermissionsVerifierI : public Glacier2::PermissionsVerifier
{
public:

    virtual bool
    checkPermissions(const string&, const string&, string&, const Ice::Current& current) const
    {
	return true;
    }
};

class SessionI : public Glacier2::Session
{
public:

    SessionI(bool shutdown) : _shutdown(shutdown)
    {
    }

    virtual void
    destroy(const Ice::Current& current)
    {
	current.adapter->remove(current.id);
	if(_shutdown)
	{
	    current.adapter->getCommunicator()->shutdown();
	}
    }
private:
    const bool _shutdown;
};

class SessionManagerI : public Glacier2::SessionManager
{
public:

    virtual Glacier2::SessionPrx
    create(const string& userId, const Glacier2::SessionControlPrx&, const Ice::Current& current)
    {
	Glacier2::SessionPtr session = new SessionI(false);
	return Glacier2::SessionPrx::uncheckedCast(current.adapter->addWithUUID(session));
    }
};

class SSLSessionManagerI : public Glacier2::SSLSessionManager
{
public:

    virtual Glacier2::SessionPrx
    create(const Glacier2::SSLInfo& info, const Glacier2::SessionControlPrx&, const Ice::Current& current)
    {
	test(info.remoteHost == "127.0.0.1");
	test(info.localHost == "127.0.0.1");
	test(info.localPort == 12348);
	try
	{
	    IceSSL::CertificatePtr cert = IceSSL::Certificate::decode(info.certs[0]);
	    test(cert->getIssuerDN() == IceSSL::DistinguishedName(
		"emailAddress=info@zeroc.com,CN=ZeroC Test CA,OU=Ice,O=ZeroC\\, Inc.,L=Palm Beach Gardens,"
		"ST=Florida,C=US"));
	    test(cert->getSubjectDN() == IceSSL::DistinguishedName(
		"CN=Client,emailAddress=info@zeroc.com,OU=Ice,O=ZeroC\\, Inc.,ST=Florida,C=US"));
	    test(cert->checkValidity());
	}
	catch(const IceSSL::CertificateReadException&)
	{
	    test(false);
	}

	Glacier2::SessionPtr session = new SessionI(true);
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
    SessionServer app;
    return app.main(argc, argv);
}

int
SessionServer::run(int argc, char* argv[])
{
    Ice::ObjectAdapterPtr adapter = communicator()->createObjectAdapterWithEndpoints(
	"SessionServer", "tcp -h 127.0.0.1 -p 12350 -t 10000");
    adapter->add(new PermissionsVerifierI, communicator()->stringToIdentity("verifier"));
    adapter->add(new SSLPermissionsVerifierI, communicator()->stringToIdentity("sslverifier"));
    adapter->add(new SessionManagerI, communicator()->stringToIdentity("sessionmanager"));
    adapter->add(new SSLSessionManagerI, communicator()->stringToIdentity("sslsessionmanager"));
    adapter->activate();
    communicator()->waitForShutdown();
    return EXIT_SUCCESS;
}
