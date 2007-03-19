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

class AMI_Hello_sayHelloI : public AMI_Hello_sayHello
{
public:

    virtual void ice_response()
    {
    }

    virtual void ice_exception(const Ice::Exception& ex)
    {
        cerr << "sayHello AMI call failed:\n" << ex << endl;
    }
};

class AsyncClient : public Ice::Application
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
    AsyncClient app;
    return app.main(argc, argv, "config.client");
}

int
AsyncClient::run(int argc, char* argv[])
{
    callbackOnInterrupt();

    HelloPrx hello = HelloPrx::checkedCast(communicator()->propertyToProxy("Hello.Proxy"));
    if(!hello)
    {
        cerr << argv[0] << ": invalid proxy" << endl;
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
            if(c == 'i')
            {
                hello->sayHello(0);
            }
            else if(c == 'd')
            {
                hello->sayHello_async(new AMI_Hello_sayHelloI, 5000);
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

void
AsyncClient::interruptCallback(int)
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
AsyncClient::menu()
{
    cout <<
        "usage:\n"
        "i: send immediate greeting\n"
        "d: send delayed greeting\n"
        "s: shutdown server\n"
        "x: exit\n"
        "?: help\n";
}
