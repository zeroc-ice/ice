// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Application.h>
#include <Glacier/Glacier.h>
#include <TestCommon.h>
#include <CallbackI.h>
#include <Ice/Security.h>
#include <IceUtil/Base64.h>
#include <CertVerifier.h>

using namespace std;
using namespace Ice;

class CallbackClient : public Application
{
public:

    virtual int run(int, char*[]);
};

int
main(int argc, char* argv[])
{
    CallbackClient app;
    return app.main(argc, argv);
}

int
CallbackClient::run(int argc, char* argv[])
{
    string ref;

    cout << "creating and activating callback receiver adapter... " << flush;
    ObjectAdapterPtr adapter = communicator()->createObjectAdapterWithEndpoints("CallbackReceiverAdapter", "default");
    adapter->activate();
    cout << "ok" << endl;

    cout << "creating and adding callback receiver object... " << flush;
    CallbackReceiverI* callbackReceiverImpl = new CallbackReceiverI;
    ObjectPtr callbackReceiver = callbackReceiverImpl;
    adapter->add(callbackReceiver, stringToIdentity("callbackReceiver"));
    cout << "ok" << endl;

    cout << "testing stringToProxy for glacier starter... " << flush;
    ref = "Glacier#starter:default -p 12346 -t 5000";
    ObjectPrx starterBase = communicator()->stringToProxy(ref);
    cout << "ok" << endl;

    cout << "testing checked cast for glacier starter... " << flush;
    Glacier::StarterPrx starter = Glacier::StarterPrx::checkedCast(starterBase);
    test(starter);
    cout << "ok" << endl;

    ByteSeq privateKey;
    ByteSeq publicKey;
    ByteSeq routerCert;

    cout << "starting up glacier router... " << flush;
    RouterPrx router;
    try
    {
	router = starter->startRouter("", "", privateKey, publicKey, routerCert);
    }
    catch (const Glacier::CannotStartRouterException& ex)
    {
	cerr << appName() << ": " << ex << ":\n" << ex.reason << endl;
	return EXIT_FAILURE;
    }
    catch (const Glacier::InvalidPasswordException& ex)
    {
	cerr << appName() << ": " << ex << endl;
	return EXIT_FAILURE;
    }

    PropertiesPtr properties = communicator()->getProperties();

    string privateKeyBase64 = IceUtil::Base64::encode(privateKey);
    string publicKeyBase64  = IceUtil::Base64::encode(publicKey);
    string routerCertString = IceUtil::Base64::encode(routerCert);

    string sysIdentifier = properties->getProperty("Ice.Security.Ssl.Config");
    IceSecurity::Ssl::SslContextType contextType = IceSecurity::Ssl::ClientServer;
    IceSecurity::Ssl::CertificateVerifierPtr certVerifier = new CertVerifier(routerCert);
    IceSecurity::Ssl::setSystemCertificateVerifier(sysIdentifier, contextType, certVerifier);
    IceSecurity::Ssl::setSystemCertAuthCertificate(sysIdentifier, IceSecurity::Ssl::Client, routerCertString);
    IceSecurity::Ssl::setSystemRSAKeysBase64(sysIdentifier, IceSecurity::Ssl::Client, privateKeyBase64, publicKeyBase64);

    // Set the keys overrides for the server.
    properties->setProperty("Ice.Security.Ssl.Overrides.Server.RSA.PrivateKey", privateKeyBase64);
    properties->setProperty("Ice.Security.Ssl.Overrides.Server.RSA.Certificate", publicKeyBase64);

    test(router);
    cout << "ok" << endl;

    cout << "pinging glacier router... " << flush;
    router->ice_ping();
    cout << "ok" << endl;

    cout << "installing glacier router... " << flush;
    communicator()->setDefaultRouter(router);
    adapter->addRouter(router);
    cout << "ok" << endl;

    cout << "testing stringToProxy... " << flush;
    ref = "callback:tcp -p 12345 -t 5000";
    ObjectPrx base = communicator()->stringToProxy(ref);
    cout << "ok" << endl;

    cout << "testing checked cast... " << flush;
    CallbackPrx twoway = CallbackPrx::checkedCast(base->ice_twoway()->ice_timeout(-1)->ice_secure(false));
    test(twoway);
    cout << "ok" << endl;

    CallbackReceiverPrx twowayR = CallbackReceiverPrx::uncheckedCast(
	adapter->createProxy(stringToIdentity("callbackReceiver")));
	
    {
	cout << "testing oneway callback... " << flush;
	CallbackPrx oneway = CallbackPrx::uncheckedCast(twoway->ice_oneway());
	CallbackReceiverPrx onewayR = CallbackReceiverPrx::uncheckedCast(twowayR->ice_oneway());
	Context context;
	context["_fwd"] = "o";
	oneway->initiateCallback(onewayR, context);
	test(callbackReceiverImpl->callbackOK());
	cout << "ok" << endl;
    }

    {
	cout << "testing twoway callback... " << flush;
	Context context;
	context["_fwd"] = "t";
	twoway->initiateCallback(twowayR, context);
	test(callbackReceiverImpl->callbackOK());
	cout << "ok" << endl;
    }

    {
	cout << "ditto, but with user exception... " << flush;
	Context context;
	context["_fwd"] = "t";
	try
	{
	    twoway->initiateCallbackEx(twowayR, context);
	    test(false);
	}
	catch (const CallbackException& ex)
	{
	    test(ex.someValue == 3.14);
	    test(ex.someString == "3.14");
	}
	test(callbackReceiverImpl->callbackOK());
	cout << "ok" << endl;
    }

    cout << "testing server shutdown... " << flush;
    twoway->shutdown();
    // No ping, otherwise the glacier router prints a warning
    // message if it's started with --Ice.ConnectionWarnings.
    cout << "ok" << endl;
    /*
    try
    {
	twoway->ice_ping();
	test(false);
    }
    // If we use the glacier router, the exact exception reason gets
    // lost.
    //catch(const ConnectFailedException&)
    catch(const UnknownLocalException&)
    {
	cout << "ok" << endl;
    }
    */

    cout << "shutting down glacier router... " << flush;
    router->shutdown();
    try
    {
	router->ice_ping();
	test(false);
    }
    catch(const ConnectFailedException&)
    {
	cout << "ok" << endl;
    }

    return EXIT_SUCCESS;
}
