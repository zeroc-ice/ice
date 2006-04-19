// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <NestedI.h>
#include <Ice/Application.h>

using namespace std;
using namespace Demo;

class NestedClient : public Ice::Application
{
public:

    virtual int run(int, char*[]);
};

int
main(int argc, char* argv[])
{
    NestedClient app;
    return app.main(argc, argv, "config.client");
}

int
NestedClient::run(int argc, char* argv[])
{
    Ice::PropertiesPtr properties = communicator()->getProperties();
    const char* proxyProperty = "Nested.Client.NestedServer";
    std::string proxy = properties->getProperty(proxyProperty);
    if(proxy.empty())
    {
	cerr << appName() << ": property `" << proxyProperty << "' not set" << endl;
	return EXIT_FAILURE;
    }

    NestedPrx nested = NestedPrx::checkedCast(communicator()->stringToProxy(proxy));
    if(!nested)
    {
	cerr << appName() << ": invalid proxy" << endl;
	return EXIT_FAILURE;
    }

    Ice::ObjectAdapterPtr adapter = communicator()->createObjectAdapter("Nested.Client");
    NestedPrx self = NestedPrx::uncheckedCast(adapter->createProxy(Ice::stringToIdentity("nestedClient")));
    adapter->add(new NestedI(self), Ice::stringToIdentity("nestedClient"));
    adapter->activate();

    cout << "Note: The maximum nesting level is sz * 2, with sz being\n"
	 << "the maximum number of threads in the server thread pool. if\n"
	 << "you specify a value higher than that, the application will\n"
	 << "block or timeout.\n"
	 << endl;

    string s;
    do
    {
	try
	{
	    cout << "enter nesting level or 'x' for exit: ";
	    cin >> s;
	    int level = atoi(s.c_str());
	    if(level > 0)
	    {
		nested->nestedCall(level, self);
	    }
	}
	catch(const Ice::Exception& ex)
	{
	    cerr << ex << endl;
	}
    }
    while(cin.good() && s != "x");

    return EXIT_SUCCESS;
}
