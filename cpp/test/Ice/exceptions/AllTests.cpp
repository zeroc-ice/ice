// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestCommon.h>
#include <Test.h>

using namespace std;

class EmptyI : virtual public Empty
{
};

class ServantLocatorI : virtual public Ice::ServantLocator
{
public:

    virtual Ice::ObjectPtr locate(const Ice::Current&, Ice::LocalObjectPtr&) { return 0; }
    virtual void finished(const Ice::Current&, const Ice::ObjectPtr&, const Ice::LocalObjectPtr&) {}
    virtual void deactivate() {}
};

class ObjectFactoryI : virtual public Ice::ObjectFactory
{
public:
    virtual Ice::ObjectPtr create(const string&) { return 0; }
    virtual void destroy() {}
};

class MyExceptionFactory : public Ice::UserExceptionFactory
{
public:

    virtual void createAndThrow(const string& type)
    {
	if(type == "::A")
	{
	    throw A();
	}
	else if(type == "::B")
	{
	    throw B();
	}
	else if(type == "::C")
	{
	    throw C();
	}
	else if(type == "::D")
	{
	    throw D();
	}
	assert(false); // Should never be reached
    }

    virtual void destroy()
    {
	// Nothing to do
    }
};

class CallbackBase : public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    CallbackBase() :
	_called(false)
    {
    }

    virtual ~CallbackBase()
    {
    }

    bool check()
    {
	IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
	while(!_called)
	{
	    if(!timedWait(IceUtil::Time::seconds(5)))
	    {
		return false;
	    }
	}
	return true;
    }

protected:

    void called()
    {
	IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
	assert(!_called);
	_called = true;
	notify();
    }

private:

    bool _called;
};

class AMI_Thrower_throwAasAI : public AMI_Thrower_throwAasA, public CallbackBase
{
public:

    virtual void ice_response()
    {
	test(false);
    }

    virtual void ice_exception(const Ice::Exception& exc)
    {
	try
	{
	    exc.ice_throw();
	}
	catch(const A& ex)
	{
	    test(ex.aMem == 1);
	}
	catch(...)
	{
	    test(false);
	}
	called();
    }
};

typedef ::IceUtil::Handle< ::AMI_Thrower_throwAasAI> AMI_Thrower_throwAasAIPtr;

class AMI_Thrower_throwAasAObjectNotExistI : public AMI_Thrower_throwAasA, public CallbackBase
{
public:

    virtual void ice_response()
    {
	test(false);
    }

    virtual void ice_exception(const Ice::Exception& exc)
    {
	try
	{
	    exc.ice_throw();
	}
	catch(const Ice::ObjectNotExistException& ex)
	{
	    Ice::Identity id = Ice::stringToIdentity("does not exist");
	    test(ex.id == id);
	}
	catch(...)
	{
	    test(false);
	}
	called();
    }
};

typedef ::IceUtil::Handle< ::AMI_Thrower_throwAasAObjectNotExistI> AMI_Thrower_throwAasAObjectNotExistIPtr;

class AMI_Thrower_throwAasAFacetNotExistI : public AMI_Thrower_throwAasA, public CallbackBase
{
public:

    virtual void ice_response()
    {
	test(false);
    }

    virtual void ice_exception(const Ice::Exception& exc)
    {
	try
	{
	    exc.ice_throw();
	}
	catch(const Ice::FacetNotExistException& ex)
	{
	    test(ex.facet.size() == 1);
	    test(ex.facet[0] == "no such facet");
	}
	catch(...)
	{
	    test(false);
	}
	called();
    }
};

typedef ::IceUtil::Handle< ::AMI_Thrower_throwAasAFacetNotExistI> AMI_Thrower_throwAasAFacetNotExistIPtr;

class AMI_Thrower_throwAasAFacetNotExist2I : public AMI_Thrower_throwAasA, public CallbackBase
{
public:

    virtual void ice_response()
    {
	test(false);
    }

