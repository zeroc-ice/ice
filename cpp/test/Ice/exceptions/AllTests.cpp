// Copyright (c) ZeroC, Inc.

#include "Ice/Ice.h"
#include "Test.h"
#include "TestHelper.h"

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
    void deactivate(string_view) final {}
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
    const string protocol = communicator->getProperties()->getIceProperty("Ice.Default.Protocol");

    cout << "testing ice_print()/what() for local exceptions... " << flush;
    {
        Ice::OperationNotExistException opNotExist{"thisFile", 99};
        string opNotExistWhat = "The dispatch failed with reply status OperationNotExist.";
        string opNotExistPrint =
            "thisFile:99 Ice::OperationNotExistException " + opNotExistWhat; // + stack trace in debug builds

        string customMessage = "custom message";
        Ice::UnknownLocalException customUle{"thisFile", 199, customMessage};
        string customUlePrint =
            "thisFile:199 Ice::UnknownLocalException " + customMessage; // + stack trace in debug builds

        {
            ostringstream str;
            opNotExist.ice_print(str);
            string result = str.str();
            test(result.find(opNotExistPrint) == 0);
        }
        {
            ostringstream str;
            customUle.ice_print(str);
            string result = str.str();
            test(result.find(customUlePrint) == 0);
        }

        test(opNotExistWhat == opNotExist.what());
        test(customMessage == customUle.what());
    }
    cout << "ok" << endl;

    string localOAEndpoint;
    {
        ostringstream ostr;
        if (communicator->getProperties()->getIceProperty("Ice.Default.Protocol") == "bt")
        {
            ostr << "default -a *";
        }
        else
        {
            ostr << "default -h *";
        }
        localOAEndpoint = ostr.str();
    }

    if (communicator->getProperties()->getIceProperty("Ice.Default.Protocol") != "ssl" &&
        communicator->getProperties()->getIceProperty("Ice.Default.Protocol") != "wss")
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
            catch (const std::invalid_argument& ex)
            {
                if (printException)
                {
                    Ice::Print printer(communicator->getLogger());
                    printer << ex.what();
                }
            }

            try
            {
                obj = nullptr;
                adapter->add(obj, Ice::stringToIdentity("x"));
            }
            catch (const std::invalid_argument& ex)
            {
                if (printException)
                {
                    Ice::Print printer(communicator->getLogger());
                    printer << ex.what();
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
        catch (const Ice::MarshalException&)
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
            catch (const Ice::MarshalException&)
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
        auto thrower2 = thrower->ice_identity<ThrowerPrx>(id);
        thrower2->throwAasA(1);
        test(false);
    }
    catch (const Ice::ObjectNotExistException& ex)
    {
        test(ex.id() == id);
    }
    catch (...)
    {
        test(false);
    }

    cout << "ok" << endl;

    cout << "catching facet not exist exception... " << flush;

    try
    {
        auto thrower2 = thrower->ice_facet<ThrowerPrx>("no such facet");
        try
        {
            thrower2->ice_ping();
            test(false);
        }
        catch (const Ice::FacetNotExistException& ex)
        {
            test(ex.facet() == "no such facet");
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
        auto thrower2 = Ice::uncheckedCast<WrongOperationPrx>(thrower);
        thrower2->noSuchOperation();
        test(false);
    }
    catch (const Ice::OperationNotExistException& ex)
    {
        test(ex.operation() == "noSuchOperation");
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

    cout << "catching dispatch exception... " << flush;

    try
    {
        thrower->throwDispatchException(static_cast<uint8_t>(Ice::ReplyStatus::OperationNotExist));
        test(false);
    }
    catch (const Ice::OperationNotExistException& ex) // remapped as expected
    {
        // The message is created locally so we don't need a cross-test variant.
        test(
            string{ex.what()} == "Dispatch failed with OperationNotExist { id = 'thrower', facet = '', operation = "
                                 "'throwDispatchException' }");
    }

    try
    {
        thrower->throwDispatchException(static_cast<uint8_t>(Ice::ReplyStatus::Unauthorized));
        test(false);
    }
    catch (const Ice::DispatchException& ex)
    {
        test(ex.replyStatus() == Ice::ReplyStatus::Unauthorized);
        test(
            string{ex.what()} == "The dispatch failed with reply status Unauthorized." ||
            string{ex.what()} == "The dispatch failed with reply status unauthorized."); // for Swift
    }

    try
    {
        thrower.throwDispatchException(212);
        test(false);
    }
    catch (const Ice::DispatchException& ex)
    {
        test(ex.replyStatus() == Ice::ReplyStatus{212});
        test(string{ex.what()} == "The dispatch failed with reply status 212.");
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
        auto thrower2 = thrower->ice_identity<ThrowerPrx>(id);
        auto f = thrower2->throwAasAAsync(1);
        try
        {
            f.get();
        }
        catch (const Ice::ObjectNotExistException& ex)
        {
            test(ex.id() == id);
        }
        catch (...)
        {
            test(false);
        }
    }

    cout << "ok" << endl;

    cout << "catching facet not exist exception with new AMI mapping... " << flush;

    {
        auto thrower2 = thrower->ice_facet<ThrowerPrx>("no such facet");
        auto f = thrower2->throwAasAAsync(1);
        try
        {
            f.get();
        }
        catch (const Ice::FacetNotExistException& ex)
        {
            test(ex.facet() == "no such facet");
        }
    }

    cout << "ok" << endl;

    cout << "catching operation not exist exception with new AMI mapping... " << flush;

    {
        auto thrower4 = Ice::uncheckedCast<WrongOperationPrx>(thrower);
        auto f = thrower4->noSuchOperationAsync();
        try
        {
            f.get();
        }
        catch (const Ice::OperationNotExistException& ex)
        {
            test(ex.operation() == "noSuchOperation");
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

    cout << "catching dispatch exception with new AMI mapping... " << flush;
    {
        auto f = thrower->throwDispatchExceptionAsync(static_cast<uint8_t>(Ice::ReplyStatus::OperationNotExist));
        try
        {
            f.get();
            test(false);
        }
        catch (const Ice::OperationNotExistException& ex) // remapped as expected
        {
            // The message is created locally so we don't need a cross-test variant.
            test(
                string{ex.what()} == "Dispatch failed with OperationNotExist { id = 'thrower', facet = '', operation = "
                                     "'throwDispatchException' }");
        }
    }

    {
        auto f = thrower->throwDispatchExceptionAsync(static_cast<uint8_t>(Ice::ReplyStatus::Unauthorized));
        try
        {
            f.get();
            test(false);
        }
        catch (const Ice::DispatchException& ex)
        {
            test(ex.replyStatus() == Ice::ReplyStatus::Unauthorized);
            test(
                string{ex.what()} == "The dispatch failed with reply status Unauthorized." ||
                string{ex.what()} == "The dispatch failed with reply status unauthorized."); // for Swift
        }
    }

    {
        auto f = thrower->throwDispatchExceptionAsync(212);
        try
        {
            f.get();
            test(false);
        }
        catch (const Ice::DispatchException& ex)
        {
            test(ex.replyStatus() == Ice::ReplyStatus{212});
            test(string{ex.what()} == "The dispatch failed with reply status 212.");
        }
    }
    cout << "ok" << endl;

    return thrower;
}
