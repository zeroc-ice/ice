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
#include <NestedI.h>

using namespace std;
using namespace Ice;

class NestedClient : public Application
{
public:

    virtual int run(int, char*[]);
};

int
main(int argc, char* argv[])
{
    NestedClient app;
    return app.main(argc, argv, "config");
}

int
NestedClient::run(int argc, char* argv[])
{
    PropertiesPtr properties = communicator()->getProperties();
    const char* refProperty = "Nested.NestedServer";
    std::string ref = properties->getProperty(refProperty);
    if(ref.empty())
    {
	cerr << appName() << ": property `" << refProperty << "' not set" << endl;
	return EXIT_FAILURE;
    }

    ObjectPrx base = communicator()->stringToProxy(ref);
    NestedPrx nested = NestedPrx::checkedCast(base);
    if(!nested)
    {
	cerr << appName() << ": invalid object reference" << endl;
	return EXIT_FAILURE;
    }

    ObjectAdapterPtr adapter = communicator()->createObjectAdapter("NestedClientAdapter");
    NestedPrx self = NestedPrx::uncheckedCast(adapter->createProxy(Ice::stringToIdentity("nestedClient")));
    adapter->add(new NestedI(self), Ice::stringToIdentity("nestedClient"));
    adapter->activate();

    cout << "Note: The maximum nesting level is (sz - 1) * 2, with sz\n"
	 << "being the number of threads in the server thread pool. if\n"
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
	catch(const Exception& ex)
	{
	    cerr << ex << endl;
	}
    }
    while(cin.good() && s != "x");

    return EXIT_SUCCESS;
}
