// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <Hello.h>

using namespace std;
using namespace Demo;

class Callback : public IceUtil::Shared
{
public:

    void response()
    {
    }

    void exception(const Ice::Exception& ex)
    {
        cerr << "sayHello AMI call failed:\n" << ex << endl;
    }
};
typedef IceUtil::Handle<Callback> CallbackPtr;

class AsyncClient : public Ice::Application
{
public:

    AsyncClient();

    virtual int run(int, char*[]);

private:

    void exception(const Ice::Exception&);
    void menu();
};

AsyncClient::AsyncClient() :
    //
    // Since this is an interactive demo we don't want any signal
    // handling.
    //
    Ice::Application(Ice::NoSignalHandling)
{
}

int
main(int argc, char* argv[])
{
    AsyncClient app;
    return app.main(argc, argv, "config.client");
}

int
AsyncClient::run(int argc, char* argv[])
{
    if(argc > 1)
    {
        cerr << appName() << ": too many arguments" << endl;
        return EXIT_FAILURE;
    }

    HelloPrx hello = HelloPrx::checkedCast(communicator()->propertyToProxy("Hello.Proxy"));
    if(!hello)
    {
        cerr << argv[0] << ": invalid proxy" << endl;
        return EXIT_FAILURE;
    }

    menu();

    CallbackPtr cb = new Callback();

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
                hello->begin_sayHello(5000, newCallback_Hello_sayHello(cb, &Callback::response, &Callback::exception));
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

