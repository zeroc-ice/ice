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
    return app.main(argc, argv, "config");
}

void
menu()
{
    cout <<
	"usage:\n"
	"t: invoke callback as twoway\n"
	"o: invoke callback as oneway\n"
	"O: invoke callback as batch oneway\n"
	"f: flush all batch requests\n"
	"v: set/reset override context field\n"
	"s: shutdown server\n"
	"x: exit\n"
	"?: help\n";
}

int
CallbackClient::run(int argc, char* argv[])
{
    RouterPrx defaultRouter = communicator()->getDefaultRouter();
    if(!defaultRouter)
    {
	cerr << argv[0] << ": no default router set" << endl;
	return EXIT_FAILURE;
    }

    Glacier2::RouterPrx router = Glacier2::RouterPrx::checkedCast(defaultRouter);
    {
	if(!router)
	{
	    cerr << argv[0] << ": configured router is not a Glacier2 router" << endl;
	    return EXIT_FAILURE;
	}
    }

    while(true)
    {
	string id;
	cout << "user id: " << flush;
	cin >> id;

	string pw;
	cout << "password: " << flush;
	cin >> pw;
    
	try
	{
	    router->createSession(id, pw);
	    break;
	}
	catch(const Glacier2::PermissionDeniedException& ex)
	{
	    cout << "permission denied:\n" << ex.reason << endl;
	}
    }

    PropertiesPtr properties = communicator()->getProperties();
    const char* proxyProperty = "Callback.Client.Callback";
    std::string proxy = properties->getProperty(proxyProperty);
    if(proxy.empty())
    {
	cerr << appName() << ": property `" << proxyProperty << "' not set" << endl;
	return EXIT_FAILURE;
    }

    ObjectPrx base = communicator()->stringToProxy(proxy);
    CallbackPrx twoway = CallbackPrx::checkedCast(base);
    CallbackPrx oneway = CallbackPrx::uncheckedCast(twoway->ice_oneway());
    CallbackPrx batchOneway = CallbackPrx::uncheckedCast(twoway->ice_batchOneway());
    CallbackPrx datagram = CallbackPrx::uncheckedCast(twoway->ice_datagram());
    CallbackPrx batchDatagram = CallbackPrx::uncheckedCast(twoway->ice_batchDatagram());
    
    ObjectAdapterPtr adapter = communicator()->createObjectAdapter("Callback.Client");
    adapter->add(new CallbackReceiverI, stringToIdentity("callbackReceiver"));
    adapter->activate();

    CallbackReceiverPrx twowayR = CallbackReceiverPrx::uncheckedCast(
	adapter->createProxy(stringToIdentity("callbackReceiver")));
    CallbackReceiverPrx onewayR = CallbackReceiverPrx::uncheckedCast(twowayR->ice_oneway());
    CallbackReceiverPrx datagramR = CallbackReceiverPrx::uncheckedCast(twowayR->ice_datagram());

    string override;

    menu();

    char c;
    do
    {
	try
	{
	    cout << "==> ";
	    cin >> c;
	    if(c == 't')
	    {
		Context context;
		context["_fwd"] = "t";
		if(!override.empty())
		{
		    context["_ovrd"] = override;
		}
		twoway->initiateCallback(twowayR, context);
	    }
	    else if(c == 'o')
	    {
		Context context;
		context["_fwd"] = "o";
		if(!override.empty())
		{
		    context["_ovrd"] = override;
		}
		oneway->initiateCallback(onewayR, context);
	    }
	    else if(c == 'O')
	    {
		Context context;
		context["_fwd"] = "O";
		if(!override.empty())
		{
		    context["_ovrd"] = override;
		}
		batchOneway->initiateCallback(onewayR, context);
	    }
	    else if(c == 'f')
	    {
		communicator()->flushBatchRequests();
	    }
	    else if(c == 'v')
	    {
		if(override.empty())
                {
		    override = "some_value";
		    cout << "override context field is now `" << override << "'" << endl;
		}
		else
		{
		    override.clear();
		    cout << "override context field is empty" << endl;
		}
	    }
	    else if(c == 's')
	    {
		twoway->shutdown();
	    }
	    else if(c == 'x')
	    {
		// Nothing to do
	    }
	    else if(c == '?')
	    {
		menu();
	    }
	    else
	    {
		cout << "unknown command `" << c << "'" << endl;
		menu();
	    }
	}
	catch(const Exception& ex)
	{
	    cerr << ex << endl;
	}
    }
    while(cin.good() && c != 'x');

    return EXIT_SUCCESS;
}
