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
#include <Session.h>

using namespace std;
using namespace Demo;

class SessionRefreshThread : public IceUtil::Thread, public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    SessionRefreshThread(const Ice::LoggerPtr& logger, const SessionPrx& session) :
	_logger(logger),
	_session(session),
	_destroy(false),
	_timeout(IceUtil::Time::seconds(5))
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
    SessionPrx _session;
    bool _destroy;
    const IceUtil::Time _timeout;
};
typedef IceUtil::Handle<SessionRefreshThread> SessionRefreshThreadPtr;

void
menu()
{
    cout <<
	"usage:\n"
	"c:     create new hello\n"
	"0-9:   greeting identified hello object\n"
	"s:     shutdown server\n"
	"x:     exit\n"
	"?:     help\n";
}

int
run(int argc, char* argv[], const Ice::CommunicatorPtr& communicator)
{
    Ice::PropertiesPtr properties = communicator->getProperties();
    const char* proxyProperty = "SessionFactory.Proxy";
    string proxy = properties->getProperty(proxyProperty);
    if(proxy.empty())
    {
	cerr << argv[0] << ": property `" << proxyProperty << "' not set" << endl;
	return EXIT_FAILURE;
    }

    Ice::ObjectPrx base = communicator->stringToProxy(proxy);
    SessionFactoryPrx factory = SessionFactoryPrx::checkedCast(base);
    if(!factory)
    {
	cerr << argv[0] << ": invalid proxy" << endl;
	return EXIT_FAILURE;
    }

    SessionPrx session = factory->create();
    if(!session)
    {
	cerr << argv[0] << ": invalid proxy" << endl;
	return EXIT_FAILURE;
    }

    SessionRefreshThreadPtr refresh = new SessionRefreshThread(communicator->getLogger(), session);
    refresh->start();

    vector<HelloPrx> hellos;

    menu();

    try
    {
	do
	{
	    cout << "==> ";
	    char c;
	    cin >> c;
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
		    cout << "index is too high. " << hellos.size() << " exist so far." << endl;
		}
	    }
	    else if(c == 'c')
	    {
		hellos.push_back(session->createHello());
		cout << "created hello object " << hellos.size()-1 << endl;
	    }
	    else if(c == 's')
	    {
		factory->shutdown();
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
	session->destroy();
    }
    catch(const Ice::Exception& ex)
    {
	cerr << ex << endl;
    }

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
