// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/Options.h>
#include <IceUtil/Thread.h>
#include <Ice/Application.h>
#include <Ice/SliceChecksums.h>
#include <IceGrid/Observer.h>
#include <IceGrid/Admin.h>
#include <fstream>

using namespace std;
using namespace Ice;
using namespace IceGrid;

class SessionKeepAliveThread : public IceUtil::Thread, public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    SessionKeepAliveThread(const Ice::LoggerPtr& logger, const IceUtil::Time& timeout, const SessionPrx& session) :
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
		    _session->keepAlive();
		}
		catch(const Ice::Exception& ex)
		{
		    Ice::Warning warn(_logger);
		    warn << "SessionKeepAliveThread: " << ex;
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
typedef IceUtil::Handle<SessionKeepAliveThread> SessionKeepAliveThreadPtr;

class RegistryObserverI : public RegistryObserver
{
public:

    virtual void 
    init(int, const ApplicationDescriptorSeq&, const Ice::Current&)
    {
    }

    virtual void
    applicationAdded(int serial, const ApplicationDescriptorPtr& app, const Ice::Current&)
    {
	cout << "application `" << app->name << "' added (serial = " << serial << ")" << endl;
    }

    virtual void 
    applicationRemoved(int serial, const std::string& name, const Ice::Current&)
    {
	cout << "application `" << name << "' removed (serial = " << serial << ")" << endl;
    }

    virtual void 
    applicationUpdated(int serial, const ApplicationUpdateDescriptor& desc, const Ice::Current&)
    {
	cout << "application `" << desc.name << "' updated (serial = " << serial << ")" << endl;
    }

    virtual void 
    applicationSynced(int serial, const ApplicationDescriptorPtr& app, const Ice::Current&)
    {
	cout << "application `" << app->name << "' synced (serial = " << serial << ")" << endl;
    }
};

class NodeObserverI : public NodeObserver
{
public:

    virtual void 
    init(const NodeDynamicInfoSeq& nodes, const Ice::Current& c)
    {
	for(NodeDynamicInfoSeq::const_iterator p = nodes.begin(); p != nodes.end(); ++p)
	{
	    nodeUp(*p, c);
	}
    }

    virtual void
    nodeUp(const NodeDynamicInfo& node, const Ice::Current& current)
    {
	cout << "node `" << node.name << "' servers: ";
	for(ServerDynamicInfoSeq::const_iterator p = node.servers.begin(); p != node.servers.end(); ++p)
	{
	    cout << p->name << " ";
	}
	cout << "adapters: ";
	for(AdapterDynamicInfoSeq::const_iterator p = node.adapters.begin(); p != node.adapters.end(); ++p)
	{
	    cout << p->id << " ";
	}	
	cout << endl;
    }

    virtual void
    nodeDown(const string& name, const Ice::Current& current)
    {
	cout << "node `" << name << "' is down" << endl;
    }

    virtual void
    updateServer(const string& node, const ServerDynamicInfo& info, const Ice::Current&)
    {
	cout << "node `" << node << "' server `" << info.name << "' updated: pid = " << info.pid << " state = `";
	switch(info.state)
	{
	case Inactive:
	    cout << "Inactive";
	    break;
	case Activating:
	    cout << "Activating";
	    break;
	case Active:
	    cout << "Active";
	    break;
	case Deactivating:
	    cout << "Deactivating";
	    break;
	case Destroying:
	    cout << "Destroying";
	    break;
	case Destroyed:
	    cout << "Destroyed";
	    break;
	}
	cout << "'" << endl;
    }

    virtual void
    updateAdapter(const string& node, const AdapterDynamicInfo& info, const Ice::Current& current)
    {
	cout << "node `" << node << "' adapter `" << info.id << "' updated: ";
	cout << current.adapter->getCommunicator()->proxyToString(info.proxy);
	cout << endl;
    }
};

class Client : public Application
{
public:

    void usage();
    virtual int run(int, char*[]);
};

int
main(int argc, char* argv[])
{
    Client app;
    return app.main(argc, argv);
}

void
Client::usage()
{
    cerr << "Usage: " << appName() << " [options] [file...]\n";
    cerr <<	
	"Options:\n"
	"-h, --help           Show this message.\n"
	"-v, --version        Display the Ice version.\n"
	;
}

int
Client::run(int argc, char* argv[])
{
    IceUtil::Options opts;
    opts.addOpt("h", "help");
    opts.addOpt("v", "version");

    vector<string> args;
    try
    {
    	args = opts.parse(argc, argv);
    }
    catch(const IceUtil::Options::BadOpt& e)
    {
        cerr << e.reason << endl;
	usage();
	return EXIT_FAILURE;
    }

    if(opts.isSet("h") || opts.isSet("help"))
    {
	usage();
	return EXIT_SUCCESS;
    }
    if(opts.isSet("v") || opts.isSet("version"))
    {
	cout << ICE_STRING_VERSION << endl;
	return EXIT_SUCCESS;
    }

    SessionManagerPrx manager = SessionManagerPrx::checkedCast(communicator()->stringToProxy("IceGrid/SessionManager"));
    if(!manager)
    {
	cerr << appName() << ": no valid session manager interface" << endl;
	return EXIT_FAILURE;
    }


    AdminPrx admin = AdminPrx::checkedCast(communicator()->stringToProxy("IceGrid/Admin"));
    if(!admin)
    {
	cerr << appName() << ": no valid administrative interface" << endl;
	return EXIT_FAILURE;
    }

    Ice::SliceChecksumDict serverChecksums = admin->getSliceChecksums();
    Ice::SliceChecksumDict localChecksums = Ice::sliceChecksums();
    for(Ice::SliceChecksumDict::const_iterator q = localChecksums.begin(); q != localChecksums.end(); ++q)
    {
        Ice::SliceChecksumDict::const_iterator r = serverChecksums.find(q->first);
        if(r == serverChecksums.end())
        {
            cerr << appName() << ": server is using unknown Slice type `" << q->first << "'" << endl;
        }
        else if(q->second != r->second)
        {
            cerr << appName() << ": server is using a different Slice definition of `" << q->first << "'" << endl;
        }
    }

    SessionPrx session = manager->createLocalSession("IceGrid.Observer");

    Ice::ObjectAdapterPtr adapter = communicator()->createObjectAdapter("IceGrid.Observer");
    RegistryObserverPrx regObserver = RegistryObserverPrx::uncheckedCast(adapter->addWithUUID(new RegistryObserverI()));
    NodeObserverPrx nodeObserver = NodeObserverPrx::uncheckedCast(adapter->addWithUUID(new NodeObserverI()));
    adapter->activate();
    manager->ice_connection()->setAdapter(adapter);

    SessionKeepAliveThreadPtr keepAlive;
    keepAlive = new SessionKeepAliveThread(communicator()->getLogger(), IceUtil::Time::seconds(5), session);
    keepAlive->start();

    session->setObserversByIdentity(regObserver->ice_getIdentity(), nodeObserver->ice_getIdentity());

    communicator()->waitForShutdown();

    keepAlive->terminate();
    keepAlive->getThreadControl().join();
    keepAlive = 0;
    
    return EXIT_SUCCESS;
}
