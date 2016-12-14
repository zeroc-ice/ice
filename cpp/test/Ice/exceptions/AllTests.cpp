// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/IceUtil.h>
#include <Ice/Ice.h>
#include <TestCommon.h>
#include <Test.h>

using namespace std;
using namespace Test;

namespace
{
const bool printException = false;
}


class EmptyI : public virtual Empty
{
};

class ServantLocatorI : public virtual Ice::ServantLocator
{
public:

#ifdef ICE_CPP11_MAPPING
    virtual shared_ptr<Ice::Object> locate(const Ice::Current&, shared_ptr<void>&) { return nullptr; }
    virtual void finished(const Ice::Current&, const shared_ptr<Ice::Object>&, const shared_ptr<void>&) {}
    virtual void deactivate(const string&) {}
#else
    virtual Ice::ObjectPtr locate(const Ice::Current&, Ice::LocalObjectPtr&) { return 0; }
    virtual void finished(const Ice::Current&, const Ice::ObjectPtr&, const Ice::LocalObjectPtr&) {}
    virtual void deactivate(const string&) {}
#endif
};

#ifndef ICE_CPP11_MAPPING // C++98
class ValueFactoryI : public virtual Ice::ValueFactory
{
public:
    virtual Ice::ObjectPtr create(const string&) { return 0; }
};
#endif

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

    void check()
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
        while(!_called)
        {
            wait();
        }
        _called = false;
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

class Callback : public CallbackBase, public IceUtil::Shared
{
public:

    Callback()
    {
    }

    Callback(const Ice::CommunicatorPtr& c)
        : _communicator(c)
    {
    }

    virtual void response()
    {
        test(false);
    }

    virtual void exception_AasA(const Ice::Exception& exc)
    {
        const A* ex = dynamic_cast<const A*>(&exc);
        test(ex);
        test(ex->aMem == 1);
        called();
    }

    virtual void exception_AorDasAorD(const Ice::Exception& exc)
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

    virtual void exception_BasB(const Ice::Exception& exc)
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

    virtual void exception_CasC(const Ice::Exception& exc)
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

    virtual void exception_ModA(const Ice::Exception& exc)
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

    virtual void exception_BasA(const Ice::Exception& exc)
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

    virtual void exception_CasA(const Ice::Exception& exc)
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

    virtual void exception_CasB(const Ice::Exception& exc)
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

    virtual void exception_UndeclaredA(const Ice::Exception& exc)
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

    virtual void exception_UndeclaredB(const Ice::Exception& exc)
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

    virtual void exception_UndeclaredC(const Ice::Exception& exc)
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

    virtual void exception_AasAObjectNotExist(const Ice::Exception& exc)
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

    virtual void exception_AasAFacetNotExist(const Ice::Exception& exc)
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

    virtual void exception_noSuchOperation(const Ice::Exception& exc)
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

    virtual void exception_LocalException(const Ice::Exception& exc)
    {
        try
        {
            exc.ice_throw();
        }
        catch(const Ice::UnknownLocalException&)
        {
        }
        catch(const Ice::OperationNotExistException&)
        {
        }
        catch(...)
        {
            test(false);
        }
        called();
    }

    virtual void exception_NonIceException(const Ice::Exception& exc)
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

private:

    Ice::CommunicatorPtr _communicator;
};

typedef IceUtil::Handle<Callback> CallbackPtr;

bool
endsWith(const string& s, const string& findme)
{
    if(s.length() > findme.length())
    {
        return 0 == s.compare(s.length() - findme.length(), findme.length(), findme);
    }
    return false;
}