    virtual void ice_exception(const Ice::Exception& exc)
    {
	try
	{
	    exc.ice_throw();
	}
	catch(const Ice::FacetNotExistException& ex)
	{
	    test(ex.facet.size() == 2);
	    test(ex.facet[0] == "no such facet");
	    test(ex.facet[1] == "no such facet either");
	}
	catch(...)
	{
	    test(false);
	}
	called();
    }
};

typedef ::IceUtil::Handle< ::AMI_Thrower_throwAasAFacetNotExist2I> AMI_Thrower_throwAasAFacetNotExist2IPtr;

class AMI_Thrower_throwAorDasAorDI : public AMI_Thrower_throwAorDasAorD, public CallbackBase
{
public:

    virtual void ice_response()
    {
	test(false);
    }

    virtual void ice_exception(const Ice::Exception& exc)
    {
	try
	{
	    exc.ice_throw();
	}
	catch(const A& ex)
	{
	    test(ex.aMem == 1);
	}
	catch(const D& ex)
	{
	    test(ex.dMem == -1);
	}
	catch(...)
	{
	    test(false);
	}
	called();
    }
};

typedef ::IceUtil::Handle< ::AMI_Thrower_throwAorDasAorDI> AMI_Thrower_throwAorDasAorDIPtr;

class AMI_Thrower_throwBasANoFactoryI : public AMI_Thrower_throwBasA, public CallbackBase
{
public:

    virtual void ice_response()
    {
	test(false);
    }

    virtual void ice_exception(const Ice::Exception& exc)
    {
	try
	{
	    exc.ice_throw();
	}
	catch(const Ice::NoUserExceptionFactoryException&)
	{
	}
	catch(...)
	{
	    test(false);
	}
	called();
    }
};

typedef ::IceUtil::Handle< ::AMI_Thrower_throwBasANoFactoryI> AMI_Thrower_throwBasANoFactoryIPtr;

class AMI_Thrower_throwBasAI : public AMI_Thrower_throwBasA, public CallbackBase
{
public:

    virtual void ice_response()
    {
	test(false);
    }

    virtual void ice_exception(const Ice::Exception& exc)
    {
	try
	{
	    exc.ice_throw();
	}
	catch(const B& ex)
	{
	    test(ex.aMem == 1);
	    test(ex.bMem == 2);
	}
	catch(...)
	{
	    test(false);
	}
	called();
    }
};

typedef ::IceUtil::Handle< ::AMI_Thrower_throwBasAI> AMI_Thrower_throwBasAIPtr;

class AMI_Thrower_throwCasANoFactoryI : public AMI_Thrower_throwCasA, public CallbackBase
{
public:

    virtual void ice_response()
    {
	test(false);
    }

    virtual void ice_exception(const Ice::Exception& exc)
    {
	try
	{
	    exc.ice_throw();
	}
	catch(const Ice::NoUserExceptionFactoryException&)
	{
	}
	catch(...)
	{
	    test(false);
	}
	called();
    }
};

typedef ::IceUtil::Handle< ::AMI_Thrower_throwCasANoFactoryI> AMI_Thrower_throwCasANoFactoryIPtr;

class AMI_Thrower_throwCasAI : public AMI_Thrower_throwCasA, public CallbackBase
{
public:

    virtual void ice_response()
    {
	test(false);
    }

    virtual void ice_exception(const Ice::Exception& exc)
    {
	try
	{
	    exc.ice_throw();
	}
	catch(const C& ex)
	{
	    test(ex.aMem == 1);
	    test(ex.bMem == 2);
	    test(ex.cMem == 3);
	}
	catch(...)
	{
	    test(false);
	}
	called();
    }
};

typedef ::IceUtil::Handle< ::AMI_Thrower_throwCasAI> AMI_Thrower_throwCasAIPtr;

class AMI_Thrower_throwBasBI : public AMI_Thrower_throwBasB, public CallbackBase
{
public:

    virtual void ice_response()
    {
	test(false);
    }

    virtual void ice_exception(const Ice::Exception& exc)
    {
	try
	{
	    exc.ice_throw();
	}
	catch(const B& ex)
	{
	    test(ex.aMem == 1);
	    test(ex.bMem == 2);
	}
	catch(...)
	{
	    test(false);
	}
	called();
    }
};

