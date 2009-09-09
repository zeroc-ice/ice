// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/IceUtil.h>
#include <Ice/Ice.h>
#include <Glacier2/Glacier2.h>
#include <Chat.h>

using namespace std;
using namespace Demo;

class PingTask : public IceUtil::TimerTask
{
public:

    PingTask(const Glacier2::SessionPrx& session) :
        _session(session)
    {
    }

    virtual void runTimerTask()
    {
        try
        {
            _session->ice_ping();
        }
        catch(const Ice::Exception&)
        {
            // Ignore
        }
    }

private:

    const Glacier2::SessionPrx _session;
};

class ChatCallbackI : public ChatCallback
{
public:

    virtual void
    message(const string& data, const Ice::Current&)
    {
#ifdef __xlC__

        //
        // The xlC compiler synchronizes cin and cout; to see the messages
        // while accepting input through cin, we have to print the messages
        // with printf
        //

        printf("%s\n", data.c_str());
        fflush(0);
#else
        cout << data << endl;
#endif
    }
};

class ChatClient : public Ice::Application
{
public:

    ChatClient() :
        //
        // Since this is an interactive demo we don't want any signal
        // handling.
        //
        Ice::Application(Ice::NoSignalHandling)
    {
    }

    virtual int
    run(int argc, char* argv[])
    {
        if(argc > 1)
        {
            cerr << appName() << ": too many arguments" << endl;
            return EXIT_FAILURE;
        }

        Ice::RouterPrx defaultRouter = communicator()->getDefaultRouter();
        if(!defaultRouter)
        {
            cerr << argv[0] << ": no default router set" << endl;
            return EXIT_FAILURE;
        }

        _router = Glacier2::RouterPrx::checkedCast(defaultRouter);
        if(!_router)
        {
            cerr << argv[0] << ": configured router is not a Glacier2 router" << endl;
            return EXIT_FAILURE;
        }

        ChatSessionPrx session;
        while(true)
        {
            cout << "This demo accepts any user-id / password combination.\n";

            string id;
            cout << "user id: " << flush;
            getline(cin, id);
            id = trim(id);

            string pw;
            cout << "password: " << flush;
            getline(cin, pw);
            pw = trim(pw);

            try
            {
#if defined(__BCPLUSPLUS__) && (__BCPLUSPLUS__ >= 0x0600)
                IceUtil::DummyBCC dummy;
#endif
                session = ChatSessionPrx::uncheckedCast(_router->createSession(id, pw));
                break;
            }
            catch(const Glacier2::PermissionDeniedException& ex)
            {
                cout << "permission denied:\n" << ex.reason << endl;
            }
        }

        _timer = new IceUtil::Timer();
        _timer->scheduleRepeated(new PingTask(session), IceUtil::Time::milliSeconds(
                                    _router->getSessionTimeout() * 500));

        Ice::Identity callbackReceiverIdent;
        callbackReceiverIdent.name = "callbackReceiver";
        callbackReceiverIdent.category = _router->getCategoryForClient();

        Ice::ObjectAdapterPtr adapter = communicator()->createObjectAdapterWithRouter("Chat.Client", defaultRouter);
        ChatCallbackPtr cb = new ChatCallbackI;
        ChatCallbackPrx callback = ChatCallbackPrx::uncheckedCast(
            adapter->add(cb, callbackReceiverIdent));
        adapter->activate();

        session->setCallback(callback);

        menu();

        try
        {
            do
            {
                string s;
                cout << "==> ";
                getline(cin, s);
                s = trim(s);
                if(!s.empty())
                {
                    if(s[0] == '/')
                    {
                        if(s == "/quit")
                        {
                            break;
                        }
                        menu();
                    }
                    else
                    {
                        session->say(s);
                    }
                }
            }
            while(cin.good());

            cleanup();
        }
        catch(const Ice::Exception& ex)
        {
            cerr << ex << endl;
            cleanup();

            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    }

private:

    void
    cleanup()
    {
        //
        // Destroy the timer before the router session is destroyed,
        // otherwise it might get a spurious ObjectNotExistException.
        //
        if(_timer)
        {
            _timer->destroy();
            _timer = 0;
        }

        if(_router)
        {
            try
            {
                _router->destroySession();
            }
            catch(const Ice::ConnectionLostException&)
            {
                //
                // Expected: the router closed the connection.
                //
            }
            _router = 0;
        }
    }

    void
    menu()
    {
        cout << "enter /quit to exit." << endl;
    }

    string
    trim(const string& s)
    {
        static const string delims = "\t\r\n ";
        string::size_type last = s.find_last_not_of(delims);
        if(last != string::npos)
        {
            return s.substr(s.find_first_not_of(delims), last+1);
        }
        return s;
    }

    Glacier2::RouterPrx _router;
    IceUtil::TimerPtr _timer;
};

int
main(int argc, char* argv[])
{
    ChatClient app;
    return app.main(argc, argv, "config.client");
}
