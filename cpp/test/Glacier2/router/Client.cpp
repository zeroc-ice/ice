// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Application.h>
#include <Glacier2/Router.h>
#include <TestCommon.h>
#include <CallbackI.h>

using namespace std;
using namespace Ice;
using namespace Test;

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
    ObjectAdapterPtr adapter = communicator()->createObjectAdapterWithEndpoints("CallbackReceiverAdapter", "");
    adapter->activate();
    // Put the print statement after activate(), so that if
    // Ice.PrintAdapterReady is set, the "ready" is the first output
    // from the client, and not the print statement below. Otherwise
    // the Python test scripts will be confused, as they expect the
    // "ready" from the Object Adapter to be the first thing that is
    // printed.
    cout << "creating and activating callback receiver adapter... " << flush;
    cout << "ok" << endl;

    cout << "testing stringToProxy for router... " << flush;
    ObjectPrx routerBase = communicator()->stringToProxy("abc/def:default -p 12346 -t 30000");
    cout << "ok" << endl;

    cout << "testing checked cast for router... " << flush;
    Glacier2::RouterPrx router = Glacier2::RouterPrx::checkedCast(routerBase);
    test(router);
    cout << "ok" << endl;

    cout << "creating session with router... " << flush;
    Glacier2::SessionPrx session = router->createSession("dummy", "abc123");
    cout << "ok" << endl;

    cout << "installing router with communicator... " << flush;
    communicator()->setDefaultRouter(router);
    cout << "ok" << endl;

    cout << "installing router with object adapter... " << flush;
    adapter->addRouter(router);
    cout << "ok" << endl;

    cout << "getting category from router... " << flush;
    string category = router->getServerProxy()->ice_getIdentity().category;
    cout << "ok" << endl;

    cout << "creating and adding callback receiver object... " << flush;
    CallbackReceiverI* callbackReceiverImpl = new CallbackReceiverI;
    ObjectPtr callbackReceiver = callbackReceiverImpl;
    Identity callbackReceiverIdent;
    callbackReceiverIdent.name = "callbackReceiver";
    callbackReceiverIdent.category = category;
    CallbackReceiverPrx twowayR = CallbackReceiverPrx::uncheckedCast(
	adapter->add(callbackReceiver, callbackReceiverIdent));
    cout << "ok" << endl;

    cout << "testing stringToProxy... " << flush;
    ObjectPrx base = communicator()->stringToProxy("callback:tcp -p 12345 -t 10000");
    cout << "ok" << endl;

    cout << "testing checked cast... " << flush;
    CallbackPrx twoway = CallbackPrx::checkedCast(base);
    test(twoway);
    cout << "ok" << endl;
	
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
	catch(const CallbackException& ex)
	{
	    test(ex.someValue == 3.14);
	    test(ex.someString == "3.14");
	}
	test(callbackReceiverImpl->callbackOK());
	cout << "ok" << endl;
    }

    cout << "testing server shutdown... " << flush;
    twoway->shutdown();
    // No ping, otherwise the router prints a warning message if it's
    // started with --Ice.Warn.Connections.
    cout << "ok" << endl;
    /*
    try
    {
	twoway->ice_ping();
	test(false);
    }
    // If we use the glacier router, the exact exception reason gets
    // lost.
    catch(const UnknownLocalException&)
    {
	cout << "ok" << endl;
    }
    */

    cout << "destroying session with router... " << flush;
    router->destroySession();
    cout << "ok" << endl;

    return EXIT_SUCCESS;
}
