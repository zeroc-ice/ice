// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestCommon.h>
#include <Test.h>

using namespace std;
using namespace Test;

class EmptyI : virtual public Empty
{
};

class ServantLocatorI : virtual public Ice::ServantLocator
{
public:

    virtual Ice::ObjectPtr locate(const Ice::Current&, Ice::LocalObjectPtr&) { return 0; }
    virtual void finished(const Ice::Current&, const Ice::ObjectPtr&, const Ice::LocalObjectPtr&) {}
    virtual void deactivate(const string&) {}
};

class ObjectFactoryI : virtual public Ice::ObjectFactory
{
public:
    virtual Ice::ObjectPtr create(const string&) { return 0; }
    virtual void destroy() {}
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
        _called = false;
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

typedef IceUtil::Handle<AMI_Thrower_throwAasAI> AMI_Thrower_throwAasAIPtr;

class AMI_Thrower_throwAasAObjectNotExistI : public AMI_Thrower_throwAasA, public CallbackBase
{
public:

    AMI_Thrower_throwAasAObjectNotExistI(const Ice::CommunicatorPtr& communicator) :
        _communicator(communicator)
    {
    }

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
            Ice::Identity id = _communicator->stringToIdentity("does not exist");
            test(ex.id == id);
        }
        catch(...)
        {
            test(false);
        }
        called();
    }

private:

    Ice::CommunicatorPtr _communicator;
};

typedef IceUtil::Handle<AMI_Thrower_throwAasAObjectNotExistI> AMI_Thrower_throwAasAObjectNotExistIPtr;

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
            test(ex.facet == "no such facet");
        }
        catch(...)
        {
            test(false);
        }
        called();
    }
};

typedef IceUtil::Handle<AMI_Thrower_throwAasAFacetNotExistI> AMI_Thrower_throwAasAFacetNotExistIPtr;

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

typedef IceUtil::Handle<AMI_Thrower_throwAorDasAorDI> AMI_Thrower_throwAorDasAorDIPtr;

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

typedef IceUtil::Handle<AMI_Thrower_throwBasAI> AMI_Thrower_throwBasAIPtr;

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

typedef IceUtil::Handle<AMI_Thrower_throwCasAI> AMI_Thrower_throwCasAIPtr;

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

typedef IceUtil::Handle<AMI_Thrower_throwBasBI> AMI_Thrower_throwBasBIPtr;

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

typedef IceUtil::Handle<AMI_Thrower_throwCasBI> AMI_Thrower_throwCasBIPtr;

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

typedef IceUtil::Handle<AMI_Thrower_throwCasCI> AMI_Thrower_throwCasCIPtr;


class AMI_Thrower_throwModAI : public AMI_Thrower_throwModA, public CallbackBase
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
        catch(const Mod::A& ex)
        {
            test(ex.aMem == 1);
            test(ex.a2Mem == 2);
        }
        catch(const Ice::OperationNotExistException&)
        {
            //
            // This operation is not supported in Java.
            //
        }
        catch(...)
        {
            test(false);
        }
        called();
    }
};

typedef IceUtil::Handle<AMI_Thrower_throwModAI> AMI_Thrower_throwModAIPtr;



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

typedef IceUtil::Handle<AMI_Thrower_throwUndeclaredAI> AMI_Thrower_throwUndeclaredAIPtr;

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

typedef IceUtil::Handle<AMI_Thrower_throwUndeclaredBI> AMI_Thrower_throwUndeclaredBIPtr;

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

typedef IceUtil::Handle<AMI_Thrower_throwUndeclaredCI> AMI_Thrower_throwUndeclaredCIPtr;

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

typedef IceUtil::Handle<AMI_Thrower_throwLocalExceptionI> AMI_Thrower_throwLocalExceptionIPtr;

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

typedef IceUtil::Handle<AMI_Thrower_throwNonIceExceptionI> AMI_Thrower_throwNonIceExceptionIPtr;

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

typedef IceUtil::Handle<AMI_WrongOperation_noSuchOperationI> AMI_WrongOperation_noSuchOperationIPtr;

