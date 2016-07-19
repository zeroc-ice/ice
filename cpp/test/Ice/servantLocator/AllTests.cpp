// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestCommon.h>
#include <Test.h>
#include <stdexcept>

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
    catch(const ObjectNotExistException& ex)
    {
        test(ex.id == obj->ice_getIdentity());
        test(ex.facet == obj->ice_getFacet());
        test(ex.operation == "requestFailedException");
    }
    catch(...)
    {
        test(false);
    }

    try
    {
        obj->unknownUserException();
        test(false);
    }
    catch(const UnknownUserException& ex)
    {
        test(ex.unknown == "reason");
    }
    catch(...)
    {
        test(false);
    }

    try
    {
        obj->unknownLocalException();
        test(false);
    }
    catch(const UnknownLocalException& ex)
    {
        test(ex.unknown == "reason");
    }
    catch(...)
    {
        test(false);
    }

    try
    {
        obj->unknownException();
        test(false);
    }
    catch(const UnknownException& ex)
    {
        test(ex.unknown == "reason");
    }
    catch(...)
    {
        test(false);
    }

    try
    {
        obj->userException();
        test(false);
    }
    catch(const UnknownUserException& ex)
    {
        test(ex.unknown == "Test::TestIntfUserException");
    }
    catch(const Ice::OperationNotExistException&)
    {
    }
    catch(...)
    {
        test(false);
    }

    try
    {
        obj->localException();
        test(false);
    }
    catch(const UnknownLocalException& ex)
    {
        test(ex.unknown.find("Ice::SocketException") != string::npos ||
             ex.unknown.find("Ice.SocketException") != string::npos);
    }
    catch(...)
    {
        test(false);
    }

    try
    {
        obj->stdException();
        test(false);
    }
    catch(const Ice::OperationNotExistException&)
    {
    }
    catch(const UnknownException& ex)
    {
        test(ex.unknown == "std::exception: Hello");
    }
    catch(...)
    {
        test(false);
    }

    try
    {
        obj->cppException();
        test(false);
    }
    catch(const UnknownException& ex)
    {
        test(ex.unknown == "unknown c++ exception");
    }
    catch(const Ice::OperationNotExistException&)
    {
    }
    catch(...)
    {
        test(false);
    }
    
    try
    {
        obj->unknownExceptionWithServantException();
        test(false);
    }
    catch(const UnknownException& ex)
    {
        test(ex.unknown == "reason");
    }
    catch(...)
    {
        test(false);
    }

    try
    {
        obj->impossibleException(false);
        test(false);
    }
    catch(const UnknownUserException&)
    {
        // Operation doesn't throw, but locate() and finished() throw TestIntfUserException.
    }
    catch(...)
    {
        test(false);
    }

    try
    {
        obj->impossibleException(true);
        test(false);
    }
    catch(const UnknownUserException&)
    {
        // Operation doesn't throw, but locate() and finished() throw TestIntfUserException.
    }
    catch(...)
    {
        test(false);
    }

    try
    {
        obj->intfUserException(false);
        test(false);
    }
    catch(const TestImpossibleException&)
    {
        // Operation doesn't throw, but locate() and finished() throw TestImpossibleException.
    }
    catch(...)
    {
        test(false);
    }

    try
    {
        obj->intfUserException(true);
        test(false);
    }
    catch(const TestImpossibleException&)
    {
        // Operation throws TestIntfUserException, but locate() and finished() throw TestImpossibleException.
    }
    catch(...)
    {
        test(false);
    }
}

TestIntfPrx
allTests(const CommunicatorPtr& communicator)
{
    cout << "testing stringToProxy... " << flush;
    ObjectPrx base = communicator->stringToProxy("asm:default -p 12010");
    test(base);
    cout << "ok" << endl;

    cout << "testing checked cast... " << flush;
    TestIntfPrx obj = TestIntfPrx::checkedCast(base);
    test(obj);
    test(obj == base);
    cout << "ok" << endl;

    cout << "testing ice_ids... " << flush;
    try
    {
        ObjectPrx o = communicator->stringToProxy("category/locate:default -p 12010");
        o->ice_ids();
        test(false);
    }
    catch(const UnknownUserException& ex)
    {
        test(ex.unknown == "Test::TestIntfUserException");
    }
    catch(...)
    {
        test(false);
    }

    try
    {
        ObjectPrx o = communicator->stringToProxy("category/finished:default -p 12010");
        o->ice_ids();
        test(false);
    }
    catch(const UnknownUserException& ex)
    {
        test(ex.unknown == "Test::TestIntfUserException");
    }
    catch(...)
    {
        test(false);
    }
    cout << "ok" << endl;

    cout << "testing servant locator..." << flush;
    base = communicator->stringToProxy("category/locate:default -p 12010");
    obj = TestIntfPrx::checkedCast(base);
    try
    {
        TestIntfPrx::checkedCast(communicator->stringToProxy("category/unknown:default -p 12010"));
    }
    catch(const ObjectNotExistException&)
    {
    }
    cout << "ok" << endl;

    cout << "testing default servant locator..." << flush;
    base = communicator->stringToProxy("anothercategory/locate:default -p 12010");
    obj = TestIntfPrx::checkedCast(base);
    base = communicator->stringToProxy("locate:default -p 12010");
    obj = TestIntfPrx::checkedCast(base);
    try
    {
        TestIntfPrx::checkedCast(communicator->stringToProxy("anothercategory/unknown:default -p 12010"));
    }
    catch(const ObjectNotExistException&)
    {
    }
    try
    {
        TestIntfPrx::checkedCast(communicator->stringToProxy("unknown:default -p 12010"));
    }
    catch(const Ice::ObjectNotExistException&)
    {
    }
    cout << "ok" << endl;

    cout << "testing locate exceptions... " << flush;
    base = communicator->stringToProxy("category/locate:default -p 12010");
    obj = TestIntfPrx::checkedCast(base);
    testExceptions(obj);
    cout << "ok" << endl;

    cout << "testing finished exceptions... " << flush;
    base = communicator->stringToProxy("category/finished:default -p 12010");
    obj = TestIntfPrx::checkedCast(base);
    testExceptions(obj);

    //
    // Only call these for category/finished.
    //
    try
    {
        obj->asyncResponse();
    }
    catch(const TestIntfUserException&)
    {
        test(false);
    }
    catch(const TestImpossibleException&)
    {
        //
        // Called by finished().
        //
    }

    try
    {
        obj->asyncException();
    }
    catch(const TestIntfUserException&)
    {
        test(false);
    }
    catch(const TestImpossibleException&)
    {
        //
        // Called by finished().
        //
    }

    cout << "ok" << endl;

    cout << "testing servant locator removal... " << flush;
    base = communicator->stringToProxy("test/activation:default -p 12010");
    TestActivationPrx activation = TestActivationPrx::checkedCast(base);
    activation->activateServantLocator(false);
    try
    {
        obj->ice_ping();
        test(false);
    }
    catch(const Ice::ObjectNotExistException&)
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
    catch(...)
    {
        test(false);
    }
    return obj;
}

