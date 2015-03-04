// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
 
#include <CounterObserverI.h>

using namespace std;
using namespace Demo;

class Client : public Ice::Application
{
public:

    virtual int run(int, char*[]);
private:

    void menu(const MTPrinterPtr& printer);
};

int
main(int argc, char* argv[])
{
    Client app;
    return app.main(argc, argv, "config.client");
}

int
Client::run(int argc, char*[])
{
    if(argc > 1)
    {
        cerr << appName() << ": too many arguments" << endl;
        return EXIT_FAILURE;
    }

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

    MTPrinterPtr printer = new MTPrinter();

    Ice::ObjectAdapterPtr adapter = communicator()->createObjectAdapterWithEndpoints("Observer", "tcp");
    CounterObserverPrx observer = 
        CounterObserverPrx::uncheckedCast(adapter->addWithUUID(new CounterObserverI(printer)));
    adapter->activate();

    counter->subscribe(observer);

    menu(printer);

    char c;
    do
    {
        try
        {
            printer->print("==> ");
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
                menu(printer);
            }
            else
            {
                cout << "unknown command `" << c << "'" << endl;
                menu(printer);
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
Client::menu(const MTPrinterPtr& printer)
{
    printer->print(
        "usage:\n"
        "i: increment the counter\n"
        "d: decrement the counter\n"
        "x: exit\n"
        "?: help\n");
}
