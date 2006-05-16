// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Application.h>
#include <Hello.h>
#include <StringConverterI.h>
#include <IceUtil/StringUtil.h>

using namespace std;
using namespace Demo;

void
menu()
{
    cout <<
	"usage:\n"
	"t: send greeting with conversion\n"
	"u: send greeting without conversion\n"
	"s: shutdown server\n"
	"x: exit\n"
	"?: help\n";
}

int
run(int argc, char* argv[], const Ice::CommunicatorPtr& communicator1, const Ice::CommunicatorPtr& communicator2)
{
    Ice::PropertiesPtr properties = communicator1->getProperties();
    const char* proxyProperty = "Hello.Proxy";
    string proxy = properties->getProperty(proxyProperty);
    if(proxy.empty())
    {
	cerr << argv[0] << ": property `" << proxyProperty << "' not set" << endl;
	return EXIT_FAILURE;
    }

    HelloPrx hello1 = HelloPrx::checkedCast(communicator1->stringToProxy(proxy));
    if(!hello1)
    {
	cerr << argv[0] << ": invalid proxy" << endl;
	return EXIT_FAILURE;
    }

    HelloPrx hello2 = HelloPrx::checkedCast(communicator2->stringToProxy(proxy));
    if(!hello2)
    {
	cerr << argv[0] << ": invalid proxy" << endl;
	return EXIT_FAILURE;
    }

    menu();

    string uber = "\374ber";

    char c;
    do
    {
	try
	{
	    cout << "==> ";
	    cin >> c;
	    if(c == 't')
	    {
		string ret = hello1->sayHello(uber);
		cout << "Received (LATIN-1): \"" << IceUtil::escapeString(ret, "") << '\"' << endl;
	    }
	    else if(c == 'u')
	    {
		string ret = hello2->sayHello(uber);
		cout << "Received (LATIN-1): \"" << IceUtil::escapeString(ret, "") << '\"' << endl;
	    }
	    else if(c == 's')
	    {
		hello1->shutdown();
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
    Ice::CommunicatorPtr communicator1;
    Ice::CommunicatorPtr communicator2;

    try
    {
        //
	// Create two communicators, one with string converter configured
	// and one without.
	//
        Ice::InitializationData initData;
        initData.stringConverter = new StringConverterI();
	initData.properties = Ice::createProperties();
	initData.properties->load("config.client");
        communicator1 = Ice::initialize(argc, argv, initData);

        communicator2 = Ice::initialize(argc, argv);

        status = run(argc, argv, communicator1, communicator2);
    }
    catch(const Ice::Exception& ex)
    {
        cerr << ex << endl;
        status = EXIT_FAILURE;
    }

    if(communicator1)
    {
        try
        {
            communicator1->destroy();
        }
        catch(const Ice::Exception& ex)
        {
            cerr << ex << endl;
            status = EXIT_FAILURE;
        }
    }

    if(communicator2)
    {
        try
        {
            communicator2->destroy();
        }
        catch(const Ice::Exception& ex)
        {
            cerr << ex << endl;
            status = EXIT_FAILURE;
        }
    }

    return status;
}

