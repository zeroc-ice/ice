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

class PermissionsVerifierI : public Glacier2::PermissionsVerifier
{
public:

    virtual bool
    checkPermissions(const string& userId, const string&, string&, const Ice::Current& current) const
    {
	map<string, string>::const_iterator p = current.ctx.find("SSL.Active");
	
	if(userId == "nossl")
	{
	    return p == current.ctx.end();
	}
	else if(userId == "ssl")
	{
	    return p != current.ctx.end();
	}
	return false;
    }
};

class SessionI : public Glacier2::Session
{
public:

    SessionI(const string& userId) :
	_userId(userId)
    {
    }

    virtual void
    destroy(const Ice::Current& current)
    {
	current.adapter->remove(current.id);
	if(_userId == "ssl")
	{
	    current.adapter->getCommunicator()->shutdown();
	}
    }
    
private:

    const string _userId;
};

class SessionManagerI : public Glacier2::SessionManager
{
public:

    virtual Glacier2::SessionPrx
    create(const string& userId, const Glacier2::SessionControlPrx&, const Ice::Current& current)
    {
	if(userId == "ssl")
	{
	    test(current.ctx.find("SSL.Active")->second == "1");
	    test(current.ctx.find("SSL.Cipher") != current.ctx.end());
	    test(current.ctx.find("SSL.Remote.Host")->second == "127.0.0.1");
	    test(current.ctx.find("SSL.Local.Host")->second == "127.0.0.1");
	    test(current.ctx.find("SSL.Local.Port")->second == "12348");
	    try
	    {
		IceSSL::CertificatePtr cert = IceSSL::Certificate::decodePEM(current.ctx.find("SSL.PeerCert")->second);
		test(cert->getIssuerDN() ==
		     "/C=US/ST=Florida/L=Palm Beach Gardens/O=ZeroC, Inc."
		     "/OU=Ice/CN=ZeroC Test CA/emailAddress=info@zeroc.com");
		test(cert->getSubjectDN() ==
		     "/C=US/ST=Florida/O=ZeroC, Inc./OU=Ice/emailAddress=info@zeroc.com/CN=Client");
		test(cert->checkValidity());
	    }
	    catch(const IceSSL::CertificateReadException&)
	    {
		test(false);
	    }
	}
	Glacier2::SessionPtr session = new SessionI(userId);
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
    adapter->add(new PermissionsVerifierI, Ice::stringToIdentity("verifier"));
    adapter->add(new SessionManagerI, Ice::stringToIdentity("sessionmanager"));
    adapter->activate();
    communicator()->waitForShutdown();
    return EXIT_SUCCESS;
}
