// **********************************************************************
//
// Copyright (c) 2002
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IcePack/Query.h>
#include <Hello.h>

using namespace std;

void
menu()
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
run(int argc, char* argv[], const Ice::CommunicatorPtr& communicator)
{
    Ice::PropertiesPtr properties = communicator->getProperties();

    IcePack::QueryPrx query = IcePack::QueryPrx::checkedCast(communicator->stringToProxy("IcePack/Query"));

    //
    // Get an object implementing the HelloFactory interface.
    //
    HelloFactoryPrx factory = HelloFactoryPrx::checkedCast(query->findObjectByType("::HelloFactory"));

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

		cout << "name: " << ends;
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
			factory = HelloFactoryPrx::checkedCast(query->findObjectByType("::HelloFactory"));
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

		cout << "name: " << ends;
		cin >> name;
		
		try
		{
		    hello = HelloPrx::checkedCast(factory->find(name));
		}
		catch(const NameNotExistException& ex)
		{
		    cout << "This name doesn't exist" << endl;
		}
	    }
	    else if(c == 'r')
	    {
		hello = HelloPrx::checkedCast(query->findObjectByType("::Hello"));
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

int
main(int argc, char* argv[])
{
    int status;
    Ice::CommunicatorPtr communicator;

    try
    {
	Ice::PropertiesPtr properties = Ice::createProperties(argc, argv);
        properties->load("config");
	communicator = Ice::initializeWithProperties(argc, argv, properties);
	status = run(argc, argv, communicator);
    }
    catch(const Ice::Exception& ex)
    {
	cerr << ex << endl;
	status = EXIT_FAILURE;
    }

    if(communicator)
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
