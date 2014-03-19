// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <NestedI.h>

using namespace std;
using namespace Demo;


class NestedClient : public Ice::Application
{
public:

    NestedClient();
    virtual int run(int, char*[]);
};

int
main(int argc, char* argv[])
{
    NestedClient app;
    return app.main(argc, argv, "config.client");
}

NestedClient::NestedClient() :
    //
    // Since this is an interactive demo we don't want any signal
    // handling.
    //
    Ice::Application(Ice::NoSignalHandling)
{
}

int
NestedClient::run(int argc, char*[])
{
    if(argc > 1)
    {
        cerr << appName() << ": too many arguments" << endl;
        return EXIT_FAILURE;
    }

    NestedPrx nested = NestedPrx::checkedCast(communicator()->propertyToProxy("Nested.Proxy"));
    if(!nested)
    {
        cerr << appName() << ": invalid proxy" << endl;
        return EXIT_FAILURE;
    }

    Ice::ObjectAdapterPtr adapter = communicator()->createObjectAdapter("Nested.Client");
    NestedPrx self = NestedPrx::uncheckedCast(adapter->createProxy(communicator()->stringToIdentity("nestedClient")));
    NestedPtr servant = new NestedI(self);
    adapter->add(servant, communicator()->stringToIdentity("nestedClient"));
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
