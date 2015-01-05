// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestCommon.h>
#include <Test.h>

using namespace std;

void
batchOneways(const Test::MyClassPrx& p)
{
    const Test::ByteS bs1(10  * 1024);
    const Test::ByteS bs2(99  * 1024);
    try
    {
        p->opByteSOneway(bs1);
    }
    catch(const Ice::MemoryLimitException&)
    {
        test(false);
    }

    try
    {
        p->opByteSOneway(bs2);
    }
    catch(const Ice::MemoryLimitException&)
    {
        test(false);
    }

    Test::MyClassPrx batch = Test::MyClassPrx::uncheckedCast(p->ice_batchOneway());
    batch->ice_flushBatchRequests();

    int i;
    p->opByteSOnewayCallCount(); // Reset the call count
    for(i = 0 ; i < 30 ; ++i)
    {
        try
        {
            batch->opByteSOneway(bs1);
        }
        catch(const Ice::LocalException&)
        {
            test(false);
        }
    }

    int count = 0;
    while(count < 27) // 3 * 9 requests auto-flushed.
    {
        count += p->opByteSOnewayCallCount();
        IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(10));
    }

    if(batch->ice_getConnection())
    {
        batch->ice_getConnection()->flushBatchRequests();

        Test::MyClassPrx batch2 = Test::MyClassPrx::uncheckedCast(p->ice_batchOneway());

        batch->ice_ping();
        batch2->ice_ping();
        batch->ice_flushBatchRequests();
        batch->ice_getConnection()->close(false);
        batch->ice_ping();
        batch2->ice_ping();

        batch->ice_getConnection();
        batch2->ice_getConnection();

        batch->ice_ping();
        batch->ice_getConnection()->close(false);
        try
        {
            batch->ice_ping();
            test(false);
        }
        catch(const Ice::CloseConnectionException&)
        {
        }
        try
        {
            batch2->ice_ping();
            test(false);
        }
        catch(const Ice::CloseConnectionException&)
        {
        }
        batch->ice_ping();
        batch2->ice_ping();
    }

    Ice::Identity identity;
    identity.name = "invalid";
    Ice::ObjectPrx batch3 = batch->ice_identity(identity);
    batch3->ice_ping();
    batch3->ice_flushBatchRequests();

    // Make sure that a bogus batch request doesn't cause troubles to other ones.
    batch3->ice_ping();
    batch->ice_ping();
    batch->ice_flushBatchRequests();
    batch->ice_ping();
}
