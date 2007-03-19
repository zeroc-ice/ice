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
#include <Glacier2/Router.h>
#include <Chat.h>

using namespace std;
using namespace Demo;

class SessionPingThread : public IceUtil::Thread, public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    SessionPingThread(const Glacier2::SessionPrx& session, long timeout) :
        _session(session),
        _timeout(IceUtil::Time::seconds(timeout)),
        _destroy(false)
    {
    }

    virtual void
    run()
    {
        Lock sync(*this);
        while(!_destroy)
        {
            timedWait(_timeout);
            if(_destroy)
            {
                break;
            }
            try
            {
                _session->ice_ping();
            }
            catch(const Ice::Exception&)
            {
                break;
            }
        }
    }

    void
    destroy()
    {
        Lock sync(*this);
        _destroy = true;
        notify();
    }

private:

    const Glacier2::SessionPrx _session;
    const IceUtil::Time _timeout;
    bool _destroy;
};
typedef IceUtil::Handle<SessionPingThread> SessionPingThreadPtr;

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

    virtual int
    run(int argc, char* argv[])
    {
        //
        // Since this is an interactive demo we want the custom interrupt
        // callback to be called when the process is interrupted.
        //
        callbackOnInterrupt();

        {
            IceUtil::Mutex::Lock sync(_mutex);
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
                session = ChatSessionPrx::uncheckedCast(_router->createSession(id, pw));
                break;
            }
            catch(const Glacier2::PermissionDeniedException& ex)
            {
                cout << "permission denied:\n" << ex.reason << endl;
            }
        }

        {
            IceUtil::Mutex::Lock sync(_mutex);
            _ping = new SessionPingThread(session, (long)_router->getSessionTimeout() / 2);
            _ping->start();
        }

        Ice::Identity callbackReceiverIdent;
        callbackReceiverIdent.name = "callbackReceiver";
        callbackReceiverIdent.category = _router->getCategoryForClient();

        Ice::ObjectAdapterPtr adapter = communicator()->createObjectAdapter("Chat.Client");
        ChatCallbackPrx callback = ChatCallbackPrx::uncheckedCast(
            adapter->add(new ChatCallbackI, callbackReceiverIdent));
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

    virtual void
    interruptCallback(int)
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

private:

    void
    cleanup()
    {
        IceUtil::Mutex::Lock sync(_mutex);
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
        if(_ping)
        {
            _ping->destroy();
            _ping->getThreadControl().join();
            _ping = 0;
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

    IceUtil::Mutex _mutex;
    Glacier2::RouterPrx _router;
    SessionPingThreadPtr _ping;
};

int
main(int argc, char* argv[])
{
    ChatClient app;
    return app.main(argc, argv, "config.client");
}