typedef ::IceUtil::Handle< ::AMI_Thrower_throwBasBI> AMI_Thrower_throwBasBIPtr;

class AMI_Thrower_throwCasBNoFactoryI : public AMI_Thrower_throwCasB, public CallbackBase
{
public:

    virtual void ice_response()
    {
	test(false);
    }

    virtual void ice_exception(const Ice::Exception& exc)
    {
	try
	{
	    exc.ice_throw();
	}
	catch(const Ice::NoUserExceptionFactoryException&)
	{
	}
	catch(...)
	{
	    test(false);
	}
	called();
    }
};

typedef ::IceUtil::Handle< ::AMI_Thrower_throwCasBNoFactoryI> AMI_Thrower_throwCasBNoFactoryIPtr;

class AMI_Thrower_throwCasBI : public AMI_Thrower_throwCasB, public CallbackBase
{
public:

    virtual void ice_response()
    {
	test(false);
    }

    virtual void ice_exception(const Ice::Exception& exc)
    {
	try
	{
	    exc.ice_throw();
	}
	catch(const C& ex)
	{
	    test(ex.aMem == 1);
	    test(ex.bMem == 2);
	    test(ex.cMem == 3);
	}
	catch(...)
	{
	    test(false);
	}
	called();
    }
};

typedef ::IceUtil::Handle< ::AMI_Thrower_throwCasBI> AMI_Thrower_throwCasBIPtr;

class AMI_Thrower_throwCasCI : public AMI_Thrower_throwCasC, public CallbackBase
{
public:

    virtual void ice_response()
    {
	test(false);
    }

    virtual void ice_exception(const Ice::Exception& exc)
    {
	try
	{
	    exc.ice_throw();
	}
	catch(const C& ex)
	{
	    test(ex.aMem == 1);
	    test(ex.bMem == 2);
	    test(ex.cMem == 3);
	}
	catch(...)
	{
	    test(false);
	}
	called();
    }
};

typedef ::IceUtil::Handle< ::AMI_Thrower_throwCasCI> AMI_Thrower_throwCasCIPtr;

class AMI_Thrower_throwUndeclaredAI : public AMI_Thrower_throwUndeclaredA, public CallbackBase
{
public:

    virtual void ice_response()
    {
	test(false);
    }

    virtual void ice_exception(const Ice::Exception& exc)
    {
	try
	{
	    exc.ice_throw();
	}
	catch(const Ice::UnknownUserException&)
	{
	}
	catch(...)
	{
	    test(false);
	}
	called();
    }
};

typedef ::IceUtil::Handle< ::AMI_Thrower_throwUndeclaredAI> AMI_Thrower_throwUndeclaredAIPtr;

class AMI_Thrower_throwUndeclaredBI : public AMI_Thrower_throwUndeclaredB, public CallbackBase
{
public:

    virtual void ice_response()
    {
	test(false);
    }

    virtual void ice_exception(const Ice::Exception& exc)
    {
	try
	{
	    exc.ice_throw();
	}
	catch(const Ice::UnknownUserException&)
	{
	}
	catch(...)
	{
	    test(false);
	}
	called();
    }
};

typedef ::IceUtil::Handle< ::AMI_Thrower_throwUndeclaredBI> AMI_Thrower_throwUndeclaredBIPtr;

class AMI_Thrower_throwUndeclaredCI : public AMI_Thrower_throwUndeclaredC, public CallbackBase
{
public:

    virtual void ice_response()
    {
	test(false);
    }

    virtual void ice_exception(const Ice::Exception& exc)
    {
	try
	{
	    exc.ice_throw();
	}
	catch(const Ice::UnknownUserException&)
	{
	}
	catch(...)
	{
	    test(false);
	}
	called();
    }
};

typedef ::IceUtil::Handle< ::AMI_Thrower_throwUndeclaredCI> AMI_Thrower_throwUndeclaredCIPtr;

class AMI_Thrower_throwLocalExceptionI : public AMI_Thrower_throwLocalException, public CallbackBase
{
public:

    virtual void ice_response()
    {
	test(false);
    }

