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

    cout << "testing stringToProxy for router starter... " << flush;
    ref = "Glacier#starter:default -p 12346 -t 2000";
    ObjectPrx starterBase = communicator()->stringToProxy(ref);
    cout << "ok" << endl;

    cout << "testing checked cast for router starter... " << flush;
    Glacier::StarterPrx starter = Glacier::StarterPrx::checkedCast(starterBase);
    test(starter);
    cout << "ok" << endl;

    cout << "testing router startup... " << flush;
    RouterPrx router = starter->startRouter("", "");
    test(router);
    router->ice_ping();
    cout << "ok" << endl;

    cout << "testing stringToProxy... " << flush;
    ref = "callback:default -p 12345 -t 2000";
    ObjectPrx base = communicator()->stringToProxy(ref);
    cout << "ok" << endl;

    cout << "testing checked cast... " << flush;
    CallbackPrx twoway = CallbackPrx::checkedCast(base->ice_twoway()->ice_timeout(-1)->ice_secure(false));
    test(twoway);
    cout << "ok" << endl;

    CallbackReceiverI* callbackReceiverImpl = new CallbackReceiverI;
    ObjectPtr callbackReceiver = callbackReceiverImpl;

    ObjectAdapterPtr adapter = communicator()->createObjectAdapterWithEndpoints("CallbackReceiverAdapter", "default");
    adapter->add(callbackReceiver, stringToIdentity("callbackReceiver"));
    adapter->activate();

    CallbackReceiverPrx twowayR = CallbackReceiverPrx::uncheckedCast(
	adapter->createProxy(stringToIdentity("callbackReceiver")));
    
    {
	cout << "testing callback... " << flush;
	Context context;
	context["_fwd"] = "t";
	twoway->initiateCallback(twowayR, context);
	test(callbackReceiverImpl->callbackOK());
	cout << "ok" << endl;
    }

    cout << "testing server shutdown... " << flush;
    twoway->shutdown();
    try
    {
	twoway->ice_ping();
	test(false);
    }
    catch(const ConnectFailedException&)
    {
	cout << "ok" << endl;
    }

    return EXIT_SUCCESS;
}
