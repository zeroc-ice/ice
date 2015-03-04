// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/IceUtil.h>
#include <Ice/Ice.h>
#include <Session.h>

#ifdef _WIN32
const DWORD SIGHUP = CTRL_LOGOFF_EVENT;
#else
#   include <csignal>
#endif

using namespace std;
using namespace Demo;

class RefreshTask : public IceUtil::TimerTask
{
public:

    RefreshTask(const Ice::LoggerPtr& logger, const SessionPrx& session) :
        _logger(logger),
        _session(session)
    {
    }

    virtual void
    runTimerTask()
    {
        try
        {
            _session->refresh();
        }
        catch(const Ice::Exception& ex)
        {
            Ice::Warning warn(_logger);
            warn << "RefreshTask: " << ex;
        }
    }

private:

    const Ice::LoggerPtr _logger;
    const SessionPrx _session;
};

class SessionClient : public Ice::Application
{
public:

    SessionClient();
    virtual int run(int, char*[]);

private:

    void menu();
    void cleanup(bool);

    //
    // The interrupt callback and main can run concurrently with one
    // another so shared variables must be mutex protected.
    //
    IceUtil::Mutex _mutex;
    IceUtil::TimerPtr _timer;
    SessionPrx _session;
};

int
main(int argc, char* argv[])
{
    SessionClient app;
    return app.main(argc, argv, "config.client");
}

SessionClient::SessionClient() :
    //
    // Since this is an interactive demo we don't want any signal
    // handling.
    //
    Ice::Application(Ice::NoSignalHandling)
{
}

int
SessionClient::run(int argc, char* argv[])
{
    if(argc > 1)
    {
        cerr << appName() << ": too many arguments" << endl;
        return EXIT_FAILURE;
    }

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
        
        _timer = new IceUtil::Timer();
        _timer->scheduleRepeated(new RefreshTask(communicator()->getLogger(), _session), IceUtil::Time::seconds(5));
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
SessionClient::cleanup(bool destroy)
{
    IceUtil::Mutex::Lock sync(_mutex);

    //
    // The refresher thread must be terminated before destroy is
    // called, otherwise it might get ObjectNotExistException. refresh
    // is set to 0 so that if session->destroy() raises an exception
    // the thread will not be re-terminated and re-joined.
    //
    if(_timer)
    {
        _timer->destroy();
        _timer = 0;
    }
    
    if(destroy && _session)
    {
        _session->destroy();
    }
    _session = 0;
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
