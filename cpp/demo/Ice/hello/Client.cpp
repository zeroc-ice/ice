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

int
run(int argc, char* argv[], Ice::CommunicatorPtr communicator)
{
    Ice::PropertiesPtr properties = communicator->getProperties();
    std::string ref = properties->getProperty("Hello.Hello");
    Ice::ObjectPrx base = communicator->stringToProxy(ref);

    HelloPrx hello = HelloPrx::checkedCast(base);
    if (!hello)
    {
	cerr << argv[0] << ": invalid object reference" << endl;
	return EXIT_FAILURE;
    }

    HelloPrx timeout = HelloPrx::uncheckedCast(hello->_timeout(2000));
    HelloPrx oneway = HelloPrx::uncheckedCast(hello->_oneway());
    HelloPrx batchOneway = HelloPrx::uncheckedCast(hello->_batchOneway());
    HelloPrx datagram = HelloPrx::uncheckedCast(hello->_datagram());
    HelloPrx batchDatagram = HelloPrx::uncheckedCast(hello->_batchDatagram());

    cout << "h: hello" << endl;
    cout << "t: hello w/ 2s timeout" << endl;
    cout << "o: hello as oneway" << endl;
    cout << "O: hello as batch oneway" << endl;
    cout << "d: hello as datagram" << endl;
    cout << "D: hello as batch datagram" << endl;
    cout << "f: flush batch request" << endl;
    cout << "s: shutdown" << endl;
    cout << "x: exit" << endl;

    char c;
    do
    {
	cout << "==> ";
	cin >> c;
	if (c == 'h')
	{
	    hello->hello();
	}
	else if (c == 't')
	{
	    timeout->hello();
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
	    batchOneway->_flush();
	    batchDatagram->_flush();
	}
  	else if (c == 's')
	{
  	    hello->shutdown();
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
    catch(const Ice::LocalException& ex)
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
	catch(const Ice::LocalException& ex)
	{
	    cerr << ex << endl;
	    status = EXIT_FAILURE;
	}
    }

    return status;
}
