// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/IceUtil.h>
#include <Ice/Ice.h>
#include <Echo.h>
#include <StringConverterI.h>

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
    const string proxyProperty = "Echo.Proxy";
    EchoPrx echo1 = EchoPrx::checkedCast(communicator1->propertyToProxy(proxyProperty));
    if(!echo1)
    {
        cerr << argv[0] << ": invalid proxy" << endl;
        return EXIT_FAILURE;
    }

    EchoPrx echo2 = EchoPrx::checkedCast(communicator2->propertyToProxy(proxyProperty));
    if(!echo2)
    {
        cerr << argv[0] << ": invalid proxy" << endl;
        return EXIT_FAILURE;
    }

    menu();

    string greeting = "Bonne journ\351e";

    char c;
    do
    {
        try
        {
            cout << "==> ";
            cin >> c;
            if(c == 't')
            {
                string ret = echo1->echoString(greeting);
                cout << "Received (LATIN-1): \"" << IceUtil::escapeString(ret, "") << '\"' << endl;
            }
            else if(c == 'u')
            {
                string ret = echo2->echoString(greeting);
                cout << "Received (LATIN-1): \"" << IceUtil::escapeString(ret, "") << '\"' << endl;
            }
            else if(c == 's')
            {
                echo1->shutdown();
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
        initData.properties = Ice::createProperties(initData.stringConverter);
        initData.properties->load("config.client");
        communicator1 = Ice::initialize(argc, argv, initData);

        Ice::InitializationData initData2;
        initData2.properties = Ice::createProperties();
        initData2.properties->load("config.client");
        communicator2 = Ice::initialize(argc, argv, initData2);

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

