//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "IceUtil/IceUtil.h"
#include "Ice/Ice.h"
#include "TestHelper.h"
#include "Test.h"

using namespace std;
using namespace Test;

namespace
{
    const bool printException = false;
}

class EmptyI final : public Empty
{
};

class ServantLocatorI final : public Ice::ServantLocator
{
public:
    Ice::ObjectPtr locate(const Ice::Current&, shared_ptr<void>&) final { return nullptr; }
    void finished(const Ice::Current&, const Ice::ObjectPtr&, const shared_ptr<void>&) final {}
    void deactivate(const string&) final {}
};

bool
endsWith(const string& s, const string& findme)
{
    if (s.length() > findme.length())
    {
        return 0 == s.compare(s.length() - findme.length(), findme.length(), findme);
    }
    return false;
}

ThrowerPrx
allTests(Test::TestHelper* helper)
{
    Ice::CommunicatorPtr communicator = helper->communicator();
    const string protocol = communicator->getProperties()->getProperty("Ice.Default.Protocol");

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
    }
    cout << "ok" << endl;

    string localOAEndpoint;
    {
        ostringstream ostr;
        if (communicator->getProperties()->getProperty("Ice.Default.Protocol") == "bt")
        {
            ostr << "default -a *";
        }
        else
        {
            ostr << "default -h *";
        }
        localOAEndpoint = ostr.str();
    }

    if (communicator->getProperties()->getProperty("Ice.Default.Protocol") != "ssl" &&
        communicator->getProperties()->getProperty("Ice.Default.Protocol") != "wss")
    {
        cout << "testing object adapter registration exceptions... " << flush;
        {
            Ice::ObjectAdapterPtr first;
            try
            {
                first = communicator->createObjectAdapter("TestAdapter0");
                test(false);
            }
            catch (const Ice::InitializationException& ex)
            {
                if (printException)
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
            catch (const Ice::AlreadyRegisteredException& ex)
            {
                if (printException)
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
            catch (const Ice::AlreadyRegisteredException& ex)
            {
                if (printException)
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
            Ice::ObjectPtr obj = std::make_shared<EmptyI>();
            adapter->add(obj, Ice::stringToIdentity("x"));
            try
            {
                adapter->add(obj, Ice::stringToIdentity("x"));
                test(false);
            }
            catch (const Ice::AlreadyRegisteredException& ex)
            {
                if (printException)
                {
                    Ice::Print printer(communicator->getLogger());
                    printer << ex;
                }
            }

            try
            {
                adapter->add(obj, Ice::stringToIdentity(""));
            }
            catch (const Ice::IllegalIdentityException& ex)
            {
                if (printException)
                {
                    Ice::Print printer(communicator->getLogger());
                    printer << ex;
                }
            }

            try
            {
                obj = nullptr;
                adapter->add(obj, Ice::stringToIdentity("x"));
            }
            catch (const Ice::IllegalServantException& ex)
            {
                if (printException)
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
            catch (const Ice::NotRegisteredException& ex)
            {
                if (printException)
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
            Ice::ServantLocatorPtr loc = make_shared<ServantLocatorI>();
            adapter->addServantLocator(loc, "x");
            try
            {
                adapter->addServantLocator(loc, "x");
                test(false);
            }
            catch (const Ice::AlreadyRegisteredException&)
            {
            }

            adapter->deactivate();
        }
        cout << "ok" << endl;
    }

    cout << "testing value factory registration exception... " << flush;
    {
        communicator->getValueFactoryManager()->add([](string_view) { return nullptr; }, "x");
        try
        {
            communicator->getValueFactoryManager()->add([](string_view) { return nullptr; }, "x");
            test(false);
        }
        catch (const Ice::AlreadyRegisteredException&)
        {
        }
    }
    cout << "ok" << endl;

    ThrowerPrx thrower(communicator, "thrower:" + helper->getTestEndpoint());

    cout << "catching exact types... " << flush;

    try
    {
        thrower->throwAasA(1);
        test(false);
    }
    catch (const A& ex)
    {
        test(ex.aMem == 1);
    }
    catch (const Ice::Exception& ex)
    {
        cout << ex << endl;
        test(false);
    }
    catch (...)
    {
        test(false);
    }

    try
    {
        thrower->throwAorDasAorD(1);
        test(false);
    }
    catch (const A& ex)
    {
        test(ex.aMem == 1);
    }
    catch (...)
    {
        test(false);
    }

    try
    {
        thrower->throwAorDasAorD(-1);
        test(false);
    }
    catch (const D& ex)
    {
        test(ex.dMem == -1);
    }
    catch (...)
    {
        test(false);
    }

    try
    {
        thrower->throwBasB(1, 2);
        test(false);
    }
    catch (const B& ex)
    {
        test(ex.aMem == 1);
        test(ex.bMem == 2);
    }
    catch (...)
    {
        test(false);
    }

    try
    {
        thrower->throwCasC(1, 2, 3);
        test(false);
    }
    catch (const C& ex)
    {
        test(ex.aMem == 1);
        test(ex.bMem == 2);
        test(ex.cMem == 3);
    }
    catch (...)
    {
        test(false);
    }

    try
    {
        thrower->throwModA(1, 2);
        test(false);
    }
    catch (const Mod::A& ex)
    {
        test(ex.aMem == 1);
        test(ex.a2Mem == 2);
    }
    catch (const Ice::OperationNotExistException&)
    {
        //
        // This operation is not supported in Java.
        //
    }
    catch (...)
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
    catch (const A& ex)
    {
        test(ex.aMem == 1);
    }
    catch (...)
    {
        test(false);
    }

    try
    {
        thrower->throwCasC(1, 2, 3);
        test(false);
    }
    catch (const B& ex)
    {
        test(ex.aMem == 1);
        test(ex.bMem == 2);
    }
    catch (...)
    {
        test(false);
    }

    try
    {
        thrower->throwModA(1, 2);
        test(false);
    }
    catch (const A& ex)
    {
        test(ex.aMem == 1);
    }
    catch (const Ice::OperationNotExistException&)
    {
        //
        // This operation is not supported in Java.
        //
    }
    catch (...)
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
    catch (const B& ex)
    {
        test(ex.aMem == 1);
        test(ex.bMem == 2);
    }
    catch (...)
    {
        test(false);
    }

    try
    {
        thrower->throwCasA(1, 2, 3);
        test(false);
    }
    catch (const C& ex)
    {
        test(ex.aMem == 1);
        test(ex.bMem == 2);
        test(ex.cMem == 3);
    }
    catch (...)
    {
        test(false);
    }

    try
    {
        thrower->throwCasB(1, 2, 3);
        test(false);
    }
    catch (const C& ex)
    {
        test(ex.aMem == 1);
        test(ex.bMem == 2);
        test(ex.cMem == 3);
    }
    catch (...)
    {
        test(false);
    }

    cout << "ok" << endl;

    if (thrower->supportsUndeclaredExceptions())
    {
        cout << "catching unknown user exception... " << flush;

        try
        {
            thrower->throwUndeclaredA(1);
            test(false);
        }
        catch (const Ice::UnknownUserException&)
        {
        }
        catch (const Ice::Exception& ex)
        {
            cout << ex << endl;
            cout << ex.ice_stackTrace() << endl;
            test(false);
        }
        catch (...)
        {
            test(false);
        }

        try
        {
            thrower->throwUndeclaredB(1, 2);
            test(false);
        }
        catch (const Ice::UnknownUserException&)
        {
        }
        catch (...)
        {
            test(false);
        }

        try
        {
            thrower->throwUndeclaredC(1, 2, 3);
            test(false);
        }
        catch (const Ice::UnknownUserException&)
        {
        }
        catch (...)
        {
            test(false);
        }

        cout << "ok" << endl;
    }

    if (thrower->ice_getConnection() && protocol != "bt")
    {
        cout << "testing memory limit marshal exception..." << flush;
        try
        {
            thrower->throwMemoryLimitException(Ice::ByteSeq());
            test(false);
        }
        catch (const Ice::MemoryLimitException&)
        {
        }
        catch (...)
        {
            test(false);
        }

        try
        {
            thrower->throwMemoryLimitException(Ice::ByteSeq(20 * 1024)); // 20KB
            test(false);
        }
        catch (const Ice::ConnectionLostException&)
        {
        }
        catch (const Ice::UnknownLocalException&)
        {
            // Expected with JS bidir server
        }
        catch (const Ice::LocalException& ex)
        {
            cerr << ex << endl;
            test(false);
        }

        try
        {
            ThrowerPrx thrower2(communicator, "thrower:" + helper->getTestEndpoint(1));
            try
            {
                thrower2->throwMemoryLimitException(Ice::ByteSeq(2 * 1024 * 1024)); // 2MB (no limits)
            }
            catch (const Ice::MemoryLimitException&)
            {
            }

            ThrowerPrx thrower3(communicator, "thrower:" + helper->getTestEndpoint(2));
            try
            {
                thrower3->throwMemoryLimitException(Ice::ByteSeq(1024)); // 1KB limit
                test(false);
            }
            catch (const Ice::ConnectionLostException&)
            {
            }
        }
        catch (const Ice::ConnectionRefusedException&)
        {
            // Expected with JS bidir server
        }
        cout << "ok" << endl;
    }

    cout << "catching object not exist exception... " << flush;

    Ice::Identity id = Ice::stringToIdentity("does not exist");
    try
    {
        ThrowerPrx thrower2(thrower->ice_identity(id));
        thrower2->throwAasA(1);
        test(false);
    }
    catch (const Ice::ObjectNotExistException& ex)
    {
        test(ex.id == id);
    }
    catch (...)
    {
        test(false);
    }

    cout << "ok" << endl;

    cout << "catching facet not exist exception... " << flush;

    try
    {
        ThrowerPrx thrower2(thrower->ice_facet("no such facet"));
        try
        {
            thrower2->ice_ping();
            test(false);
        }
        catch (const Ice::FacetNotExistException& ex)
        {
            test(ex.facet == "no such facet");
        }
    }
    catch (...)
    {
        test(false);
    }

    cout << "ok" << endl;

    cout << "catching operation not exist exception... " << flush;

    try
    {
        WrongOperationPrx thrower2(thrower);
        thrower2->noSuchOperation();
        test(false);
    }
    catch (const Ice::OperationNotExistException& ex)
    {
        test(ex.operation == "noSuchOperation");
    }
    catch (...)
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
    catch (const Ice::UnknownLocalException&)
    {
    }
    catch (...)
    {
        test(false);
    }
    try
    {
        thrower->throwLocalExceptionIdempotent();
        test(false);
    }
    catch (const Ice::UnknownLocalException&)
    {
    }
    catch (const Ice::OperationNotExistException&)
    {
    }
    catch (...)
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
    catch (const Ice::UnknownException&)
    {
    }
    catch (...)
    {
        test(false);
    }

    cout << "ok" << endl;

    cout << "testing asynchronous exceptions... " << flush;

    try
    {
        thrower->throwAfterResponse();
    }
    catch (...)
    {
        test(false);
    }

    try
    {
        thrower->throwAfterException();
        test(false);
    }
    catch (const A&)
    {
    }
    catch (...)
    {
        test(false);
    }

    cout << "ok" << endl;

    cout << "catching exact types with new AMI mapping... " << flush;
    {
        auto f = thrower->throwAasAAsync(1);
        try
        {
            f.get();
            test(false);
        }
        catch (const A& ex)
        {
            test(ex.aMem == 1);
        }
        catch (const Ice::Exception&)
        {
            test(false);
        }
        catch (...)
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
        catch (const A& ex)
        {
            test(ex.aMem == 1);
        }
        catch (...)
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
        catch (const D& ex)
        {
            test(ex.dMem == -1);
        }
        catch (...)
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
        catch (const B& ex)
        {
            test(ex.aMem == 1);
            test(ex.bMem == 2);
        }
        catch (...)
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
        catch (const C& ex)
        {
            test(ex.aMem == 1);
            test(ex.bMem == 2);
            test(ex.cMem == 3);
        }
        catch (...)
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
        catch (const A& ex)
        {
            test(ex.aMem == 1);
        }
        catch (const Ice::OperationNotExistException&)
        {
            //
            // This operation is not supported in Java.
            //
        }
        catch (...)
        {
            test(false);
        }
    }

    //
    // repeat with callback API and no exception callback
    //
    {
        promise<bool> sent;
        thrower->throwAasAAsync(1, []() { test(false); }, nullptr, [&](bool value) { sent.set_value(value); });
        sent.get_future().get(); // Wait for sent
    }

    {
        promise<bool> sent;
        thrower->throwAorDasAorDAsync(1, []() { test(false); }, nullptr, [&](bool value) { sent.set_value(value); });
        sent.get_future().get(); // Wait for sent
    }

    {
        promise<bool> sent;
        thrower->throwAorDasAorDAsync(-1, []() { test(false); }, nullptr, [&](bool value) { sent.set_value(value); });
        sent.get_future().get(); // Wait for sent
    }

    {
        promise<bool> sent;
        thrower->throwBasBAsync(1, 2, []() { test(false); }, nullptr, [&](bool value) { sent.set_value(value); });
        sent.get_future().get(); // Wait for sent
    }

    {
        promise<bool> sent;
        thrower->throwCasCAsync(1, 2, 3, []() { test(false); }, nullptr, [&](bool value) { sent.set_value(value); });
        sent.get_future().get(); // Wait for sent
    }

    {
        promise<bool> sent;
        thrower->throwModAAsync(1, 2, []() { test(false); }, nullptr, [&](bool value) { sent.set_value(value); });
        sent.get_future().get(); // Wait for sent
    }
    cout << "ok" << endl;

    cout << "catching derived types with new AMI mapping... " << flush;
    {
        auto f = thrower->throwBasAAsync(1, 2);
        try
        {
            f.get();
        }
        catch (const B& ex)
        {
            test(ex.aMem == 1);
            test(ex.bMem == 2);
        }
        catch (...)
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
        catch (const C& ex)
        {
            test(ex.aMem == 1);
            test(ex.bMem == 2);
            test(ex.cMem == 3);
        }
        catch (...)
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
        catch (const C& ex)
        {
            test(ex.aMem == 1);
            test(ex.bMem == 2);
            test(ex.cMem == 3);
        }
        catch (...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    if (thrower->supportsUndeclaredExceptions())
    {
        cout << "catching unknown user exception with new AMI mapping... " << flush;
        {
            auto f = thrower->throwUndeclaredAAsync(1);
            try
            {
                f.get();
                test(false);
            }
            catch (const Ice::UnknownUserException&)
            {
            }
            catch (const Ice::Exception& ex)
            {
                cout << ex << endl;
                cout << ex.ice_stackTrace() << endl;
                test(false);
            }
            catch (...)
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
            catch (const Ice::UnknownUserException&)
            {
            }
            catch (...)
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
            catch (const Ice::UnknownUserException&)
            {
            }
            catch (...)
            {
                test(false);
            }
        }
        cout << "ok" << endl;
    }

    cout << "catching object not exist exception with new AMI mapping... " << flush;
    {
        id = Ice::stringToIdentity("does not exist");
        ThrowerPrx thrower2(thrower->ice_identity(id));
        auto f = thrower2->throwAasAAsync(1);
        try
        {
            f.get();
        }
        catch (const Ice::ObjectNotExistException& ex)
        {
            test(ex.id == id);
        }
        catch (...)
        {
            test(false);
        }
    }

    cout << "ok" << endl;

    cout << "catching facet not exist exception with new AMI mapping... " << flush;

    {
        ThrowerPrx thrower2(thrower->ice_facet("no such facet"));
        auto f = thrower2->throwAasAAsync(1);
        try
        {
            f.get();
        }
        catch (const Ice::FacetNotExistException& ex)
        {
            test(ex.facet == "no such facet");
        }
    }

    cout << "ok" << endl;

    cout << "catching operation not exist exception with new AMI mapping... " << flush;

    {
        WrongOperationPrx thrower4(thrower);
        auto f = thrower4->noSuchOperationAsync();
        try
        {
            f.get();
        }
        catch (const Ice::OperationNotExistException& ex)
        {
            test(ex.operation == "noSuchOperation");
        }
        catch (...)
        {
            test(false);
        }
    }

    cout << "ok" << endl;

    cout << "catching unknown local exception with new AMI mapping... " << flush;
    {
        auto f = thrower->throwLocalExceptionAsync();
        try
        {
            f.get();
            test(false);
        }
        catch (const Ice::UnknownLocalException&)
        {
        }
        catch (...)
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
        catch (const Ice::UnknownLocalException&)
        {
        }
        catch (const Ice::OperationNotExistException&)
        {
        }
        catch (...)
        {
            test(false);
        }
    }
    cout << "ok" << endl;

    cout << "catching unknown non-Ice exception with new AMI mapping... " << flush;

    {
        auto f = thrower->throwNonIceExceptionAsync();
        try
        {
            f.get();
            test(false);
        }
        catch (const Ice::UnknownException&)
        {
        }
        catch (...)
        {
            test(false);
        }
    }

    cout << "ok" << endl;

    return thrower;
}
