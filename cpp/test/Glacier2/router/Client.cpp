// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Application.h>
#include <Glacier2/Router.h>
#include <TestCommon.h>
#include <CallbackI.h>

using namespace std;
using namespace Ice;
using namespace Test;

class AMI_Callback_initiateNestedCallbackI : public AMI_Callback_initiateNestedCallback,
					     public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    AMI_Callback_initiateNestedCallbackI() :
	_haveResponse(false)
    {
    }

    virtual void
    ice_response(Int response)
    {
	Lock sync(*this);
	_haveResponse = true;
	_response = response;
	notify();
    }

    virtual void
    ice_exception(const Exception& e)
    {
	Lock sync(*this);
	_haveResponse = true;
	_ex.reset(e.ice_clone());
	notify();
    }

    int
    waitResponse() const
    {
	Lock sync(*this);
	while(!_haveResponse)
	{
	    if(!timedWait(IceUtil::Time::milliSeconds(5000)))
	    {
		throw TimeoutException(__FILE__, __LINE__);
	    }
	}
	if(_ex.get())
	{
	    _ex->ice_throw();
	}
	return _response;
    }

private:

    bool _haveResponse;
    auto_ptr<Exception> _ex;
    Int _response;
};
typedef IceUtil::Handle<AMI_Callback_initiateNestedCallbackI> AMI_Callback_initiateNestedCallbackIPtr;

class CallbackClient : public Application
{
public:

    virtual int run(int, char*[]);
};

int
main(int argc, char* argv[])
{
    //
    // We must disable connection warnings, because we attempt to ping
    // the router before session establishment, as well as after
    // session destruction. Both will cause a ConnectionLostException.
    //
    Ice::PropertiesPtr properties = Ice::getDefaultProperties(argc, argv);
    properties->setProperty("Ice.Warn.Connections", "0");

    CallbackClient app;
    return app.main(argc, argv);
}

