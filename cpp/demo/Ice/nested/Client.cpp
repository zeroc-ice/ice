// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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

    virtual int run(int, char*[]);
    virtual void interruptCallback(int);
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
    //
    // Since this is an interactive demo we want the custom interrupt
    // callback to be called when the process is interrupted.
    //
    callbackOnInterrupt();

    NestedPrx nested = NestedPrx::checkedCast(communicator()->propertyToProxy("Nested.NestedServer"));
    if(!nested)
    {
        cerr << appName() << ": invalid proxy" << endl;
        return EXIT_FAILURE;
    }

    Ice::ObjectAdapterPtr adapter = communicator()->createObjectAdapter("Nested.Client");
    NestedPrx self = NestedPrx::uncheckedCast(adapter->createProxy(communicator()->stringToIdentity("nestedClient")));
    adapter->add(new NestedI(self), communicator()->stringToIdentity("nestedClient"));
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

void
NestedClient::interruptCallback(int)
{
/*
 * For this demo we won't destroy the communicator since it has to
 * wait for any outstanding invocations to complete which may take
 * some time if the nesting level is exceeded.
 *
    try
    {
        communicator()->destroy();
    }
    catch(const IceUtil::Exception& ex)
    {
        cerr << appName() << ": " << ex << endl;
    }
    catch(...)
    {
        cerr << appName() << ": unknown exception" << endl;
    }
*/
    exit(EXIT_SUCCESS);
}
