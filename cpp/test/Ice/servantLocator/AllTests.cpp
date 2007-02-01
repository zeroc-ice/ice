// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
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
testExceptions(const TestIntfPrx& obj, bool collocated)
{
    try
    {
        obj->requestFailedException();
        test(false);
    }
    catch(const ObjectNotExistException& ex)
    {
        if(!collocated)
        {
            test(ex.id == obj->ice_getIdentity());
            test(ex.facet == obj->ice_getFacet());
            test(ex.operation == "requestFailedException");
        }
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

    try
    {
        obj->unknownLocalException();
        test(false);
    }
    catch(const UnknownLocalException& ex)
    {
        test(ex.unknown == "reason");
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

    try
    {
        obj->userException();
        test(false);
    }
    catch(const UnknownUserException& ex)
    {
        //cerr << ex.unknown << endl;
        test(!collocated);
        test(ex.unknown == "Test::TestIntfUserException");
    }
    catch(const TestIntfUserException&)
    {
        test(collocated);
    }

    try
    {
        obj->localException();
        test(false);
    }
    catch(const UnknownLocalException& ex)
    {
        //cerr << ex.unknown << endl;
        test(!collocated);
        test(ex.unknown.find("Ice::SocketException:\nsocket exception: unknown error") != string::npos);
    }
    catch(const SocketException&)
    {
        test(collocated);
    }

    try
    {
        obj->stdException();
        test(false);
    }
    catch(const UnknownException& ex)
    {
        //cerr << ex.unknown << endl;
        test(!collocated);
        test(ex.unknown == "std::exception: Hello");
    }
    catch(const std::runtime_error&)
    {
        test(collocated);
    }

    try
    {
        obj->cppException();
        test(false);
    }
    catch(const UnknownException& ex)
    {
        //cerr << ex.unknown << endl;
        test(!collocated);
        test(ex.unknown == "unknown c++ exception");
    }
    catch(const int&)
    {
        test(collocated);
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
}

TestIntfPrx
allTests(const CommunicatorPtr& communicator, bool collocated)
{
    cout << "testing stringToProxy... " << flush;
    ObjectPrx base = communicator->stringToProxy("asm:default -p 12010 -t 10000");
    test(base);
    cout << "ok" << endl;

    cout << "testing checked cast... " << flush;
    TestIntfPrx obj = TestIntfPrx::checkedCast(base);
    test(obj);
    test(obj == base);
    cout << "ok" << endl;

    cout << "testing servant locator..." << flush;
    base = communicator->stringToProxy("category/locate:default -p 12010 -t 10000");
    obj = TestIntfPrx::checkedCast(base);
    try
    {
        TestIntfPrx::checkedCast(communicator->stringToProxy("category/unknown:default -p 12010 -t 10000"));
    }
    catch(const ObjectNotExistException&)
    {
    }
    cout << "ok" << endl;

    cout << "testing default servant locator..." << flush;
    base = communicator->stringToProxy("anothercategory/locate:default -p 12010 -t 10000");
    obj = TestIntfPrx::checkedCast(base);
    base = communicator->stringToProxy("locate:default -p 12010 -t 10000");
    obj = TestIntfPrx::checkedCast(base);
    try
    {
        TestIntfPrx::checkedCast(communicator->stringToProxy("anothercategory/unknown:default -p 12010 -t 10000"));
    }
    catch(const ObjectNotExistException&)
    {
    }
    try
    {
        TestIntfPrx::checkedCast(communicator->stringToProxy("unknown:default -p 12010 -t 10000"));
    }
    catch(const Ice::ObjectNotExistException&)
    {
    }
    cout << "ok" << endl;

    cout << "testing locate exceptions... " << flush;
    base = communicator->stringToProxy("category/locate:default -p 12010 -t 10000");
    obj = TestIntfPrx::checkedCast(base);
    testExceptions(obj, collocated);
    cout << "ok" << endl;

    cout << "testing finished exceptions... " << flush;
    base = communicator->stringToProxy("category/finished:default -p 12010 -t 10000");
    obj = TestIntfPrx::checkedCast(base);
    testExceptions(obj, collocated);
    cout << "ok" << endl;

    return obj;
}

