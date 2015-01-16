// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
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
batchOneways(id<TestOperationsMyClassPrx> p)
{
    ICEByte buf1[10 * 1024];
    ICEByte buf2[99 * 1024];
    TestOperationsMutableByteS *bs1 = [TestOperationsMutableByteS dataWithBytes:buf1 length:sizeof(buf1)];
    TestOperationsMutableByteS *bs2 = [TestOperationsMutableByteS dataWithBytes:buf2 length:sizeof(buf2)];

    @try
    {
        [p opByteSOneway:bs1];
    }
    @catch(ICEMemoryLimitException*)
    {
        test(NO);
    }

    @try
    {
        [p opByteSOneway:bs2];
    }
    @catch(ICEMemoryLimitException*)
    {
        test(NO);
    }

    id<TestOperationsMyClassPrx> batch = [TestOperationsMyClassPrx uncheckedCast:[p ice_batchOneway]];
    int i;
    [p opByteSOnewayCallCount];
    for(i = 0 ; i < 30 ; ++i)
    {
        @try
        {
            [batch opByteSOneway:bs1];
        }
        @catch(ICEMemoryLimitException*)
        {
            test(NO);
        }
    }

    int count = 0;
    while(count < 27) // 3 * 9 requests auto-flushed.
    {
        count += [p opByteSOnewayCallCount];
        [NSThread sleepForTimeInterval:10 / 1000.0];
    }

    if([batch ice_getConnection])
    {
        [[batch ice_getConnection] flushBatchRequests];

        id<TestOperationsMyClassPrx> batch2 = [TestOperationsMyClassPrx uncheckedCast:[p ice_batchOneway]];

        [batch ice_ping];
        [batch2 ice_ping];
        [batch ice_flushBatchRequests];
        [[batch ice_getConnection] close:NO];
        [batch ice_ping];
        [batch2 ice_ping];

        [batch ice_getConnection];
        [batch2 ice_getConnection];

        [batch ice_ping];
        [[batch ice_getConnection] close:NO];
        @try
        {
            [batch ice_ping];
            test(NO);
        }
        @catch(ICECloseConnectionException*)
        {
        }
        @try
        {
            [batch2 ice_ping];
            test(NO);
        }
        @catch(ICECloseConnectionException*)
        {
        }
        [batch ice_ping];
        [batch2 ice_ping];
    }

    ICEIdentity *identity = [ICEIdentity identity:@"invalid" category:@""];
    id<ICEObjectPrx> batch3 = [batch ice_identity:identity];
    [batch3 ice_ping];
    [batch3 ice_flushBatchRequests];

    // Make sure that a bogus batch request doesn't cause troubles to other ones.
    [batch3 ice_ping];
    [batch ice_ping];
    [batch ice_flushBatchRequests];
    [batch ice_ping];
}
