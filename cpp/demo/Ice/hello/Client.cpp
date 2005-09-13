// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Application.h>
#include <Hello.h>

using namespace std;
using namespace Demo;

class HelloClient : public Ice::Application
{
public:

    virtual int run(int, char*[]);

private:

    void menu();
};

int
main(int argc, char* argv[])
{
    HelloClient app;
    return app.main(argc, argv, "config");
}

int
HelloClient::run(int argc, char* argv[])
{
    Ice::PropertiesPtr properties = communicator()->getProperties();
    const char* proxyProperty = "Hello.Proxy";
    string proxy = properties->getProperty(proxyProperty);
    if(proxy.empty())
    {
	cerr << argv[0] << ": property `" << proxyProperty << "' not set" << endl;
	return EXIT_FAILURE;
    }

    HelloPrx twoway = HelloPrx::checkedCast(
	communicator()->stringToProxy(proxy)->ice_twoway()->ice_timeout(-1)->ice_secure(false));
    if(!twoway)
    {
	cerr << argv[0] << ": invalid proxy" << endl;
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
	    if(c == 't')
	    {
		twoway->sayHello();
	    }
	    else if(c == 'o')
	    {
		oneway->sayHello();
	    }
	    else if(c == 'O')
	    {
		batchOneway->sayHello();
	    }
	    else if(c == 'd')
	    {
 		if(secure)
		{
		    cout << "secure datagrams are not supported" << endl;
		}
		else
		{
		    datagram->sayHello();
		}
	    }
	    else if(c == 'D')
	    {
 		if(secure)
		{
		    cout << "secure datagrams are not supported" << endl;
		}
		else
		{
		    batchDatagram->sayHello();
		}
	    }
	    else if(c == 'f')
	    {
		communicator()->flushBatchRequests();
	    }
	    else if(c == 'T')
	    {
		if(timeout == -1)
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
		
		if(timeout == -1)
		{
		    cout << "timeout is now switched off" << endl;
		}
		else
		{
		    cout << "timeout is now set to 2000ms" << endl;
		}
	    }
	    else if(c == 'S')
	    {
		secure = !secure;
		
		twoway = HelloPrx::uncheckedCast(twoway->ice_secure(secure));
		oneway = HelloPrx::uncheckedCast(oneway->ice_secure(secure));
		batchOneway = HelloPrx::uncheckedCast(batchOneway->ice_secure(secure));
		datagram = HelloPrx::uncheckedCast(datagram->ice_secure(secure));
		batchDatagram = HelloPrx::uncheckedCast(batchDatagram->ice_secure(secure));
		
		if(secure)
		{
		    cout << "secure mode is now on" << endl;
		}
		else
		{
		    cout << "secure mode is now off" << endl;
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
	catch(const Ice::Exception& ex)
	{
	    cerr << ex << endl;
	}
    }
    while(cin.good() && c != 'x');

    return EXIT_SUCCESS;
}

void
HelloClient::menu()
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
