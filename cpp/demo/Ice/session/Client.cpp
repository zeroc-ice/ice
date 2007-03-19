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
#include <Session.h>

using namespace std;
using namespace Demo;

class SessionRefreshThread : public IceUtil::Thread, public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    SessionRefreshThread(const Ice::LoggerPtr& logger, const IceUtil::Time& timeout, const SessionPrx& session) :
        _logger(logger),
        _session(session),
        _timeout(timeout),
        _terminated(false)
    {
    }

    virtual void
    run()
    {
        Lock sync(*this);
        while(!_terminated)
        {
            timedWait(_timeout);
            if(!_terminated)
            {
                try
                {
                    _session->refresh();
                }
                catch(const Ice::Exception& ex)
                {
                    Ice::Warning warn(_logger);
                    warn << "SessionRefreshThread: " << ex;
                    _terminated = true;
                }
            }
        }
    }

    void
    terminate()
    {
        Lock sync(*this);
        _terminated = true;
        notify();
    }

private:

    const Ice::LoggerPtr _logger;
    const SessionPrx _session;
    const IceUtil::Time _timeout;
    bool _terminated;
};

typedef IceUtil::Handle<SessionRefreshThread> SessionRefreshThreadPtr;

class SessionClient : public Ice::Application
{
public:

    virtual int run(int, char*[]);
    virtual void interruptCallback(int);

private:

    void menu();
    void cleanup(bool);

    //
    // The interrupt callback and main can run concurrently with one
    // another so shared variables must be mutex protected.
    //
    IceUtil::Mutex _mutex;
    SessionRefreshThreadPtr _refresh;
    SessionPrx _session;
};

int
main(int argc, char* argv[])
{
    SessionClient app;
    return app.main(argc, argv, "config.client");
}

int
SessionClient::run(int argc, char* argv[])
{
    //
    // Since this is an interactive demo we want the custom interrupt
    // callback to be called when the process is interrupted.
    //
    callbackOnInterrupt();

    string name;
    cout << "Please enter your name ==> ";
    cin >> name;
    if(!cin.good())
    {
        return EXIT_FAILURE;
    }

    Ice::ObjectPrx base = communicator()->propertyToProxy("SessionFactory.Proxy");
    SessionFactoryPrx factory = SessionFactoryPrx::checkedCast(base);
    if(!factory)
    {
        cerr << argv[0] << ": invalid proxy" << endl;
        return EXIT_FAILURE;
    }

    {
        IceUtil::Mutex::Lock sync(_mutex);
        _session = factory->create(name);
        
        _refresh = new SessionRefreshThread(
            communicator()->getLogger(), IceUtil::Time::seconds(5), _session);
        _refresh->start();
    }

    vector<HelloPrx> hellos;

    menu();

    try
    {
        bool destroy = true;
        bool shutdown = false;
        while(true)
        {
            cout << "==> ";
            char c;
            cin >> c;
            if(!cin.good())
            {
                break;
            }
            if(isdigit(c))
            {
                string s;
                s += c;
                vector<HelloPrx>::size_type index = atoi(s.c_str());
                if(index < hellos.size())
                {
                    hellos[index]->sayHello();
                }
                else
                {
                    cout << "Index is too high. " << hellos.size() << " hello objects exist so far.\n"
                         << "Use `c' to create a new hello object." << endl;
                }
            }
            else if(c == 'c')
            {
                hellos.push_back(_session->createHello());
                cout << "Created hello object " << hellos.size() - 1 << endl;
            }
            else if(c == 's')
            {
                destroy = false;
                shutdown = true;
                break;
            }
            else if(c == 'x')
            {
                break;
            }
            else if(c == 't')
            {
                destroy = false;
                break;
            }
            else if(c == '?')
            {
                menu();
            }
            else
            {
                cout << "Unknown command `" << c << "'." << endl;
                menu();
            }
        }

        //
        // The refresher thread must be terminated before destroy is
        // called, otherwise it might get ObjectNotExistException. refresh
        // is set to 0 so that if session->destroy() raises an exception
        // the thread will not be re-terminated and re-joined.
        //
        cleanup(destroy);
        if(shutdown)
        {
            factory->shutdown();
        }
    }
    catch(...)
    {
        try
        {
            cleanup(true);
        }
        catch(...)
        {
        }
        throw;
    }

    return EXIT_SUCCESS;
}

void
SessionClient::interruptCallback(int)
{
    //
    // Terminate the refresh thread, destroy the session and then
    // destroy the communicator, followed by an exit. We have to call
    // exit because main may be blocked in a cin >> s call which
    // cannot be interrupted portably.
    //
    cleanup(true);

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
SessionClient::cleanup(bool destroy)
{
    IceUtil::Mutex::Lock sync(_mutex);
    if(_refresh)
    {
        _refresh->terminate();
        _refresh->getThreadControl().join();
        _refresh = 0;
    }
    
    if(destroy && _session)
    {
        _session->destroy();
        _session = 0;
    }
}

void
SessionClient::menu()
{
    cout <<
        "usage:\n"
        "c:     create a new per-client hello object\n"
        "0-9:   send a greeting to a hello object\n"
        "s:     shutdown the server and exit\n"
        "x:     exit\n"
        "t:     exit without destroying the session\n"
        "?:     help\n";
}
