// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Ice.h>
#include <Hello.h>

using namespace std;

void
menu()
{
    cout <<
	"usage:\n"
	"t: send greeting as twoway\n"
	"o: send greeting as oneway\n"
	"O: send greeting as batch oneway\n"
	"d: send greeting as datagram\n"
	"D: send greeting as batch datagram\n"
	"f: flush all batch requests\n"
	"T: set a timeout\n"
	"S: switch secure mode on/off\n"
	"s: shutdown server\n"
	"x: exit\n"
	"?: help\n";
}

int
run(int argc, char* argv[], const Ice::CommunicatorPtr& communicator)
{
    Ice::PropertiesPtr properties = communicator->getProperties();
    const char* refProperty = "Hello.Hello";
    std::string ref = properties->getProperty(refProperty);
    if (ref.empty())
    {
	cerr << argv[0] << ": property `" << refProperty << "' not set" << endl;
	return EXIT_FAILURE;
    }

    Ice::ObjectPrx base = communicator->stringToProxy(ref);
    HelloPrx twoway = HelloPrx::checkedCast(base);
    if (!twoway)
    {
	cerr << argv[0] << ": invalid object reference" << endl;
	return EXIT_FAILURE;
    }
    HelloPrx oneway = HelloPrx::uncheckedCast(twoway->ice_oneway());
    HelloPrx batchOneway = HelloPrx::uncheckedCast(twoway->ice_batchOneway());
    HelloPrx datagram = HelloPrx::uncheckedCast(twoway->ice_datagram());
    HelloPrx batchDatagram = HelloPrx::uncheckedCast(twoway->ice_batchDatagram());

    bool secure = false;
    int timeout = -1;

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
		twoway->hello();
	    }
	    else if (c == 'o')
	    {
		oneway->hello();
	    }
	    else if (c == 'O')
	    {
		batchOneway->hello();
	    }
	    else if (c == 'd')
	    {
		datagram->hello();
	    }
	    else if (c == 'D')
	    {
		batchDatagram->hello();
	    }
	    else if (c == 'f')
	    {
		batchOneway->ice_flush();
		batchDatagram->ice_flush();
	    }
	    else if (c == 'T')
	    {
		if (timeout == -1)
		{
		    timeout = 2000;
		}
		else
		{
		    timeout = -1;
		}
		
		twoway = HelloPrx::uncheckedCast(twoway->ice_timeout(timeout));
		oneway = HelloPrx::uncheckedCast(oneway->ice_timeout(timeout));
		batchOneway = HelloPrx::uncheckedCast(batchOneway->ice_timeout(timeout));
		
		if (timeout == -1)
		{
		    cout << "timeout is now switched off" << endl;
		}
		else
		{
		    cout << "timeout is now set to 2000ms" << endl;
		}
	    }
	    else if (c == 'S')
	    {
		secure = !secure;
		
		twoway = HelloPrx::uncheckedCast(twoway->ice_secure(secure));
		oneway = HelloPrx::uncheckedCast(oneway->ice_secure(secure));
		batchOneway = HelloPrx::uncheckedCast(batchOneway->ice_secure(secure));
		datagram = HelloPrx::uncheckedCast(datagram->ice_secure(secure));
		batchDatagram = HelloPrx::uncheckedCast(batchDatagram->ice_secure(secure));
		
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
	catch(const Ice::Exception& ex)
	{
	    cerr << "HelloClient:" << ex << endl;
	}
    }
    while (cin.good() && c != 'x');

    return EXIT_SUCCESS;
}

int
main(int argc, char* argv[])
{
    int status;
    Ice::CommunicatorPtr communicator;

    try
    {
	Ice::PropertiesPtr properties = Ice::createPropertiesFromFile(argc, argv, "config");
	communicator = Ice::initializeWithProperties(properties);
	status = run(argc, argv, communicator);
    }
    catch(const Ice::Exception& ex)
    {
	cerr << ex << endl;
	status = EXIT_FAILURE;
    }

    if (communicator)
    {
	try
	{
	    communicator->destroy();
	}
	catch(const Ice::Exception& ex)
	{
	    cerr << ex << endl;
	    status = EXIT_FAILURE;
	}
    }

    return status;
}