    virtual void ice_exception(const Ice::Exception& exc)
    {
	try
	{
	    exc.ice_throw();
	}
	catch(const Ice::UnknownLocalException&)
	{
	}
	catch(...)
	{
	    test(false);
	}
	called();
    }
};

typedef ::IceUtil::Handle< ::AMI_Thrower_throwLocalExceptionI> AMI_Thrower_throwLocalExceptionIPtr;

class AMI_Thrower_throwNonIceExceptionI : public AMI_Thrower_throwNonIceException, public CallbackBase
{
public:

    virtual void ice_response()
    {
	test(false);
    }

    virtual void ice_exception(const Ice::Exception& exc)
    {
	try
	{
	    exc.ice_throw();
	}
	catch(const Ice::UnknownException&)
	{
	}
	catch(...)
	{
	    test(false);
	}
	called();
    }
};

typedef ::IceUtil::Handle< ::AMI_Thrower_throwNonIceExceptionI> AMI_Thrower_throwNonIceExceptionIPtr;

class AMI_WrongOperation_noSuchOperationI : public AMI_WrongOperation_noSuchOperation, public CallbackBase
{
public:

    virtual void ice_response()
    {
	test(false);
    }

    virtual void ice_exception(const Ice::Exception& exc)
    {
	try
	{
	    exc.ice_throw();
	}
	catch(const Ice::OperationNotExistException& ex)
	{
	    test(ex.operation == "noSuchOperation");
	}
	catch(...)
	{
	    test(false);
	}
	called();
    }
};

typedef ::IceUtil::Handle< ::AMI_WrongOperation_noSuchOperationI> AMI_WrongOperation_noSuchOperationIPtr;

