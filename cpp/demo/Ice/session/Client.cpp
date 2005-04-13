// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IceUtil/Thread.h>
#include <HelloSession.h>

using namespace std;
using namespace Demo;

//
// This thread pings the session object with the given timeout frequency.
//
class SessionRefreshThread : public IceUtil::Thread, public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    SessionRefreshThread(const Ice::LoggerPtr& logger, const SessionPrx& session,
			 const IceUtil::Time& timeout) :
	_logger(logger),
	_session(session),
	_destroy(false),
	_timeout(timeout)
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
    	    //
	    // If the refresh fails we're done.
	    //
	    try
	    {
		_session->refresh();
	    }
	    catch(const Ice::Exception& ex)
	    {
		Ice::Warning warn(_logger);
		warn << "SessionRefreshThread: " << ex;
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

    const Ice::LoggerPtr _logger;
    const SessionPrx _session;
    bool _destroy;
    const IceUtil::Time _timeout;
};
typedef IceUtil::Handle<SessionRefreshThread> SessionRefreshThreadPtr;

void
menu()
{
    cout <<
	"usage:\n"
	"h: send greeting\n"
	"s: shutdown server\n"
	"x: exit\n"
	"?: help\n";
}

int
run(int argc, char* argv[], const Ice::CommunicatorPtr& communicator)
{
    Ice::PropertiesPtr properties = communicator->getProperties();
    const char* proxyProperty = "SessionManager.Proxy";
    string proxy = properties->getProperty(proxyProperty);
    if(proxy.empty())
    {
	cerr << argv[0] << ": property `" << proxyProperty << "' not set" << endl;
	return EXIT_FAILURE;
    }

    //
    // Get the session manager object, and create a session.
    //
    Ice::ObjectPrx base = communicator->stringToProxy(proxy);
    SessionManagerPrx manager = SessionManagerPrx::checkedCast(base);
    if(!manager)
    {
	cerr << argv[0] << ": invalid proxy" << endl;
	return EXIT_FAILURE;
    }

    HelloSessionPrx hello = HelloSessionPrx::uncheckedCast(manager->create());
    if(!hello)
    {
	cerr << argv[0] << ": invalid proxy" << endl;
	return EXIT_FAILURE;
    }

    //
    // Create a thread to ping the object at regular intervals.
    //
    SessionRefreshThreadPtr refresh = new SessionRefreshThread(
	communicator->getLogger(), hello, IceUtil::Time::seconds(5));
    refresh->start();

    menu();

    try
    {
	do
	{
	    cout << "==> ";
	    char c;
	    cin >> c;
	    if(c == 'h')
	    {
		hello->sayHello();
	    }
	    else if(c == 's')
	    {
		manager->shutdown();
	    }
	    else if(c == 'x')
	    {
		break;
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
	while(cin.good());

    	//
	// Destroy the session before we finish.
	//
	hello->destroy();
    }
    catch(const Ice::Exception& ex)
    {
	cerr << ex << endl;
    }

    //
    // Destroy the ping thread, and join with it to ensure that it
    // actually completes.
    //
    refresh->destroy();
    refresh->getThreadControl().join();

    return EXIT_SUCCESS;
}

int
main(int argc, char* argv[])
{
    int status;
    Ice::CommunicatorPtr communicator;

    try
    {
	Ice::PropertiesPtr properties = Ice::createProperties();
        properties->load("config");
	communicator = Ice::initializeWithProperties(argc, argv, properties);
	status = run(argc, argv, communicator);
    }
    catch(const Ice::Exception& ex)
    {
	cerr << ex << endl;
	status = EXIT_FAILURE;
    }

    if(communicator)
    {
	try
	{
	    communicator->destroy();
	}
	catch(const Ice::Exception& ex)
	{
	    cerr << ex << endl;
	    status = EXIT_FAILURE;
	}
    }

    return status;
}
