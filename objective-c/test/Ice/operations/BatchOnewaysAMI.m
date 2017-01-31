// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/Ice.h>
#import <TestCommon.h>
#import <OperationsTest.h>

#import <Foundation/Foundation.h>

void
batchOnewaysAMI(id<TestOperationsMyClassPrx> p)
{
    ICEByte buf1[10 * 1024];
    TestOperationsMutableByteS *bs1 = [TestOperationsMutableByteS dataWithBytes:buf1 length:sizeof(buf1)];

    id<TestOperationsMyClassPrx> batch = [TestOperationsMyClassPrx uncheckedCast:[p ice_batchOneway]];

    [batch end_ice_flushBatchRequests:[batch begin_ice_flushBatchRequests]]; // Empty flush
    [[batch begin_ice_flushBatchRequests] isCompleted]; // Empty flush
    [[batch begin_ice_flushBatchRequests] isSent]; // Empty flush
    [[batch begin_ice_flushBatchRequests] sentSynchronously]; // Empty flush

    int i;
    [p opByteSOnewayCallCount];
    for(i = 0 ; i < 30 ; ++i)
    {
        [batch begin_opByteSOneway:bs1];
    }

    int count = 0;
    while(count < 27) // 3 * 9 requests auto-flushed.
    {
        count += [p opByteSOnewayCallCount];
        [NSThread sleepForTimeInterval:10 / 1000.0];
    }

    if([batch ice_getConnection])
    {
        id<TestOperationsMyClassPrx> batch1 = [TestOperationsMyClassPrx uncheckedCast:[p ice_batchOneway]];
        id<TestOperationsMyClassPrx> batch2 = [TestOperationsMyClassPrx uncheckedCast:[p ice_batchOneway]];

        [batch1 end_ice_ping:[batch1 begin_ice_ping]];
        [batch2 end_ice_ping:[batch2 begin_ice_ping]];
        [batch1 end_ice_flushBatchRequests:[batch1 begin_ice_flushBatchRequests]];
        [[batch1 ice_getConnection] close:ICECloseGracefullyAndWait];
        [batch1 end_ice_ping:[batch1 begin_ice_ping]];
        [batch2 end_ice_ping:[batch2 begin_ice_ping]];

        [batch1 ice_getConnection];
        [batch2 ice_getConnection];

        [batch1 end_ice_ping:[batch1 begin_ice_ping]];
        [[batch1 ice_getConnection] close:ICECloseGracefullyAndWait];
        [batch1 end_ice_ping:[batch1 begin_ice_ping]];
        [batch2 end_ice_ping:[batch2 begin_ice_ping]];
    }

    ICEIdentity *identity = [ICEIdentity identity:@"invalid" category:@""];
    id<ICEObjectPrx> batch3 = [batch ice_identity:identity];
    [batch3 begin_ice_ping];
    [batch3 end_ice_flushBatchRequests:[batch3 begin_ice_flushBatchRequests]];

    // Make sure that a bogus batch request doesn't cause troubles to other ones.
    [batch3 begin_ice_ping];
    [batch begin_ice_ping];
    [batch end_ice_flushBatchRequests:[batch begin_ice_flushBatchRequests]];
    [batch begin_ice_ping];
}
