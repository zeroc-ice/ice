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
    if (ref.empty())
    {
	cerr << argv[0] << ": property `" << refProperty << "' not set" << endl;
	return EXIT_FAILURE;
    }

    ObjectPrx base = communicator()->stringToProxy(ref);
    NestedPrx nested = NestedPrx::checkedCast(base);
    if (!nested)
    {
	cerr << argv[0] << ": invalid object reference" << endl;
	return EXIT_FAILURE;
    }

    ObjectAdapterPtr adapter = communicator()->createObjectAdapter("NestedClientAdapter");
    NestedPrx self = NestedPrx::uncheckedCast(adapter->createProxy("nestedClient"));
    adapter->add(new NestedI(self), "nestedClient");
    adapter->activate();

    cout << "Note: The maximum nesting level is (sz - 1) * 2, with sz\n"
	 << "being the number of threads in the thread pool. if you\n"
	 << "specify a value higher than that, the application will block\n"
	 << "or timeout.\n"
	 << endl;

    string s;
    do
    {
	try
	{
	    cout << "enter nesting level or 'x' for exit: ";
	    cin >> s;
	    int level = atoi(s.c_str());
	    if (level > 0)
	    {
		nested->nested(level, self);
	    }
	}
	catch(const LocalException& ex)
	{
	    cerr << ex << endl;
	}
    }
    while (cin.good() && s != "x");

    return EXIT_SUCCESS;
}