ThrowerPrxPtr
allTests(const Ice::CommunicatorPtr& communicator)
{
    cout << "testing ice_print()/what()... " << flush;
    {
        A a;
        string aMsg = "::Test::A";

        Ice::UnknownLocalException ule("thisFile", 99);
        string uleMsg = "thisFile:99: ::Ice::UnknownLocalException:\nunknown local exception";

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

        {
            E ex("E");
            ostringstream os;
            ex.ice_print(os);
            test(os.str() == "::Test::E");
            test(ex.data == "E");
        }

        //
        // Test custom ice_print
        //
        {
            F ex("F");
            ostringstream os;
            ex.ice_print(os);
            test(os.str() == "::Test::F data:'F'");
            test(ex.data == "F");
        }

        {
            G ex(__FILE__, __LINE__, "G");
            ostringstream os;
            ex.ice_print(os);
            test(endsWith(os.str(), "Test::G"));
            test(ex.data == "G");
        }

        {
            H ex(__FILE__, __LINE__, "H");
            ostringstream os;
            ex.ice_print(os);
            test(endsWith(os.str(), "Test::H data:'H'"));
            test(ex.data == "H");
        }

    }
    cout << "ok" << endl;

    string localOAEndpoint;
    {
        ostringstream ostr;
        if(communicator->getProperties()->getProperty("Ice.Default.Protocol") == "bt")
        {
            ostr << "default -a *";
        }
        else
        {
            ostr << "default -h *";
        }
        localOAEndpoint = ostr.str();
    }
#ifdef ICE_OS_UWP
    bool uwp = true;
#else
    bool uwp = false;
#endif

    if(!uwp || (communicator->getProperties()->getProperty("Ice.Default.Protocol") != "ssl" &&
                  communicator->getProperties()->getProperty("Ice.Default.Protocol") != "wss"))
    {
        cout << "testing object adapter registration exceptions... " << flush;
        {
            Ice::ObjectAdapterPtr first;
            try
            {
                first = communicator->createObjectAdapter("TestAdapter0");
                test(false);
            }
            catch(const Ice::InitializationException& ex)
            {
                if(printException)
                {
                    Ice::Print printer(communicator->getLogger());
                    printer << ex;
                }
                // Expected
            }

            communicator->getProperties()->setProperty("TestAdapter0.Endpoints", localOAEndpoint);
            first = communicator->createObjectAdapter("TestAdapter0");
            try
            {
                Ice::ObjectAdapterPtr second = communicator->createObjectAdapter("TestAdapter0");
                test(false);
            }
            catch(const Ice::AlreadyRegisteredException& ex)
            {
                if(printException)
                {
                    Ice::Print printer(communicator->getLogger());
                    printer << ex;
                }

                // Expected
            }

            try
            {
                Ice::ObjectAdapterPtr second =
                    communicator->createObjectAdapterWithEndpoints("TestAdapter0", "ssl -h foo -p 12011");
                test(false);
            }
            catch(const Ice::AlreadyRegisteredException& ex)
            {
                if(printException)
                {
                    Ice::Print printer(communicator->getLogger());
                    printer << ex;
                }

                // Expected.
            }
            first->deactivate();
        }
        cout << "ok" << endl;

        cout << "testing servant registration exceptions... " << flush;
        {
            communicator->getProperties()->setProperty("TestAdapter1.Endpoints", localOAEndpoint);
            Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter1");
            Ice::ObjectPtr obj = ICE_MAKE_SHARED(EmptyI);
            adapter->add(obj, Ice::stringToIdentity("x"));
            try
            {
                adapter->add(obj, Ice::stringToIdentity("x"));
                test(false);
            }
            catch(const Ice::AlreadyRegisteredException& ex)
            {
                if(printException)
                {
                    Ice::Print printer(communicator->getLogger());
                    printer << ex;
                }
            }

            try
            {
                adapter->add(obj, Ice::stringToIdentity(""));
            }
            catch(const Ice::IllegalIdentityException& ex)
            {
                test(ex.id.name == "");
                if(printException)
                {
                    Ice::Print printer(communicator->getLogger());
                    printer << ex;
                }
            }

            try
            {
                adapter->add(0, Ice::stringToIdentity("x"));
            }
            catch(const Ice::IllegalServantException& ex)
            {
                if(printException)
                {
                    Ice::Print printer(communicator->getLogger());
                    printer << ex;
                }
            }

            adapter->remove(Ice::stringToIdentity("x"));
            try
            {
                adapter->remove(Ice::stringToIdentity("x"));
                test(false);
            }
            catch(const Ice::NotRegisteredException& ex)
            {
                if(printException)
                {
                    Ice::Print printer(communicator->getLogger());
                    printer << ex;
                }
            }

            adapter->deactivate();
        }
        cout << "ok" << endl;

        cout << "testing servant locator registrations exceptions... " << flush;
        {
            communicator->getProperties()->setProperty("TestAdapter2.Endpoints", localOAEndpoint);
            Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter2");
            Ice::ServantLocatorPtr loc = ICE_MAKE_SHARED(ServantLocatorI);
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
    }

    cout << "testing value factory registration exception... " << flush;
    {
#ifdef ICE_CPP11_MAPPING
        communicator->getValueFactoryManager()->add(
            [](const std::string&)
            {
                return nullptr;
            },
            "x");
        try
        {
            communicator->getValueFactoryManager()->add(
                [](const std::string&)
                {
                    return nullptr;
                },
                "x");
            test(false);
        }
        catch(const Ice::AlreadyRegisteredException&)
        {
        }
#else
        Ice::ValueFactoryPtr vf = new ValueFactoryI;
        communicator->getValueFactoryManager()->add(vf, "x");
        try
        {
            communicator->getValueFactoryManager()->add(vf, "x");
            test(false);
        }
        catch(const Ice::AlreadyRegisteredException&)
        {
        }
#endif
    }
    cout << "ok" << endl;

    cout << "testing stringToProxy... " << flush;
    string ref = "thrower:" + getTestEndpoint(communicator, 0);
    Ice::ObjectPrxPtr base = communicator->stringToProxy(ref);
    test(base);
    cout << "ok" << endl;

    cout << "testing checked cast... " << flush;
    ThrowerPrxPtr thrower = ICE_CHECKED_CAST(ThrowerPrx, base);
    test(thrower);
#ifdef ICE_CPP11_MAPPING
    test(Ice::targetEqualTo(thrower, base));
#else
    test(thrower == base);
#endif
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
        catch(const Ice::UnknownUserException&)
        {
        }
        catch(const Ice::Exception& ex)
        {
            cout << ex << endl;
            cout << ex.ice_stackTrace() << endl;
            test(false);
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
        catch(const Ice::UnknownUserException&)
        {
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
        catch(const Ice::UnknownUserException&)
        {
        }
        catch(...)
        {
            test(false);
        }

        cout << "ok" << endl;
    }

    if(thrower->ice_getConnection())
    {
        cout << "testing memory limit marshal exception..." << flush;
        try
        {
            thrower->throwMemoryLimitException(Ice::ByteSeq());
            test(false);
        }
        catch(const Ice::MemoryLimitException&)
        {
        }
        catch(...)
        {
            test(false);
        }

        try
        {
            thrower->throwMemoryLimitException(Ice::ByteSeq(20 * 1024)); // 20KB
            test(false);
        }
        catch(const Ice::ConnectionLostException&)
        {
        }
        catch(const Ice::LocalException& ex)
        {
            cerr << ex << endl;
            test(false);
        }

        ThrowerPrxPtr thrower2 =
            ICE_UNCHECKED_CAST(ThrowerPrx, communicator->stringToProxy("thrower:" + getTestEndpoint(communicator, 1)));
        try
        {
            thrower2->throwMemoryLimitException(Ice::ByteSeq(2 * 1024 * 1024)); // 2MB (no limits)
        }
        catch(const Ice::MemoryLimitException&)
        {
        }
        ThrowerPrxPtr thrower3 =
            ICE_UNCHECKED_CAST(ThrowerPrx, communicator->stringToProxy("thrower:" + getTestEndpoint(communicator, 2)));
        try
        {
            thrower3->throwMemoryLimitException(Ice::ByteSeq(1024)); // 1KB limit
            test(false);
        }
        catch(const Ice::ConnectionLostException&)
        {
        }

        cout << "ok" << endl;
    }

    cout << "catching object not exist exception... " << flush;

    Ice::Identity id = Ice::stringToIdentity("does not exist");
    try
    {
        ThrowerPrxPtr thrower2 = ICE_UNCHECKED_CAST(ThrowerPrx, thrower->ice_identity(id));
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
#ifdef ICE_CPP11_MAPPING
        ThrowerPrxPtr thrower2 = Ice::uncheckedCast<ThrowerPrx>(thrower, "no such facet");
#else
        ThrowerPrxPtr thrower2 = ThrowerPrx::uncheckedCast(thrower, "no such facet");
#endif
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
        WrongOperationPrxPtr thrower2 = ICE_UNCHECKED_CAST(WrongOperationPrx, thrower);
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
    catch(const Ice::UnknownLocalException&)
    {
    }
    catch(...)
    {
        test(false);
    }
    try
    {
        thrower->throwLocalExceptionIdempotent();
        test(false);
    }
    catch(const Ice::UnknownLocalException&)
    {
    }
    catch(const Ice::OperationNotExistException&)
    {
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
    }
    catch(...)
    {
        test(false);
    }

    cout << "ok" << endl;

    cout << "testing asynchronous exceptions... " << flush;

    try
    {
        thrower->throwAfterResponse();
    }
    catch(...)
    {
        test(false);
    }

    try
    {
        thrower->throwAfterException();
        test(false);
    }
    catch(const A&)
    {
    }
    catch(...)
    {
        test(false);
    }

    cout << "ok" << endl;

    cout << "catching exact types with new AMI mapping... " << flush;
#ifdef ICE_CPP11_MAPPING
    {
        auto f = thrower->throwAasAAsync(1);
        try
        {
            f.get();
            test(false);
        }
        catch(const A& ex)
        {
            test(ex.aMem == 1);
        }
        catch(const Ice::Exception&)
        {
            test(false);
        }
        catch(...)
        {
            test(false);
        }
    }

    {
        auto f = thrower->throwAorDasAorDAsync(1);
        try
        {
            f.get();
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
    }

    {
        auto f = thrower->throwAorDasAorDAsync(-1);
        try
        {
            f.get();
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
    }
    {
        auto f = thrower->throwBasBAsync(1, 2);
        try
        {
            f.get();
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
    }
    {
        auto f = thrower->throwCasCAsync(1, 2, 3);
        try
        {
            f.get();
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
    }
    {
        auto f = thrower->throwModAAsync(1, 2);
        try
        {
            f.get();
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
    }

    //
    // repeat with callback API and no exception callback
    //
    {
        promise<bool> sent;
        thrower->throwAasAAsync(1,
            []()
            {
                test(false);
            },
            nullptr,
            [&](bool value)
            {
                sent.set_value(value);
            });
        sent.get_future().get(); // Wait for sent
    }

    {
        promise<bool> sent;
        thrower->throwAorDasAorDAsync(1,
            []()
            {
                test(false);
            },
            nullptr,
            [&](bool value)
            {
                sent.set_value(value);
            });
        sent.get_future().get(); // Wait for sent
    }

    {
        promise<bool> sent;
        thrower->throwAorDasAorDAsync(-1,
            []()
            {
                test(false);
            },
            nullptr,
            [&](bool value)
            {
                sent.set_value(value);
            });
        sent.get_future().get(); // Wait for sent
    }

    {
        promise<bool> sent;
        thrower->throwBasBAsync(1, 2,
            []()
            {
                test(false);
            },
            nullptr,
            [&](bool value)
            {
                sent.set_value(value);
            });
        sent.get_future().get(); // Wait for sent
    }

    {
        promise<bool> sent;
        thrower->throwCasCAsync(1, 2, 3,
            []()
            {
                test(false);
            },
            nullptr,
            [&](bool value)
            {
                sent.set_value(value);
            });
        sent.get_future().get(); // Wait for sent
    }

    {
        promise<bool> sent;
        thrower->throwModAAsync(1, 2,
            []()
            {
                test(false);
            },
            nullptr,
            [&](bool value)
            {
                sent.set_value(value);
            });
        sent.get_future().get(); // Wait for sent
    }
#else
    {
        CallbackPtr cb = new Callback;
        Callback_Thrower_throwAasAPtr callback =
            newCallback_Thrower_throwAasA(cb, &Callback::response, &Callback::exception_AasA);
        thrower->begin_throwAasA(1, callback);
        cb->check();
    }
    {
        CallbackPtr cb = new Callback;
        Callback_Thrower_throwAorDasAorDPtr callback =
            newCallback_Thrower_throwAorDasAorD(cb, &Callback::response, &Callback::exception_AorDasAorD);
        thrower->begin_throwAorDasAorD(1, callback);
        cb->check();
    }
    {
        CallbackPtr cb = new Callback;
        Callback_Thrower_throwAorDasAorDPtr callback =
            newCallback_Thrower_throwAorDasAorD(cb, &Callback::response, &Callback::exception_AorDasAorD);
        thrower->begin_throwAorDasAorD(-1, callback);
        cb->check();
    }
    {
        CallbackPtr cb = new Callback;
        Callback_Thrower_throwBasBPtr callback =
            newCallback_Thrower_throwBasB(cb, &Callback::response, &Callback::exception_BasB);
        thrower->begin_throwBasB(1, 2, callback);
        cb->check();
    }
    {
        CallbackPtr cb = new Callback;
        Callback_Thrower_throwCasCPtr callback =
            newCallback_Thrower_throwCasC(cb, &Callback::response, &Callback::exception_CasC);
        thrower->begin_throwCasC(1, 2, 3, callback);
        cb->check();
    }
    {
        CallbackPtr cb = new Callback;
        Callback_Thrower_throwModAPtr callback =
            newCallback_Thrower_throwModA(cb, &Callback::response, &Callback::exception_ModA);
        thrower->begin_throwModA(1, 2, callback);
        cb->check();
    }
#endif
    cout << "ok" << endl;

    cout << "catching derived types with new AMI mapping... " << flush;
#ifdef ICE_CPP11_MAPPING
    {
        auto f = thrower->throwBasAAsync(1, 2);
        try
        {
            f.get();
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
    }

    {
        auto f = thrower->throwCasAAsync(1, 2, 3);
        try
        {
            f.get();
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
    }

    {
        auto f = thrower->throwCasBAsync(1, 2, 3);
        try
        {
            f.get();
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
    }
#else
    {
        CallbackPtr cb = new Callback;
        Callback_Thrower_throwBasAPtr callback =
            newCallback_Thrower_throwBasA(cb, &Callback::response, &Callback::exception_BasA);
        thrower->begin_throwBasA(1, 2, callback);
        cb->check();
    }

    {
        CallbackPtr cb = new Callback;
        Callback_Thrower_throwCasAPtr callback =
            newCallback_Thrower_throwCasA(cb, &Callback::response, &Callback::exception_CasA);
        thrower->begin_throwCasA(1, 2, 3, callback);
        cb->check();
    }

    {
        CallbackPtr cb = new Callback;
        Callback_Thrower_throwCasBPtr callback =
            newCallback_Thrower_throwCasB(cb, &Callback::response, &Callback::exception_CasB);
        thrower->begin_throwCasB(1, 2, 3, callback);
        cb->check();
    }
#endif
    cout << "ok" << endl;

    if(thrower->supportsUndeclaredExceptions())
    {
        cout << "catching unknown user exception with new AMI mapping... " << flush;
#ifdef ICE_CPP11_MAPPING
        {
            auto f = thrower->throwUndeclaredAAsync(1);
            try
            {
                f.get();
                test(false);
            }
            catch(const Ice::UnknownUserException&)
            {
            }
            catch(const Ice::Exception& ex)
            {
                cout << ex << endl;
                cout << ex.ice_stackTrace() << endl;
                test(false);
            }
            catch(...)
            {
                test(false);
            }
        }

        {
            auto f = thrower->throwUndeclaredBAsync(1, 2);
            try
            {
                f.get();
                test(false);
            }
            catch(const Ice::UnknownUserException&)
            {
            }
            catch(...)
            {
                test(false);
            }
        }

        {
            auto f = thrower->throwUndeclaredCAsync(1, 2, 3);
            try
            {
                f.get();
            }
            catch(const Ice::UnknownUserException&)
            {
            }
            catch(...)
            {
                test(false);
            }
        }
#else
        {
            CallbackPtr cb = new Callback;
            Callback_Thrower_throwUndeclaredAPtr callback =
                newCallback_Thrower_throwUndeclaredA(cb, &Callback::response, &Callback::exception_UndeclaredA);
            thrower->begin_throwUndeclaredA(1, callback);
            cb->check();
        }

        {
            CallbackPtr cb = new Callback;
            Callback_Thrower_throwUndeclaredBPtr callback =
                newCallback_Thrower_throwUndeclaredB(cb, &Callback::response, &Callback::exception_UndeclaredB);
            thrower->begin_throwUndeclaredB(1, 2, callback);
            cb->check();
        }

        {
            CallbackPtr cb = new Callback;
            Callback_Thrower_throwUndeclaredCPtr callback =
                newCallback_Thrower_throwUndeclaredC(cb, &Callback::response, &Callback::exception_UndeclaredC);
            thrower->begin_throwUndeclaredC(1, 2, 3, callback);
            cb->check();
        }
#endif
        cout << "ok" << endl;
    }

    cout << "catching object not exist exception with new AMI mapping... " << flush;

    {
#ifdef ICE_CPP11_MAPPING
        id = Ice::stringToIdentity("does not exist");
        shared_ptr<ThrowerPrx> thrower2 = Ice::uncheckedCast<ThrowerPrx>(thrower->ice_identity(id));
        auto f = thrower2->throwAasAAsync(1);
        try
        {
            f.get();
        }
        catch(const Ice::ObjectNotExistException& ex)
        {
            test(ex.id == id);
        }
        catch(...)
        {
            test(false);
        }
#else
        id = Ice::stringToIdentity("does not exist");
        ThrowerPrx thrower2 = ThrowerPrx::uncheckedCast(thrower->ice_identity(id));
        CallbackPtr cb = new Callback(communicator);
        Callback_Thrower_throwAasAPtr callback =
            newCallback_Thrower_throwAasA(cb, &Callback::response, &Callback::exception_AasAObjectNotExist);
        thrower2->begin_throwAasA(1, callback);
        cb->check();
#endif
    }

    cout << "ok" << endl;

    cout << "catching facet not exist exception with new AMI mapping... " << flush;

    {
#ifdef ICE_CPP11_MAPPING
        shared_ptr<ThrowerPrx> thrower2 = Ice::uncheckedCast<ThrowerPrx>(thrower, "no such facet");
        auto f = thrower2->throwAasAAsync(1);
        try
        {
            f.get();
        }
        catch(const Ice::FacetNotExistException& ex)
        {
            test(ex.facet == "no such facet");
        }
#else
        ThrowerPrx thrower2 = ThrowerPrx::uncheckedCast(thrower, "no such facet");
        CallbackPtr cb = new Callback;
        Callback_Thrower_throwAasAPtr callback =
            newCallback_Thrower_throwAasA(cb, &Callback::response, &Callback::exception_AasAFacetNotExist);
        thrower2->begin_throwAasA(1, callback);
        cb->check();
#endif
    }

    cout << "ok" << endl;

    cout << "catching operation not exist exception with new AMI mapping... " << flush;

    {
#ifdef ICE_CPP11_MAPPING
        shared_ptr<WrongOperationPrx> thrower4 = Ice::uncheckedCast<WrongOperationPrx>(thrower);
        auto f = thrower4->noSuchOperationAsync();
        try
        {
            f.get();
        }
        catch(const Ice::OperationNotExistException& ex)
        {
            test(ex.operation == "noSuchOperation");
        }
        catch(...)
        {
            test(false);
        }
#else
        CallbackPtr cb = new Callback;
        Callback_WrongOperation_noSuchOperationPtr callback =
            newCallback_WrongOperation_noSuchOperation(cb, &Callback::response,
                                                       &Callback::exception_noSuchOperation);
        WrongOperationPrx thrower4 = WrongOperationPrx::uncheckedCast(thrower);
        thrower4->begin_noSuchOperation(callback);
        cb->check();
#endif
    }

    cout << "ok" << endl;

    cout << "catching unknown local exception with new AMI mapping... " << flush;
#ifdef ICE_CPP11_MAPPING
    {
        auto f = thrower->throwLocalExceptionAsync();
        try
        {
            f.get();
            test(false);
        }
        catch(const Ice::UnknownLocalException&)
        {
        }
        catch(...)
        {
            test(false);
        }
    }

    {
        auto f = thrower->throwLocalExceptionIdempotentAsync();
        try
        {
            f.get();
            test(false);
        }
        catch(const Ice::UnknownLocalException&)
        {
        }
        catch(const Ice::OperationNotExistException&)
        {
        }
        catch(...)
        {
            test(false);
        }
    }
#else
    {
        CallbackPtr cb = new Callback;
        Callback_Thrower_throwLocalExceptionPtr callback =
            newCallback_Thrower_throwLocalException(cb, &Callback::response, &Callback::exception_LocalException);
        thrower->begin_throwLocalException(callback);
        cb->check();
    }

    {
        CallbackPtr cb = new Callback;
        Callback_Thrower_throwLocalExceptionIdempotentPtr callback =
            newCallback_Thrower_throwLocalExceptionIdempotent(cb, &Callback::response,
                                                              &Callback::exception_LocalException);
        thrower->begin_throwLocalExceptionIdempotent(callback);
        cb->check();
    }
#endif
    cout << "ok" << endl;

    cout << "catching unknown non-Ice exception with new AMI mapping... " << flush;

    {
#ifdef ICE_CPP11_MAPPING
        auto f = thrower->throwNonIceExceptionAsync();
        try
        {
            f.get();
            test(false);
        }
        catch(const Ice::UnknownException&)
        {
        }
        catch(...)
        {
            test(false);
        }

#else
        CallbackPtr cb = new Callback;
        Callback_Thrower_throwNonIceExceptionPtr callback =
            newCallback_Thrower_throwNonIceException(cb, &Callback::response, &Callback::exception_NonIceException);
        thrower->begin_throwNonIceException(callback);
        cb->check();
#endif
    }

    cout << "ok" << endl;

    return thrower;
}
