// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IceGrid/Query.h>
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

void
HelloClient::menu()
{
    cout <<
	"usage:\n"
	"t: send greeting\n"
	"s: shutdown server\n"
	"x: exit\n"
	"?: help\n";
}

int
HelloClient::run(int argc, char* argv[])
{
    //
    // First we try to connect to the object with the `hello'
    // identity. If it's not registered with the registry, we 
    // search for an object with the ::Demo::Hello type.
    //
    HelloPrx hello;
    try
    {
	hello = HelloPrx::checkedCast(communicator()->stringToProxy("hello"));
    }
    catch(const Ice::NotRegisteredException&)
    {
	string proxy = communicator()->getProperties()->getProperty("IceGrid.InstanceName") + "/Query";
	IceGrid::QueryPrx query = IceGrid::QueryPrx::checkedCast(communicator()->stringToProxy(proxy));
	hello = HelloPrx::checkedCast(query->findObjectByType("::Demo::Hello"));
    }
    if(!hello)
    {
	cerr << argv[0] << ": couldn't find a `::Demo::Hello' object." << endl;
	return EXIT_FAILURE;
    }
    
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
		hello->sayHello();
	    }
	    else if(c == 's')
	    {
		hello->shutdown();
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

