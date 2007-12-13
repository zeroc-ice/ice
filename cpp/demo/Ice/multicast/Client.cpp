// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <Hello.h>

using namespace std;
using namespace Demo;

class MulticastClient : public Ice::Application
{
public:

    virtual int run(int, char*[]);
    virtual void interruptCallback(int);

private:

    void menu();
};

int
main(int argc, char* argv[])
{
    MulticastClient app;
    return app.main(argc, argv, "config.client");
}

int
MulticastClient::run(int argc, char* argv[])
{
    if(argc > 1)
    {
        cerr << appName() << ": too many arguments" << endl;
        return EXIT_FAILURE;
    }

    //
    // Since this is an interactive demo we want the custom interrupt
    // callback to be called when the process is interrupted.
    //
    callbackOnInterrupt();

    HelloPrx proxy = HelloPrx::uncheckedCast(communicator()->propertyToProxy("Hello.Proxy")->ice_datagram());

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
                proxy->sayHello();
            }
            else if(c == 's')
            {
                proxy->shutdown();
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

void
MulticastClient::interruptCallback(int)
{
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
    exit(EXIT_SUCCESS);
}

void
MulticastClient::menu()
{
    cout <<
        "usage:\n"
        "t: send multicast message\n"
        "s: shutdown server\n"
        "x: exit\n"
        "?: help\n";
}
