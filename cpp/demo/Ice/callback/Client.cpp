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
	"t: send callback as twoway\n"
	"o: send callback as oneway\n"
	"O: send callback as batch oneway\n"
	"d: send callback as datagram\n"
	"D: send callback as batch datagram\n"
	"f: flush all batch requests\n"
	"S: switch secure mode on/off\n"
	"s: shutdown server\n"
	"x: exit\n"
	"?: help\n";
}

int
CallbackClient::run(int argc, char* argv[])
{
    PropertiesPtr properties = communicator()->getProperties();
    const char* refProperty = "Callback.Callback";
    std::string ref = properties->getProperty(refProperty);
    if (ref.empty())
    {
	cerr << appName() << ": property `" << refProperty << "' not set" << endl;
	return EXIT_FAILURE;
    }

    ObjectPrx base = communicator()->stringToProxy(ref);
    CallbackPrx twoway = CallbackPrx::checkedCast(base->ice_twoway()->ice_timeout(-1)->ice_secure(false));
    if (!twoway)
    {
	cerr << appName() << ": invalid object reference" << endl;
	return EXIT_FAILURE;
    }
    CallbackPrx oneway = CallbackPrx::uncheckedCast(twoway->ice_oneway());
    CallbackPrx batchOneway = CallbackPrx::uncheckedCast(twoway->ice_batchOneway());
    CallbackPrx datagram = CallbackPrx::uncheckedCast(twoway->ice_datagram());
    CallbackPrx batchDatagram = CallbackPrx::uncheckedCast(twoway->ice_batchDatagram());
    
    ObjectAdapterPtr adapter = communicator()->createObjectAdapter("CallbackReceiverAdapter");
    adapter->add(new CallbackReceiverI, stringToIdentity("callbackReceiver"));
    adapter->activate();

    CallbackReceiverPrx twowayR = CallbackReceiverPrx::uncheckedCast(
	adapter->createProxy(stringToIdentity("callbackReceiver")));
    CallbackReceiverPrx onewayR = CallbackReceiverPrx::uncheckedCast(twowayR->ice_oneway());
//    CallbackReceiverPrx batchOnewayR = CallbackReceiverPrx::uncheckedCast(twowayR->ice_batchOneway());
    CallbackReceiverPrx datagramR = CallbackReceiverPrx::uncheckedCast(twowayR->ice_datagram());
//    CallbackReceiverPrx batchDatagramR = CallbackReceiverPrx::uncheckedCast(twowayR->ice_batchDatagram());

    bool secure = false;
    string secureStr = "";

    menu();

    char c;
    do
    {
	try
	{
	    cout << "==> ";
	    cin >> c;
	    if (c == 't')
	    {
		Context context;
		context["_fwd"] = "t" + secureStr;
		twoway->initiateCallback(twowayR, context);
	    }
	    else if (c == 'o')
	    {
		Context context;
		context["_fwd"] = "o" + secureStr;
		oneway->initiateCallback(onewayR, context);
	    }
	    else if (c == 'O')
	    {
		Context context;
		context["_fwd"] = "o" + secureStr;
		batchOneway->initiateCallback(onewayR, context);
	    }
	    else if (c == 'd')
	    {
		Context context;
		context["_fwd"] = "d" + secureStr;
		datagram->initiateCallback(datagramR, context);
	    }
	    else if (c == 'D')
	    {
		Context context;
		context["_fwd"] = "d" + secureStr;
		batchDatagram->initiateCallback(datagramR, context);
	    }
	    else if (c == 'f')
	    {
		batchOneway->ice_flush();
		batchDatagram->ice_flush();
	    }
	    else if (c == 'S')
	    {
		secure = !secure;
		secureStr = secure ? "s" : "";
		
		twoway = CallbackPrx::uncheckedCast(twoway->ice_secure(secure));
		oneway = CallbackPrx::uncheckedCast(oneway->ice_secure(secure));
		batchOneway = CallbackPrx::uncheckedCast(batchOneway->ice_secure(secure));
		datagram = CallbackPrx::uncheckedCast(datagram->ice_secure(secure));
		batchDatagram = CallbackPrx::uncheckedCast(batchDatagram->ice_secure(secure));

		twowayR = CallbackReceiverPrx::uncheckedCast(twowayR->ice_secure(secure));
		onewayR = CallbackReceiverPrx::uncheckedCast(onewayR->ice_secure(secure));
//		batchOnewayR = CallbackReceiverPrx::uncheckedCast(batchOnewayR->ice_secure(secure));
		datagramR = CallbackReceiverPrx::uncheckedCast(datagramR->ice_secure(secure));
//		batchDatagramR = CallbackReceiverPrx::uncheckedCast(batchDatagramR->ice_secure(secure));
		
		if (secure)
		{
		    cout << "secure mode is now on" << endl;
		}
		else
		{
		    cout << "secure mode is now off" << endl;
		}
	    }
	    else if (c == 's')
	    {
		twoway->shutdown();
	    }
	    else if (c == 'x')
	    {
		// Nothing to do
	    }
	    else if (c == '?')
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
    while (cin.good() && c != 'x');

    return EXIT_SUCCESS;
}