int
CallbackClient::run(int argc, char* argv[])
{
    ObjectAdapterPtr adapter;

    {
	adapter = communicator()->createObjectAdapterWithEndpoints("CallbackReceiverAdapter", "");
	adapter->activate();
	// Put the print statement after activate(), so that if
	// Ice.PrintAdapterReady is set, the "ready" is the first
	// output from the client, and not the print statement
	// below. Otherwise the Python test scripts will be confused,
	// as they expect the "ready" from the Object Adapter to be
	// the first thing that is printed.
	cout << "creating and activating callback receiver adapter... " << flush;
	cout << "ok" << endl;
    }

    ObjectPrx routerBase;

    {
	cout << "testing stringToProxy for router... " << flush;
	routerBase = communicator()->stringToProxy("abc/def:default -p 12347 -t 10000");
	cout << "ok" << endl;
    }
    
    Glacier2::RouterPrx router;

    {
	cout << "testing checked cast for router... " << flush;
	router = Glacier2::RouterPrx::checkedCast(routerBase);
	test(router);
	cout << "ok" << endl;
    }

    {
	cout << "installing router with communicator... " << flush;
	communicator()->setDefaultRouter(router);
	cout << "ok" << endl;
    }

    ObjectPrx base;

    {
	cout << "testing stringToProxy for server object... " << flush;
	base = communicator()->stringToProxy("c1/callback:tcp -p 12010 -t 10000");
	cout << "ok" << endl;
    }
	
    {
	cout << "trying to ping server before session creation... " << flush;
	try
	{
	    base->ice_ping();
	    test(false);
	}
	catch(const ConnectionLostException&)
	{
	    cout << "ok" << endl;
	}
    }

    Glacier2::SessionPrx session;

    {
	cout << "trying to create session with wrong password... " << flush;
	try
	{
	    session = router->createSession("userid", "xxx");
	    test(false);
	}
	catch(const Glacier2::PermissionDeniedException&)
	{
	    cout << "ok" << endl;
	}
    }

    {
	cout << "trying to destroy non-existing session... " << flush;
	try
	{
	    router->destroySession();
	    test(false);
	}
	catch(const Glacier2::SessionNotExistException&)
	{
	    cout << "ok" << endl;
	}
    }

    {
	cout << "creating session with correct password... " << flush;
	session = router->createSession("userid", "abc123");
	cout << "ok" << endl;
    }

    {
	cout << "trying to create a second session... " << flush;
	try
	{
	    router->createSession("userid", "abc123");
	    test(false);
	}
	catch(const Glacier2::CannotCreateSessionException&)
	{
	    cout << "ok" << endl;
	}
    }

    {
	cout << "pinging server after session creation... " << flush;
	base->ice_ping();
	cout << "ok" << endl;
    }

    CallbackPrx twoway;

    {
	cout << "testing checked cast for server object... " << flush;
	twoway = CallbackPrx::checkedCast(base);
	test(twoway);
	cout << "ok" << endl;
    }

    {
	cout << "installing router with object adapter... " << flush;
	adapter->addRouter(router);
	cout << "ok" << endl;
    }

    string category;

    {
	cout << "getting category from router... " << flush;
	category = router->getCategoryForClient();
	cout << "ok" << endl;
    }

    CallbackReceiverI* callbackReceiverImpl;
    ObjectPtr callbackReceiver;
    CallbackReceiverPrx twowayR;
    CallbackReceiverPrx fakeTwowayR;
    
    {
	cout << "creating and adding callback receiver object... " << flush;
	callbackReceiverImpl = new CallbackReceiverI;
	callbackReceiver = callbackReceiverImpl;
	Identity callbackReceiverIdent;
	callbackReceiverIdent.name = "callbackReceiver";
	callbackReceiverIdent.category = category;
	twowayR = CallbackReceiverPrx::uncheckedCast(adapter->add(callbackReceiver, callbackReceiverIdent));
	Identity fakeCallbackReceiverIdent;
	fakeCallbackReceiverIdent.name = "callbackReceiver";
	fakeCallbackReceiverIdent.category = "dummy";
	fakeTwowayR = CallbackReceiverPrx::uncheckedCast(adapter->add(callbackReceiver, fakeCallbackReceiverIdent));
	cout << "ok" << endl;
    }
    
    {
	cout << "testing oneway callback... " << flush;
	CallbackPrx oneway = CallbackPrx::uncheckedCast(twoway->ice_oneway());
	CallbackReceiverPrx onewayR = CallbackReceiverPrx::uncheckedCast(twowayR->ice_oneway());
	Context context;
	context["_fwd"] = "o";
	oneway->initiateCallback(onewayR, context);
	test(callbackReceiverImpl->callbackOK());
	cout << "ok" << endl;
    }

    {
	cout << "testing twoway callback... " << flush;
	Context context;
	context["_fwd"] = "t";
	twoway->initiateCallback(twowayR, context);
	test(callbackReceiverImpl->callbackOK());
	cout << "ok" << endl;
    }

    {
	cout << "testing nested twoway callback... " << flush;
	Context context;
	context["_fwd"] = "t";
	AMI_Callback_initiateNestedCallbackIPtr cb0 = new AMI_Callback_initiateNestedCallbackI();
	twoway->initiateNestedCallback_async(cb0, 0, twowayR, context);
	AMI_Callback_initiateNestedCallbackIPtr cb1 = new AMI_Callback_initiateNestedCallbackI();
	twoway->initiateNestedCallback_async(cb1, 1, twowayR, context);
	AMI_Callback_initiateNestedCallbackIPtr cb2 = new AMI_Callback_initiateNestedCallbackI();
	twoway->initiateNestedCallback_async(cb2, 2, twowayR, context);
	test(callbackReceiverImpl->answerNestedCallbacks(3));
	test(cb0->waitResponse() == 0);
	test(cb1->waitResponse() == 1);
	test(cb2->waitResponse() == 2);
	cout << "ok" << endl;
    }

    {
	cout << "ditto, but with user exception... " << flush;
	Context context;
	context["_fwd"] = "t";
	try
	{
	    twoway->initiateCallbackEx(twowayR, context);
	    test(false);
	}
	catch(const CallbackException& ex)
	{
	    test(ex.someValue == 3.14);
	    test(ex.someString == "3.14");
	}
	test(callbackReceiverImpl->callbackOK());
	cout << "ok" << endl;
    }

    {
	cout << "trying twoway callback with fake category... " << flush;
	Context context;
	context["_fwd"] = "t";
	try
	{
	    twoway->initiateCallback(fakeTwowayR, context);
	    test(false);
	}
	catch(const ObjectNotExistException&)
	{
	    cout << "ok" << endl;
	}
    }

    {
	cout << "testing whether other allowed category is accepted... " << flush;
	Context context;
	context["_fwd"] = "t";
	CallbackPrx otherCategoryTwoway = CallbackPrx::uncheckedCast(
	    twoway->ice_newIdentity(stringToIdentity("c2/callback")));
	otherCategoryTwoway->initiateCallback(twowayR, context);
	test(callbackReceiverImpl->callbackOK());
	cout << "ok" << endl;
    }

    {
	cout << "testing whether disallowed category gets rejected... " << flush;
	Context context;
	context["_fwd"] = "t";
	try
	{
	    CallbackPrx otherCategoryTwoway = CallbackPrx::uncheckedCast(
		twoway->ice_newIdentity(stringToIdentity("c3/callback")));
	    otherCategoryTwoway->initiateCallback(twowayR, context);
	    test(false);
	}
	catch(const ObjectNotExistException&)
	{
	    cout << "ok" << endl;
	}
    }

    {
	cout << "testing whether user-id as category is accepted... " << flush;
	Context context;
	context["_fwd"] = "t";
	CallbackPrx otherCategoryTwoway = CallbackPrx::uncheckedCast(
	    twoway->ice_newIdentity(stringToIdentity("_userid/callback")));
	otherCategoryTwoway->initiateCallback(twowayR, context);
	test(callbackReceiverImpl->callbackOK());
	cout << "ok" << endl;
    }

    {
	cout << "testing server shutdown... " << flush;
	twoway->shutdown();
	// No ping, otherwise the router prints a warning message if it's
	// started with --Ice.Warn.Connections.
	cout << "ok" << endl;
	/*
	  try
	  {
	  base->ice_ping();
	  test(false);
	  }
	  // If we use the glacier router, the exact exception reason gets
	  // lost.
	  catch(const UnknownLocalException&)
	  {
	  cout << "ok" << endl;
	  }
	*/
    }
    
    {
	cout << "destroying session... " << flush;
	router->destroySession();
	cout << "ok" << endl;
    }

    {
	cout << "removing router from object adapter... " << flush;
	adapter->removeRouter(router);
	cout << "ok" << endl;
    }
    
    {
	cout << "trying to ping server after session destruction... " << flush;
	try
	{
	    base->ice_ping();
	    test(false);
	}
	catch(const ConnectionLostException&)
	{
	    cout << "ok" << endl;
	}
    }

    if(argc >= 2 && strcmp(argv[1], "--shutdown") == 0)
    {
	{
	    cout << "uninstalling router with communicator... " << flush;
	    communicator()->setDefaultRouter(0);
	    cout << "ok" << endl;
	}

	ObjectPrx adminBase;

	{
	    cout << "testing stringToProxy for admin object... " << flush;
	    adminBase = communicator()->stringToProxy("ABC/DEF:tcp -h 127.0.0.1 -p 12348 -t 10000");
	    cout << "ok" << endl;
	}
	
/*
	{
	    cout << "uninstalling router with admin object... " << flush;
	    adminBase->ice_router(0);
	    cout << "ok" << endl;
	}
*/

	Glacier2::AdminPrx admin;
	
	{
	    cout << "testing checked cast for admin object... " << flush;
	    admin = Glacier2::AdminPrx::checkedCast(adminBase);
	    test(admin);
	    cout << "ok" << endl;
	}

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
    }
    
    return EXIT_SUCCESS;
}
