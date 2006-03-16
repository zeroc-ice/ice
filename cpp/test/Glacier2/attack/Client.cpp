// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Application.h>
#include <Glacier2/Router.h>
#include <Backend.h>
#include <TestCommon.h>

using namespace std;
using namespace Ice;
using namespace Test;

class CallbackClient : public Application
{
public:

    virtual int run(int, char*[]);
};

int
main(int argc, char* argv[])
{
    PropertiesPtr properties = getDefaultProperties(argc, argv);
    
    //
    // We want to check whether the client retries for evicted
    // proxies, even with regular retries disabled.
    //
    properties->setProperty("Ice.RetryIntervals", "-1");
	
    CallbackClient app;
    return app.main(argc, argv);
}

int
CallbackClient::run(int argc, char* argv[])
{
    cout << "getting router... " << flush;
    ObjectPrx routerBase = communicator()->stringToProxy("Glacier2/router:default -p 12347 -t 10000");
    Glacier2::RouterPrx router = Glacier2::RouterPrx::checkedCast(routerBase);
    test(router);
    cout << "ok" << endl;

    cout << "creating session... " << flush;
    Glacier2::SessionPrx session = router->createSession("userid", "abc123");
    communicator()->setDefaultRouter(router);
    cout << "ok" << endl;

    cout << "making thousands of invocations on proxies... " << flush;
    ObjectPrx backendBase = communicator()->stringToProxy("dummy:tcp -p 12010 -t 10000");
    BackendPrx backend = BackendPrx::uncheckedCast(backendBase);
    backend->ice_ping();

    string msg;
    for(int i = 1; i <= 100000; ++i)
    {
	if(i % 100 == 0)
	{
	    if(!msg.empty())
	    {
		cout << string(msg.size(), '\b');
	    }
	    ostringstream s;
	    s << i;
	    msg = s.str();
	    cout << msg << flush;
	}

	Identity ident;
	string::iterator p;

	ident.name.resize(1 + rand() % 2);
	for(p = ident.name.begin(); p != ident.name.end(); ++p)
	{
	    *p = static_cast<char>('A' + rand() % 26);
	}

	ident.category.resize(1); // (rand() % 2);
	for(p = ident.category.begin(); p != ident.category.end(); ++p)
	{
	    *p = static_cast<char>('a' + rand() % 26);
	}

	backend = BackendPrx::uncheckedCast(backendBase->ice_newIdentity(ident));
	backend->ice_ping();
    }
//    cout << string(msg.size(), '\b') << string(msg.size(), ' ') << string(msg.size(), '\b');
    cout << ' ';
    cout << "ok" << endl;
    
    cout << "testing server and router shutdown... " << flush;
    backend->shutdown();
    communicator()->setDefaultRouter(0);
    ObjectPrx adminBase = communicator()->stringToProxy("Glacier2/admin:tcp -h 127.0.0.1 -p 12348 -t 10000");
    Glacier2::AdminPrx admin = Glacier2::AdminPrx::checkedCast(adminBase);
    test(admin);
    admin->shutdown();
    try
    {
	admin->ice_ping();
	test(false);
    }
    catch(const Ice::LocalException&)
    {
	cout << "ok" << endl;
    }

    return EXIT_SUCCESS;
}
