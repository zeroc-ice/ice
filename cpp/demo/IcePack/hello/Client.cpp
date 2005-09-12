// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Application.h>
#include <IcePack/Query.h>
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
	"c: create a hello object\n"
	"d: destroy the current hello object\n"
	"s: set the current hello object\n"
	"r: set the current hello object to a random hello object\n"
	"S: show the name of the current hello object\n"
	"t: send greeting\n"
	"x: exit\n"
	"?: help\n";
}

int
HelloClient::run(int argc, char* argv[])
{
    Ice::PropertiesPtr properties = communicator()->getProperties();

    IcePack::QueryPrx query = IcePack::QueryPrx::checkedCast(communicator()->stringToProxy("IcePack/Query"));

    //
    // Get an object implementing the HelloFactory interface.
    //
    HelloFactoryPrx factory = HelloFactoryPrx::checkedCast(query->findObjectByType("::Demo::HelloFactory"));

    //
    // By default we create a Hello object named 'Foo'.
    //
    HelloPrx hello;
    try
    {
	hello = factory->find("Foo");
    }
    catch(const NameNotExistException&)
    {
	hello = factory->create("Foo");
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
	    else if(c == 'c')
	    {
		string name;

		cout << "name: ";
		cin >> name;
		
		if(!name.empty())
		{
		    try
		    {
			hello = factory->find(name);
			cout << "Hello object named '" << name << "' already exists" << endl;
		    }
		    catch(const NameNotExistException&)
		    {
			factory = HelloFactoryPrx::checkedCast(query->findObjectByType("::Demo::HelloFactory"));
			hello = factory->create(name);
		    }
		}
	    }
	    else if(c == 'd')
	    {
		if(Ice::identityToString(hello->ice_getIdentity()) == "Foo")
		{
		    cout << "Can't delete the default Hello object named 'Foo'" << endl;
		}
		else
		{
		    hello->destroy();

		    try
		    {
			hello = factory->find("Foo");
		    }
		    catch(const NameNotExistException&)
		    {
			hello = factory->create("Foo");
		    }
		}
	    }
	    else if(c == 's')
	    {
		string name;

		cout << "name: ";
		cin >> name;
		
		try
		{
		    hello = HelloPrx::checkedCast(factory->find(name));
		}
		catch(const NameNotExistException&)
		{
		    cout << "This name doesn't exist" << endl;
		}
	    }
	    else if(c == 'r')
	    {
		hello = HelloPrx::checkedCast(query->findObjectByType("::Demo::Hello"));
	    }
	    else if(c == 'S')
	    {
		cout << Ice::identityToString(hello->ice_getIdentity()) << endl;
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
