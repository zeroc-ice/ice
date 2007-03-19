// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <Glacier2/Router.h>
#include <CallbackI.h>

using namespace std;
using namespace Demo;

class CallbackClient : public Ice::Application
{
public:

    virtual int run(int, char*[]);
    virtual void interruptCallback(int);
};

int
main(int argc, char* argv[])
{
    CallbackClient app;
    return app.main(argc, argv, "config.client");
}

void
menu()
{
    cout <<
        "usage:\n"
        "t: invoke callback as twoway\n"
        "o: invoke callback as oneway\n"
        "O: invoke callback as batch oneway\n"
        "f: flush all batch requests\n"
        "v: set/reset override context field\n"
        "F: set/reset fake category\n"
        "s: shutdown server\n"
        "x: exit\n"
        "?: help\n";
}

int
CallbackClient::run(int argc, char* argv[])
{
    //
    // Since this is an interactive demo we want the custom interrupt
    // callback to be called when the process is interrupted.
    //
    callbackOnInterrupt();

    Ice::RouterPrx defaultRouter = communicator()->getDefaultRouter();
    if(!defaultRouter)
    {
        cerr << argv[0] << ": no default router set" << endl;
        return EXIT_FAILURE;
    }

    Glacier2::RouterPrx router = Glacier2::RouterPrx::checkedCast(defaultRouter);
    if(!router)
    {
        cerr << argv[0] << ": configured router is not a Glacier2 router" << endl;
        return EXIT_FAILURE;
    }

    while(true)
    {
        cout << "This demo accepts any user-id / password combination.\n";

        string id;
        cout << "user id: " << flush;
        cin >> id;

        string pw;
        cout << "password: " << flush;
        cin >> pw;
    
        try
        {
            router->createSession(id, pw);
            break;
        }
        catch(const Glacier2::PermissionDeniedException& ex)
        {
            cout << "permission denied:\n" << ex.reason << endl;
        }
    }

    Ice::Identity callbackReceiverIdent;
    callbackReceiverIdent.name = "callbackReceiver";
    callbackReceiverIdent.category = router->getCategoryForClient();
    Ice::Identity callbackReceiverFakeIdent;
    callbackReceiverFakeIdent.name = "callbackReceiver";
    callbackReceiverFakeIdent.category = "fake";

    Ice::ObjectPrx base = communicator()->propertyToProxy("Callback.Proxy");
    CallbackPrx twoway = CallbackPrx::checkedCast(base);
    CallbackPrx oneway = CallbackPrx::uncheckedCast(twoway->ice_oneway());
    CallbackPrx batchOneway = CallbackPrx::uncheckedCast(twoway->ice_batchOneway());
    
    Ice::ObjectAdapterPtr adapter = communicator()->createObjectAdapter("Callback.Client");
    adapter->add(new CallbackReceiverI, callbackReceiverIdent);
    adapter->add(new CallbackReceiverI, callbackReceiverFakeIdent); // Should never be called for the fake identity. 
    adapter->activate();

    CallbackReceiverPrx twowayR = CallbackReceiverPrx::uncheckedCast(adapter->createProxy(callbackReceiverIdent));
    CallbackReceiverPrx onewayR = CallbackReceiverPrx::uncheckedCast(twowayR->ice_oneway());

    string override;
    bool fake = false;

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
                Ice::Context context;
                context["_fwd"] = "t";
                if(!override.empty())
                {
                    context["_ovrd"] = override;
                }
                twoway->initiateCallback(twowayR, context);
            }
            else if(c == 'o')
            {
                Ice::Context context;
                context["_fwd"] = "o";
                if(!override.empty())
                {
                    context["_ovrd"] = override;
                }
                oneway->initiateCallback(onewayR, context);
            }
            else if(c == 'O')
            {
                Ice::Context context;
                context["_fwd"] = "O";
                if(!override.empty())
                {
                    context["_ovrd"] = override;
                }
                batchOneway->initiateCallback(onewayR, context);
            }
            else if(c == 'f')
            {
                communicator()->flushBatchRequests();
            }
            else if(c == 'v')
            {
                if(override.empty())
                {
                    override = "some_value";
                    cout << "override context field is now `" << override << "'" << endl;
                }
                else
                {
                    override.clear();
                    cout << "override context field is empty" << endl;
                }
            }
            else if(c == 'F')
            {
                fake = !fake;

                if(fake)
                {
                    twowayR = CallbackReceiverPrx::uncheckedCast(twowayR->ice_identity(callbackReceiverFakeIdent));
                    onewayR = CallbackReceiverPrx::uncheckedCast(onewayR->ice_identity(callbackReceiverFakeIdent));
                }
                else
                {
                    twowayR = CallbackReceiverPrx::uncheckedCast(twowayR->ice_identity(callbackReceiverIdent));
                    onewayR = CallbackReceiverPrx::uncheckedCast(onewayR->ice_identity(callbackReceiverIdent));
                }
                
                cout << "callback receiver identity: " << communicator()->identityToString(twowayR->ice_getIdentity())
                     << endl;
            }
            else if(c == 's')
            {
                twoway->shutdown();
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
CallbackClient::interruptCallback(int)
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
