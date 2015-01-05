// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <Context.h>

using namespace std;
using namespace Demo;

class ContextClient : public Ice::Application
{
public:

    ContextClient();

    virtual int run(int, char*[]);

private:

    void menu();
};

int
main(int argc, char* argv[])
{
    ContextClient app;
    return app.main(argc, argv, "config.client");
}

ContextClient::ContextClient() :
    //
    // Since this is an interactive demo we don't want any signal
    // handling.
    //
    Ice::Application(Ice::NoSignalHandling)
{
}

int
ContextClient::run(int argc, char* argv[])
{
    if(argc > 1)
    {
        cerr << appName() << ": too many arguments" << endl;
        return EXIT_FAILURE;
    }

    ContextPrx proxy = ContextPrx::checkedCast(communicator()->propertyToProxy("Context.Proxy"));
    if(!proxy)
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
            if(c == '1')
            {
                proxy->call();
            }
            else if(c == '2')
            {
                Ice::Context ctx;
                ctx["type"] = "Explicit";
                proxy->call(ctx);
            }
            else if(c == '3')
            {
                Ice::Context ctx;
                ctx["type"] = "Per-Proxy";
                ContextPrx proxy2 = proxy->ice_context(ctx);
                proxy2->call();
            }
            else if(c == '4')
            {
                Ice::ImplicitContextPtr ic = communicator()->getImplicitContext();
                Ice::Context ctx;
                ctx["type"] = "Implicit";
                ic->setContext(ctx);
                proxy->call();
                ic->setContext(Ice::Context());
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
ContextClient::menu()
{
    cout <<
        "usage:\n"
        "1: use no request context\n"
        "2: use explicit request context\n"
        "3: use per-proxy request context\n"
        "4: use implicit request context\n"
        "s: shutdown server\n"
        "x: exit\n"
        "?: help\n";
}
