// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <Callback.h>

using namespace std;
using namespace Demo;

class CallbackReceiverI : public CallbackReceiver
{
public:

    virtual void callback(const Ice::Current&)
    {
#ifdef __xlC__
        //
        // The xlC compiler synchronizes cin and cout; to see the messages
        // while accepting input through cin, we have to print the messages
        // with printf
        //
        printf("received callback\n");
        fflush(0);
#else
        cout << "received callback" << endl;
#endif
    }
};

class CallbackClient : public Ice::Application
{
public:

    CallbackClient();

    virtual int run(int, char*[]);

private:

    void menu();
};

int
main(int argc, char* argv[])
{
    CallbackClient app;
    return app.main(argc, argv, "config.client");
}

CallbackClient::CallbackClient() :
    //
    // Since this is an interactive demo we don't want any signal
    // handling.
    //
    Ice::Application(Ice::NoSignalHandling)
{
}

int
CallbackClient::run(int argc, char*[])
{
    if(argc > 1)
    {
        cerr << appName() << ": too many arguments" << endl;
        return EXIT_FAILURE;
    }

    CallbackSenderPrx sender = CallbackSenderPrx::checkedCast(
        communicator()->propertyToProxy("CallbackSender.Proxy")->ice_twoway()->ice_timeout(-1)->ice_secure(false));
    if(!sender)
    {
        cerr << appName() << ": invalid proxy" << endl;
        return EXIT_FAILURE;
    }
    
    Ice::ObjectAdapterPtr adapter = communicator()->createObjectAdapter("Callback.Client");
    CallbackReceiverPtr cr = new CallbackReceiverI;
    adapter->add(cr, communicator()->stringToIdentity("callbackReceiver"));
    adapter->activate();

    CallbackReceiverPrx receiver = CallbackReceiverPrx::uncheckedCast(
        adapter->createProxy(communicator()->stringToIdentity("callbackReceiver")));

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
                sender->initiateCallback(receiver);
            }
            else if(c == 's')
            {
                sender->shutdown();
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
CallbackClient::menu()
{
    cout <<
        "usage:\n"
        "t: send callback\n"
        "s: shutdown server\n"
        "x: exit\n"
        "?: help\n";
}