ThrowerPrx
allTests(const Ice::CommunicatorPtr& communicator, bool collocated)
{
    cout << "testing ice_print()/what()... " << flush;
    {
        A a;
        string aMsg = "Test::A";

        Ice::UnknownLocalException ule("thisFile", 99);
        string uleMsg = "thisFile:99: Ice::UnknownLocalException:\nunknown local exception";

        //
        // Test ice_print().
        //
        {
            stringstream str;
            a.ice_print(str);
            test(str.str() == aMsg);
        }
        {
            stringstream str;
            ule.ice_print(str);
            test(str.str() == uleMsg);
        }

        //
        // Test operator<<().
        //
        {
            stringstream str;
            str << a;
            test(str.str() == aMsg);
        }
        {
            stringstream str;
            str << ule;
            test(str.str() == uleMsg);
        }

        //
        // Test what(). (Called twice because of lazy initialization in what().)
        //
        test(aMsg == a.what());
        test(aMsg == a.what());

        test(uleMsg == ule.what());
        test(uleMsg == ule.what());
    }
    cout << "ok" << endl;

    cout << "testing object adapter registration exceptions... " << flush;
    {
        Ice::ObjectAdapterPtr first;
        try
        {
            first = communicator->createObjectAdapter("TestAdapter0");
            test(false);
        }
        catch(const Ice::InitializationException&)
        {
            // Expected
        }

        communicator->getProperties()->setProperty("TestAdapter0.Endpoints", "default");
        first = communicator->createObjectAdapter("TestAdapter0");
        try
        {
            Ice::ObjectAdapterPtr second = communicator->createObjectAdapter("TestAdapter0");
            test(false);
        }
        catch(const Ice::AlreadyRegisteredException&)
        {
            // Expected
        }

        try
        {
            Ice::ObjectAdapterPtr second = 
                communicator->createObjectAdapterWithEndpoints("TestAdapter0", "ssl -h foo -p 12011 -t 10000");
            test(false);
        }
        catch(const Ice::AlreadyRegisteredException&)
        {
            // Expected.
        }
        //
        // Properties must remain unaffected if an exception occurs.
        //
        test(communicator->getProperties()->getProperty("TestAdapter0.Endpoints") == "default");
        first->deactivate();
    }
    cout << "ok" << endl;
    
    cout << "testing servant registration exceptions... " << flush;
    {
        communicator->getProperties()->setProperty("TestAdapter1.Endpoints", "default");
        Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter1");
        Ice::ObjectPtr obj = new EmptyI;
        adapter->add(obj, communicator->stringToIdentity("x"));
        try
        {
            adapter->add(obj, communicator->stringToIdentity("x"));
            test(false);
        }
        catch(const Ice::AlreadyRegisteredException&)
        {
        }

        adapter->remove(communicator->stringToIdentity("x"));
        try
        {
            adapter->remove(communicator->stringToIdentity("x"));
            test(false);
        }
        catch(const Ice::NotRegisteredException&)
        {
        }

        adapter->deactivate();
    }
    cout << "ok" << endl;

    cout << "testing servant locator registrations exceptions... " << flush;
    {
        communicator->getProperties()->setProperty("TestAdapter2.Endpoints", "default");
        Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter2");
        Ice::ServantLocatorPtr loc = new ServantLocatorI;
        adapter->addServantLocator(loc, "x");
        try
        {
            adapter->addServantLocator(loc, "x");
            test(false);
        }
        catch(const Ice::AlreadyRegisteredException&)
        {
        }

        adapter->deactivate();
    }
    cout << "ok" << endl;

    cout << "testing object factory registration exception... " << flush;
    {
        Ice::ObjectFactoryPtr of = new ObjectFactoryI;
        communicator->addObjectFactory(of, "x");
        try
        {
            communicator->addObjectFactory(of, "x");
            test(false);
        }
        catch(const Ice::AlreadyRegisteredException&)
        {
        }
    }
    cout << "ok" << endl;

    cout << "testing stringToProxy... " << flush;
    string ref = "thrower:default -p 12010 -t 10000";
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

#if (!defined(_MSC_VER) || _MSC_VER >= 1300)
//
// With VC6 SP5, there is no way to call ::A::__write from ::Mod::A
//
    try
    {
        thrower->throwModA(1, 2);
        test(false);
    }
    catch(const Mod::A& ex)
    {
        test(ex.aMem == 1);
        test(ex.a2Mem == 2);
    }
    catch(const Ice::OperationNotExistException&)
    {
        //
        // This operation is not supported in Java.
        //
    }
    catch(...)
    {
        test(false);
    }
#endif

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

#if (!defined(_MSC_VER) || _MSC_VER >= 1300)
//
// With VC6 SP5, there is no way to call ::A::__write from ::Mod::A
//
    try
    {
        thrower->throwModA(1, 2);
        test(false);
    }
    catch(const A& ex)
    {
        test(ex.aMem == 1);
    }
    catch(const Ice::OperationNotExistException&)
    {
        //
        // This operation is not supported in Java.
        //
    }
    catch(...)
    {
        test(false);
    }
#endif

    cout << "ok" << endl;

    cout << "catching derived types... " << flush;
        
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
            // We get an unknown user exception without collocation
            // optimization.
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
            // We get an unknown user exception without collocation
            // optimization.
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
            // We get an unknown user exception without
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

    Ice::Identity id = communicator->stringToIdentity("does not exist");
    try
    {
        ThrowerPrx thrower2 = ThrowerPrx::uncheckedCast(thrower->ice_identity(id));
        thrower2->throwAasA(1);
//      thrower2->ice_ping();
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
            test(ex.facet == "no such facet");
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
        // We get an unknown local exception without collocation
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
        // We get an unknown exception without collocation
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

    if(!collocated)
    {
        cout << "catching exact types with AMI... " << flush;

        {
            AMI_Thrower_throwAasAIPtr cb = new AMI_Thrower_throwAasAI;
            thrower->throwAasA_async(cb, 1);
            test(cb->check());
            // Let's check if we can reuse the same callback object for another call.
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
            // Let's check if we can reuse the same callback object for another call.
            thrower->throwCasC_async(cb, 1, 2, 3);
            test(cb->check());
        }
        
#if (!defined(_MSC_VER) || _MSC_VER >= 1300)
//
// With VC6 SP5, there is no way to call ::A::__write from ::Mod::A
//
        {
            AMI_Thrower_throwModAIPtr cb = new AMI_Thrower_throwModAI;
            thrower->throwModA_async(cb, 1, 2);
            test(cb->check());
        }
#endif

        cout << "ok" << endl;
        
        cout << "catching derived types... " << flush;
        
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
            id = communicator->stringToIdentity("does not exist");
            ThrowerPrx thrower2 = ThrowerPrx::uncheckedCast(thrower->ice_identity(id));
            AMI_Thrower_throwAasAObjectNotExistIPtr cb = new AMI_Thrower_throwAasAObjectNotExistI(communicator);
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

        cout << "ok" << endl;

        cout << "catching operation not exist exception with AMI... " << flush;

        {
            AMI_WrongOperation_noSuchOperationIPtr cb = new AMI_WrongOperation_noSuchOperationI;
            WrongOperationPrx thrower4 = WrongOperationPrx::uncheckedCast(thrower);
            thrower4->noSuchOperation_async(cb);
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
        
    }

    return thrower;
}