ThrowerPrx
allTests(const Ice::CommunicatorPtr& communicator, bool collocated)
{
    cout << "testing servant registration exceptions... " << flush;
    {
	Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter1");
	Ice::ObjectPtr obj = new EmptyI;
	adapter->add(obj, Ice::stringToIdentity("x"));
	bool gotException = false;
	try
	{
	    adapter->add(obj, Ice::stringToIdentity("x"));
	}
	catch(const Ice::AlreadyRegisteredException&)
	{
	    gotException = true;
	}
	test(gotException);

	gotException = false;
	adapter->remove(Ice::stringToIdentity("x"));
	try
	{
	    adapter->remove(Ice::stringToIdentity("x"));
	}
	catch(const Ice::NotRegisteredException&)
	{
	    gotException = true;
	}
	test(gotException);

	adapter->deactivate();
    }
    cout << "ok" << endl;

    cout << "testing servant locator registrations exceptions... " << flush;
    {
	Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter2");
	Ice::ServantLocatorPtr loc = new ServantLocatorI;
	adapter->addServantLocator(loc, "x");
	bool gotException = false;
	try
	{
	    adapter->addServantLocator(loc, "x");
	}
	catch(const Ice::AlreadyRegisteredException&)
	{
	    gotException = true;
	}
	test(gotException);

	gotException = false;
	adapter->removeServantLocator("x");
	try
	{
	    adapter->removeServantLocator("x");
	}
	catch(const Ice::NotRegisteredException&)
	{
	    gotException = true;
	}
	test(gotException);

	adapter->deactivate();
    }
    cout << "ok" << endl;

    cout << "testing object factory registration exceptions... " << flush;
    {
	Ice::ObjectFactoryPtr of = new ObjectFactoryI;
	communicator->addObjectFactory(of, "x");
	bool gotException = false;
	try
	{
	    communicator->addObjectFactory(of, "x");
	}
	catch(const Ice::AlreadyRegisteredException&)
	{
	    gotException = true;
	}
	test(gotException);

	gotException = false;
	communicator->removeObjectFactory("x");
	try
	{
	    communicator->removeObjectFactory("x");
	}
	catch(const Ice::NotRegisteredException&)
	{
	    gotException = true;
	}
	test(gotException);
    }
    cout << "ok" << endl;

    cout << "testing user exception factory registration exceptions... " << flush;
    {
	Ice::UserExceptionFactoryPtr f = new MyExceptionFactory;
	communicator->addUserExceptionFactory(f, "::x");
	bool gotException = false;
	try
	{
	    communicator->addUserExceptionFactory(f, "::x");
	}
	catch(const Ice::AlreadyRegisteredException&)
	{
	    gotException = true;
	}
	test(gotException);

	gotException = false;
	communicator->removeUserExceptionFactory("::x");
	try
	{
	    communicator->removeUserExceptionFactory("::x");
	}
	catch(const Ice::NotRegisteredException&)
	{
	    gotException = true;
	}
	test(gotException);
    }
    cout << "ok" << endl;

    cout << "testing stringToProxy... " << flush;
    string ref = "thrower:default -p 12345 -t 2000";
    Ice::ObjectPrx base = communicator->stringToProxy(ref);
    test(base);
    cout << "ok" << endl;

    cout << "testing checked cast... " << flush;
    ThrowerPrx thrower = ThrowerPrx::checkedCast(base);
    test(thrower);
    test(thrower == base);
    cout << "ok" << endl;

    cout << "catching exact types... " << flush;

    try
    {
	thrower->throwAasA(1);
	test(false);
    }
    catch(const A& ex)
    {
	test(ex.aMem == 1);
    }
    catch(const Ice::Exception& ex)
    {
	cout << ex << endl;
	test(false);
    }
    catch(...)
    {
	test(false);
    }

    try
    {
	thrower->throwAorDasAorD(1);
	test(false);
    }
    catch(const A& ex)
    {
	test(ex.aMem == 1);
    }
    catch(...)
    {
	test(false);
    }

    try
    {
	thrower->throwAorDasAorD(-1);
	test(false);
    }
    catch(const D& ex)
    {
	test(ex.dMem == -1);
    }
    catch(...)
    {
	test(false);
    }

    try
    {
	thrower->throwBasB(1, 2);
	test(false);
    }
    catch(const B& ex)
    {
	test(ex.aMem == 1);
	test(ex.bMem == 2);
    }
    catch(...)
    {
	test(false);
    }

    try
    {
	thrower->throwCasC(1, 2, 3);
	test(false);
    }
    catch(const C& ex)
    {
	test(ex.aMem == 1);
	test(ex.bMem == 2);
	test(ex.cMem == 3);
    }
    catch(...)
    {
	test(false);
    }

    cout << "ok" << endl;

    cout << "catching base types... " << flush;

    try
    {
	thrower->throwBasB(1, 2);
	test(false);
    }
    catch(const A& ex)
    {
	test(ex.aMem == 1);
    }
    catch(...)
    {
	test(false);
    }

    try
    {
	thrower->throwCasC(1, 2, 3);
	test(false);
    }
    catch(const B& ex)
    {
	test(ex.aMem == 1);
	test(ex.bMem == 2);
    }
    catch(...)
    {
	test(false);
    }

    cout << "ok" << endl;

    if(!collocated) // If the server is collocated, exception factories are not needed.
    {
	cout << "catching derived types w/o exception factories... " << flush;
	
	try
	{
	    thrower->throwBasA(1, 2);
	    test(false);
	}
	catch(const Ice::NoUserExceptionFactoryException&)
	{
	}
	catch(...)
	{
	    test(false);
	}
	
	try
	{
	    thrower->throwCasA(1, 2, 3);
	    test(false);
	}
	catch(const Ice::NoUserExceptionFactoryException&)
	{
	}
	catch(...)
	{
	    test(false);
	}
	
	try
	{
	    thrower->throwCasB(1, 2, 3);
	    test(false);
	}
	catch(const Ice::NoUserExceptionFactoryException&)
	{
	}
	catch(...)
	{
	    test(false);
	}
	
	cout << "ok" << endl;
	
	cout << "catching derived types w/ exception factories... " << flush;
	
	Ice::UserExceptionFactoryPtr factory = new MyExceptionFactory;
	communicator->addUserExceptionFactory(factory, "::A");
	communicator->addUserExceptionFactory(factory, "::B");
	communicator->addUserExceptionFactory(factory, "::C");
	communicator->addUserExceptionFactory(factory, "::D");
    }
    else
    {
	cout << "catching derived types... " << flush;
    }
	
    try
    {
	thrower->throwBasA(1, 2);
	test(false);
    }
    catch(const B& ex)
    {
	test(ex.aMem == 1);
	test(ex.bMem == 2);
    }
    catch(...)
    {
	test(false);
    }

    try
    {
	thrower->throwCasA(1, 2, 3);
	test(false);
    }
    catch(const C& ex)
    {
	test(ex.aMem == 1);
    	test(ex.bMem == 2);
    	test(ex.cMem == 3);
    }
    catch(...)
    {
	test(false);
    }

    try
    {
	thrower->throwCasB(1, 2, 3);
	test(false);
    }
    catch(const C& ex)
    {
	test(ex.aMem == 1);
	test(ex.bMem == 2);
	test(ex.cMem == 3);
    }
    catch(...)
    {
	test(false);
    }

    cout << "ok" << endl;

    if(thrower->supportsUndeclaredExceptions())
    {
	cout << "catching unknown user exception... " << flush;
	
	try
	{
	    thrower->throwUndeclaredA(1);
	    test(false);
	}
	catch(const A& ex)
	{
	    //
	    // We get the original exception with collocation
	    // optimization.
	    //
	    test(collocated);
	    test(ex.aMem == 1);
	}
	catch(const Ice::UnknownUserException&)
	{
	    //
	    // We get the an unknown user exception without
	    // collocation optimization.
	    //
	    test(!collocated);
	}
	catch(...)
	{
	    test(false);
	}
	
	try
	{
	    thrower->throwUndeclaredB(1, 2);
	    test(false);
	}
	catch(const B& ex)
	{
	    //
	    // We get the original exception with collocation
	    // optimization.
	    //
	    test(collocated);
	    test(ex.aMem == 1);
	    test(ex.bMem == 2);
	}
	catch(const Ice::UnknownUserException&)
	{
	    //
	    // We get the an unknown user exception without
	    // collocation optimization.
	    //
	    test(!collocated);
	}
	catch(...)
	{
	    test(false);
	}
	
	try
	{
	    thrower->throwUndeclaredC(1, 2, 3);
	    test(false);
	}
	catch(const C& ex)
	{
	    //
	    // We get the original exception with collocation
	    // optimization.
	    //
	    test(collocated);
	    test(ex.aMem == 1);
	    test(ex.bMem == 2);
	    test(ex.cMem == 3);
	}
	catch(const Ice::UnknownUserException&)
	{
	    //
	    // We get the an unknown user exception without
	    // collocation optimization.
	    //
	    test(!collocated);
	}
	catch(...)
	{
	    test(false);
	}
	
	cout << "ok" << endl;
    }

    cout << "catching object not exist exception... " << flush;

    Ice::Identity id = Ice::stringToIdentity("does not exist");
    try
    {
	ThrowerPrx thrower2 = ThrowerPrx::uncheckedCast(thrower->ice_newIdentity(id));
	thrower2->throwAasA(1);
//	thrower2->ice_ping();
	test(false);
    }
    catch(const Ice::ObjectNotExistException& ex)
    {
	test(ex.id == id);
    }
    catch(...)
    {
	test(false);
    }

    cout << "ok" << endl;
    
    cout << "catching facet not exist exception... " << flush;

    try
    {
	ThrowerPrx thrower2 = ThrowerPrx::uncheckedCast(thrower, "no such facet");
	try
	{
	    thrower2->ice_ping();
	    test(false);
	}
	catch(const Ice::FacetNotExistException& ex)
	{
	    test(ex.facet.size() == 1);
	    test(ex.facet[0] == "no such facet");
	}

	ThrowerPrx thrower3 = ThrowerPrx::uncheckedCast(thrower2, "no such facet either");
	try
	{
	    thrower3->ice_ping();
	    test(false);
	}
	catch(const Ice::FacetNotExistException& ex)
	{
	    test(ex.facet.size() == 2);
	    test(ex.facet[0] == "no such facet");
	    test(ex.facet[1] == "no such facet either");
	}
    }
    catch(...)
    {
	test(false);
    }

    cout << "ok" << endl;
    
    cout << "catching operation not exist exception... " << flush;

    try
    {
	WrongOperationPrx thrower2 = WrongOperationPrx::uncheckedCast(thrower);
	thrower2->noSuchOperation();
	test(false);
    }
    catch(const Ice::OperationNotExistException& ex)
    {
	test(ex.operation == "noSuchOperation");
    }
    catch(...)
    {
	test(false);
    }

    cout << "ok" << endl;
    
    cout << "catching unknown local exception... " << flush;

    try
    {
	thrower->throwLocalException();
	test(false);
    }
    catch(const Ice::TimeoutException&)
    {
	//
	// We get the original exception with collocation
	// optimization.
	//
	test(collocated);
    }
    catch(const Ice::UnknownLocalException&)
    {
	//
	// We get the an unknown local exception without collocation
	// optimization.
	//
	test(!collocated);
    }
    catch(...)
    {
	test(false);
    }

    cout << "ok" << endl;
    
    cout << "catching unknown non-Ice exception... " << flush;
    
    try
    {
	thrower->throwNonIceException();
	test(false);
    }
    catch(const Ice::UnknownException&)
    {
	//
	// We get the an unknown exception without collocation
	// optimization.
	//
	assert(!collocated);
    }
    catch(...)
    {
	//
	// We get the original exception with collocation
	// optimization.
	//
	assert(collocated);
    }
    
    cout << "ok" << endl;

    if(!collocated) // Remove exception factories if they have been installed before.
    {
	communicator->removeUserExceptionFactory("::A");
	communicator->removeUserExceptionFactory("::B");
	communicator->removeUserExceptionFactory("::C");
	communicator->removeUserExceptionFactory("::D");
    }

    if(!collocated)
    {
	cout << "catching exact types with AMI... " << flush;

	{
	    AMI_Thrower_throwAasAIPtr cb = new AMI_Thrower_throwAasAI;
	    thrower->throwAasA_async(cb, 1);
	    test(cb->check());
	}
	
	{
	    AMI_Thrower_throwAorDasAorDIPtr cb = new AMI_Thrower_throwAorDasAorDI;
	    thrower->throwAorDasAorD_async(cb, 1);
	    test(cb->check());
	}
	
	{
	    AMI_Thrower_throwAorDasAorDIPtr cb = new AMI_Thrower_throwAorDasAorDI;
	    thrower->throwAorDasAorD_async(cb, -1);
	    test(cb->check());
	}
	
	{
	    AMI_Thrower_throwBasBIPtr cb = new AMI_Thrower_throwBasBI;
	    thrower->throwBasB_async(cb, 1, 2);
	    test(cb->check());
	}
	
	{
	    AMI_Thrower_throwCasCIPtr cb = new AMI_Thrower_throwCasCI;
	    thrower->throwCasC_async(cb, 1, 2, 3);
	    test(cb->check());
	}
	
	cout << "ok" << endl;
	
	cout << "catching derived types w/o exception factories with AMI... " << flush;
	
	{
	    AMI_Thrower_throwBasANoFactoryIPtr cb = new AMI_Thrower_throwBasANoFactoryI;
	    thrower->throwBasA_async(cb, 1, 2);
	    test(cb->check());
	}

	{
	    AMI_Thrower_throwCasANoFactoryIPtr cb = new AMI_Thrower_throwCasANoFactoryI;
	    thrower->throwCasA_async(cb, 1, 2, 3);
	    test(cb->check());
	}
	
	{
	    AMI_Thrower_throwCasBNoFactoryIPtr cb = new AMI_Thrower_throwCasBNoFactoryI;
	    thrower->throwCasB_async(cb, 1, 2, 3);
	    test(cb->check());
	}
	
	cout << "ok" << endl;

	cout << "catching derived types w/ exception factories with AMI... " << flush;
	
	Ice::UserExceptionFactoryPtr factory = new MyExceptionFactory;
	communicator->addUserExceptionFactory(factory, "::A");
	communicator->addUserExceptionFactory(factory, "::B");
	communicator->addUserExceptionFactory(factory, "::C");
	communicator->addUserExceptionFactory(factory, "::D");
	
	{
	    AMI_Thrower_throwBasAIPtr cb = new AMI_Thrower_throwBasAI;
	    thrower->throwBasA_async(cb, 1, 2);
	    test(cb->check());
	}

	{
	    AMI_Thrower_throwCasAIPtr cb = new AMI_Thrower_throwCasAI;
	    thrower->throwCasA_async(cb, 1, 2, 3);
	    test(cb->check());
	}
	
	{
	    AMI_Thrower_throwCasBIPtr cb = new AMI_Thrower_throwCasBI;
	    thrower->throwCasB_async(cb, 1, 2, 3);
	    test(cb->check());
	}
	
	cout << "ok" << endl;

	if(thrower->supportsUndeclaredExceptions())
	{
	    cout << "catching unknown user exception with AMI... " << flush;
	    
	    {
		AMI_Thrower_throwUndeclaredAIPtr cb = new AMI_Thrower_throwUndeclaredAI;
		thrower->throwUndeclaredA_async(cb, 1);
		test(cb->check());
	    }

	    {
		AMI_Thrower_throwUndeclaredBIPtr cb = new AMI_Thrower_throwUndeclaredBI;
		thrower->throwUndeclaredB_async(cb, 1, 2);
		test(cb->check());
	    }

	    {
		AMI_Thrower_throwUndeclaredCIPtr cb = new AMI_Thrower_throwUndeclaredCI;
		thrower->throwUndeclaredC_async(cb, 1, 2, 3);
		test(cb->check());
	    }
	
	    cout << "ok" << endl;
	}

	cout << "catching object not exist exception with AMI... " << flush;

	{
	    Ice::Identity id = Ice::stringToIdentity("does not exist");
	    ThrowerPrx thrower2 = ThrowerPrx::uncheckedCast(thrower->ice_newIdentity(id));
	    AMI_Thrower_throwAasAObjectNotExistIPtr cb = new AMI_Thrower_throwAasAObjectNotExistI;
	    thrower2->throwAasA_async(cb, 1);
	    test(cb->check());
	}

	cout << "ok" << endl;

	cout << "catching facet not exist exception with AMI... " << flush;

	ThrowerPrx thrower2 = ThrowerPrx::uncheckedCast(thrower, "no such facet");
	{
	    AMI_Thrower_throwAasAFacetNotExistIPtr cb = new AMI_Thrower_throwAasAFacetNotExistI;
	    thrower2->throwAasA_async(cb, 1);
	    test(cb->check());
	}

	ThrowerPrx thrower3 = ThrowerPrx::uncheckedCast(thrower2, "no such facet either");
	{
	    AMI_Thrower_throwAasAFacetNotExist2IPtr cb = new AMI_Thrower_throwAasAFacetNotExist2I;
	    thrower3->throwAasA_async(cb, 1);
	    test(cb->check());
	}

	cout << "ok" << endl;

	cout << "catching operation not exist exception with AMI... " << flush;

	{
	    AMI_WrongOperation_noSuchOperationIPtr cb = new AMI_WrongOperation_noSuchOperationI;
	    WrongOperationPrx thrower2 = WrongOperationPrx::uncheckedCast(thrower);
	    thrower2->noSuchOperation_async(cb);
	    test(cb->check());
	}

	cout << "ok" << endl;
    
	cout << "catching unknown local exception with AMI... " << flush;

	{
	    AMI_Thrower_throwLocalExceptionIPtr cb = new AMI_Thrower_throwLocalExceptionI;
	    thrower->throwLocalException_async(cb);
	    test(cb->check());
	}
	
	cout << "ok" << endl;

	cout << "catching unknown non-Ice exception with AMI... " << flush;
	
	AMI_Thrower_throwNonIceExceptionIPtr cb = new AMI_Thrower_throwNonIceExceptionI;
	thrower->throwNonIceException_async(cb);
	test(cb->check());
	
	cout << "ok" << endl;
	
	communicator->removeUserExceptionFactory("::A");
	communicator->removeUserExceptionFactory("::B");
	communicator->removeUserExceptionFactory("::C");
	communicator->removeUserExceptionFactory("::D");

    }

    return thrower;
}
