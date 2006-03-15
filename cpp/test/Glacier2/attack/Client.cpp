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
    // We want to check whether the client retries for reaped proxies,
    // even with regular retries disabled.
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

    cout << "testing stringToProxy for server object... " << flush;
    ObjectPrx base = communicator()->stringToProxy("c1/callback:tcp -p 12010 -t 10000");
    cout << "ok" << endl;
    
    cout << "getting admin proxy... " << flush;
    ObjectPrx adminBase = communicator()->stringToProxy("router:tcp -h 127.0.0.1 -p 12348 -t 10000");
    Glacier2::AdminPrx admin = Glacier2::AdminPrx::checkedCast(adminBase);
    test(admin);
    cout << "ok" << endl;
    
    cout << "testing Glacier2 shutdown... " << flush;
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
