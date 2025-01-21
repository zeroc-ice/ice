
// Copyright (c) ZeroC, Inc.

#include "Ice/Ice.h"
#include "Test.h"
#include "TestHelper.h"

using namespace std;
using namespace Ice;
using namespace Test;

void
testExceptions(const TestIntfPrx& obj)
{
    try
    {
        obj->requestFailedException();
        test(false);
    }
    catch (const ObjectNotExistException& ex)
    {
        test(ex.id() == obj->ice_getIdentity());
        test(ex.facet() == obj->ice_getFacet());
        test(ex.operation() == "requestFailedException");
    }
    catch (...)
    {
        test(false);
    }

    try
    {
        obj->unknownUserException();
        test(false);
    }
    catch (const UnknownUserException& ex)
    {
        test(string{ex.what()} == "reason");
    }
    catch (...)
    {
        test(false);
    }

    try
    {
        obj->unknownLocalException();
        test(false);
    }
    catch (const UnknownLocalException& ex)
    {
        test(string{ex.what()} == "reason");
    }
    catch (...)
    {
        test(false);
    }

    try
    {
        obj->unknownException();
        test(false);
    }
    catch (const UnknownException& ex)
    {
        test(string{ex.what()} == "reason");
    }
    catch (...)
    {
        test(false);
    }

    try
    {
        obj->userException();
        test(false);
    }
    catch (const UnknownUserException& ex)
    {
        test(string{ex.what()}.find("::Test::TestIntfUserException") != string::npos);
    }
    catch (const OperationNotExistException&)
    {
    }
    catch (...)
    {
        test(false);
    }

    try
    {
        obj->localException();
        test(false);
    }
    catch (const UnknownLocalException& ex)
    {
        test(
            string{ex.what()}.find("SocketException") != string::npos ||
            string{ex.what()}.find("Ice.SocketException") != string::npos);
    }
    catch (...)
    {
        test(false);
    }

    try
    {
        obj->stdException();
        test(false);
    }
    catch (const OperationNotExistException&)
    {
    }
    catch (const UnknownException& ex)
    {
        string message{ex.what()};
        test(
            message == "dispatch failed with std::runtime_error: Hello" ||
            message == "dispatch failed with class std::runtime_error: Hello");
    }
    catch (...)
    {
        test(false);
    }

    try
    {
        obj->cppException();
        test(false);
    }
    catch (const UnknownException& ex)
    {
        test(string{ex.what()} == "dispatch failed with unknown: c++ exception");
    }
    catch (const OperationNotExistException&)
    {
    }
    catch (...)
    {
        test(false);
    }

    try
    {
        obj->unknownExceptionWithServantException();
        test(false);
    }
    catch (const UnknownException& ex)
    {
        test(string{ex.what()} == "reason");
    }
    catch (...)
    {
        test(false);
    }

    try
    {
        obj->impossibleException(false);
        test(false);
    }
    catch (const UnknownUserException&)
    {
        // Operation doesn't throw, but locate() and finished() throw TestIntfUserException.
    }
    catch (...)
    {
        test(false);
    }

    try
    {
        obj->impossibleException(true);
        test(false);
    }
    catch (const UnknownUserException&)
    {
        // Operation doesn't throw, but locate() and finished() throw TestIntfUserException.
    }
    catch (...)
    {
        test(false);
    }

    try
    {
        obj->intfUserException(false);
        test(false);
    }
    catch (const TestImpossibleException&)
    {
        // Operation doesn't throw, but locate() and finished() throw TestImpossibleException.
    }
    catch (...)
    {
        test(false);
    }

    try
    {
        obj->intfUserException(true);
        test(false);
    }
    catch (const TestImpossibleException&)
    {
        // Operation throws TestIntfUserException, but locate() and finished() throw TestImpossibleException.
    }
    catch (...)
    {
        test(false);
    }
}

TestIntfPrx
allTests(Test::TestHelper* helper)
{
    CommunicatorPtr communicator = helper->communicator();
    const string endp = helper->getTestEndpoint();

    TestIntfPrx obj(communicator, "asm:" + endp);

    cout << "testing ice_ids... " << flush;
    try
    {
        ObjectPrx o(communicator, "category/locate:" + endp);
        [[maybe_unused]] vector<string> _ = o->ice_ids();
        test(false);
    }
    catch (const UnknownUserException& ex)
    {
        test(string{ex.what()}.find("::Test::TestIntfUserException") != string::npos);
    }
    catch (...)
    {
        test(false);
    }

    try
    {
        ObjectPrx o(communicator, "category/finished:" + endp);
        [[maybe_unused]] vector<string> _ = o->ice_ids();
        test(false);
    }
    catch (const UnknownUserException& ex)
    {
        test(string{ex.what()}.find("::Test::TestIntfUserException") != string::npos);
    }
    catch (...)
    {
        test(false);
    }
    cout << "ok" << endl;

    cout << "testing servant locator..." << flush;
    obj = TestIntfPrx(communicator, "category/locate:" + endp);
    try
    {
        TestIntfPrx(communicator, "category/unknown:" + endp)->ice_ping();
    }
    catch (const ObjectNotExistException&)
    {
    }
    cout << "ok" << endl;

    cout << "testing default servant locator..." << flush;
    obj = TestIntfPrx(communicator, "anothercategory/locate:" + endp);
    obj = TestIntfPrx(communicator, "locate:" + endp);
    try
    {
        TestIntfPrx(communicator, "anothercategory/unknown:" + endp)->ice_ping();
    }
    catch (const ObjectNotExistException&)
    {
    }
    try
    {
        TestIntfPrx(communicator, "unknown:" + endp)->ice_ping();
    }
    catch (const ObjectNotExistException&)
    {
    }
    cout << "ok" << endl;

    cout << "testing locate exceptions... " << flush;
    obj = TestIntfPrx(communicator, "category/locate:" + endp);
    testExceptions(obj);
    cout << "ok" << endl;

    cout << "testing finished exceptions... " << flush;
    obj = TestIntfPrx(communicator, "category/finished:" + endp);
    testExceptions(obj);

    //
    // Only call these for category/finished.
    //
    try
    {
        obj->asyncResponse();
    }
    catch (const TestIntfUserException&)
    {
        test(false);
    }
    catch (const TestImpossibleException&)
    {
        //
        // Called by finished().
        //
    }

    try
    {
        obj->asyncException();
    }
    catch (const TestIntfUserException&)
    {
        test(false);
    }
    catch (const TestImpossibleException&)
    {
        //
        // Called by finished().
        //
    }

    cout << "ok" << endl;

    cout << "testing servant locator removal... " << flush;
    TestActivationPrx activation(communicator, "test/activation:" + endp);
    activation->activateServantLocator(false);
    try
    {
        obj->ice_ping();
        test(false);
    }
    catch (const ObjectNotExistException&)
    {
        cout << "ok" << endl;
    }
    cout << "testing servant locator addition... " << flush;
    activation->activateServantLocator(true);
    try
    {
        obj->ice_ping();
        cout << "ok" << endl;
    }
    catch (...)
    {
        test(false);
    }
    return obj;
}
