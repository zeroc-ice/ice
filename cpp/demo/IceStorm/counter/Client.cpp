// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Application.h>
 
#include <CounterObserverI.h>

using namespace std;
using namespace Demo;

class Client : public Ice::Application
{
public:

    virtual int run(int, char*[]);
private:

    void menu();
};

int
main(int argc, char* argv[])
{
    Client app;
    return app.main(argc, argv, "config.client");
}

int
Client::run(int argc, char* argv[])
{
    Ice::PropertiesPtr properties = communicator()->getProperties();

    const string proxyProperty = "Counter.Proxy";
    string proxy = properties->getProperty(proxyProperty);
    if(proxy.empty())
    {
	cerr << appName() << ": property `" << proxyProperty << "' not set" << endl;
	return EXIT_FAILURE;
    }

    CounterPrx counter = CounterPrx::uncheckedCast(communicator()->stringToProxy(proxy));
    if(!counter)
    {
	cerr << appName() << ": invalid proxy" << endl;
	return EXIT_FAILURE;
    }

    Ice::ObjectAdapterPtr adapter = communicator()->createObjectAdapterWithEndpoints("Observer", "tcp");
    CounterObserverPrx observer = CounterObserverPrx::uncheckedCast(adapter->addWithUUID(new CounterObserverI));
    adapter->activate();

    counter->subscribe(observer);

    menu();

    char c;
    do
    {
	try
	{
	    cout << "==> ";
	    cin >> c;
	    if(c == 'i')
	    {
		counter->inc(1);
	    }
	    else if(c == 'd')
	    {
		counter->inc(-1);
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

    counter->unsubscribe(observer);

    return EXIT_SUCCESS;
}

void
Client::menu()
{
    cout <<
	"usage:\n"
	"i: increment the counter\n"
	"d: decrement the counter\n"
	"x: exit\n"
	"?: help\n";
}
