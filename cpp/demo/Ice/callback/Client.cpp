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
	cerr << argv[0] << ": property `" << refProperty << "' not set" << endl;
	return EXIT_FAILURE;
    }

    ObjectPrx base = communicator()->stringToProxy(ref);
    CallbackPrx twoway = CallbackPrx::checkedCast(base->ice_twoway()->ice_timeout(-1)->ice_secure(false));
    if (!twoway)
    {
	cerr << argv[0] << ": invalid object reference" << endl;
	return EXIT_FAILURE;
    }
    CallbackPrx oneway = CallbackPrx::uncheckedCast(twoway->ice_oneway());
    CallbackPrx batchOneway = CallbackPrx::uncheckedCast(twoway->ice_batchOneway());
    CallbackPrx datagram = CallbackPrx::uncheckedCast(twoway->ice_datagram());
    CallbackPrx batchDatagram = CallbackPrx::uncheckedCast(twoway->ice_batchDatagram());
    
    ObjectAdapterPtr adapter = communicator()->createObjectAdapter("CallbackReceiverAdapter");
    CallbackReceiverPrx receiver = CallbackReceiverPrx::uncheckedCast(
	adapter->createProxy(stringToIdentity("callbackReceiver")));
    adapter->add(new CallbackReceiverI, stringToIdentity("callbackReceiver"));
    adapter->activate();

    bool secure = false;

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
		twoway->initiateCallback(receiver);
	    }
	    else if (c == 'o')
	    {
		oneway->initiateCallback(receiver);
	    }
	    else if (c == 'O')
	    {
		batchOneway->initiateCallback(receiver);
	    }
	    else if (c == 'd')
	    {
		datagram->initiateCallback(receiver);
	    }
	    else if (c == 'D')
	    {
		batchDatagram->initiateCallback(receiver);
	    }
	    else if (c == 'f')
	    {
		batchOneway->ice_flush();
		batchDatagram->ice_flush();
	    }
	    else if (c == 'S')
	    {
		secure = !secure;
		
		twoway = CallbackPrx::uncheckedCast(twoway->ice_secure(secure));
		oneway = CallbackPrx::uncheckedCast(oneway->ice_secure(secure));
		batchOneway = CallbackPrx::uncheckedCast(batchOneway->ice_secure(secure));
		datagram = CallbackPrx::uncheckedCast(datagram->ice_secure(secure));
		batchDatagram = CallbackPrx::uncheckedCast(batchDatagram->ice_secure(secure));
		
		if (secure)
		{
		    cout << "secure mode is now on" << endl;
		}
		else
		{
		    cout << "secure mode is now off" << endl;
		}
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
