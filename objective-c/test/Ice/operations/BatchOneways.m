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
batchOneways(id<TestOperationsMyClassPrx> p)
{
    ICEByte buf1[10 * 1024];
    TestOperationsMutableByteS *bs1 = [TestOperationsMutableByteS dataWithBytes:buf1 length:sizeof(buf1)];

    id<TestOperationsMyClassPrx> batch = [TestOperationsMyClassPrx uncheckedCast:[p ice_batchOneway]];

    [batch ice_flushBatchRequests]; // Empty flush

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
        id<TestOperationsMyClassPrx> batch1 = [TestOperationsMyClassPrx uncheckedCast:[p ice_batchOneway]];
        id<TestOperationsMyClassPrx> batch2 = [TestOperationsMyClassPrx uncheckedCast:[p ice_batchOneway]];

        [batch1 ice_ping];
        [batch2 ice_ping];
        [batch1 ice_flushBatchRequests];
        [[batch1 ice_getConnection] close:ICECloseGracefullyAndWait];
        [batch1 ice_ping];
        [batch2 ice_ping];

        [batch1 ice_getConnection];
        [batch2 ice_getConnection];

        [batch1 ice_ping];
        [[batch1 ice_getConnection] close:ICECloseGracefullyAndWait];
        [batch1 ice_ping];
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

    if([batch ice_getConnection])
    {
        ICEInitializationData* initData = [ICEInitializationData initializationData];
        initData.properties = [[[p ice_getCommunicator] getProperties] clone];
        __block int _count = 0;
        __block int _size = 0;
        __block int _lastRequestSize = 0;
        __block BOOL _enqueue = NO;
        initData.batchRequestInterceptor = ^(id<ICEBatchRequest> request, int count, int size)
            {
                test([[request getOperation] isEqualToString:@"opByteSOneway"] ||
                     [[request getOperation] isEqualToString:@"ice_ping"]);
                test([[request getProxy] ice_isBatchOneway]);

                if(count > 0)
                {
                    test(_lastRequestSize + _size == size);
                }
                _count = count;
                _size = size;

                if(_size + [request getSize] > 25000)
                {
                    [[request getProxy] begin_ice_flushBatchRequests];
                    _size = 18; // header
                }

                if(_enqueue)
                {
                    _lastRequestSize = [request getSize];
                    ++_count;
                    [request enqueue];
                }
            };

        id<ICECommunicator> ic = [ICEUtil createCommunicator:initData];

        batch = [TestOperationsMyClassPrx uncheckedCast:[[ic stringToProxy:[p ice_toString]] ice_batchOneway]];

        test(_count == 0);
        [batch ice_ping];
        [batch ice_ping];
        [batch ice_ping];
        test(_count == 0);

        _enqueue = YES;
        [batch ice_ping];
        [batch ice_ping];
        [batch ice_ping];
        test(_count == 3);

        [batch ice_flushBatchRequests];
        [batch ice_ping];
        test(_count == 1);

        [batch opByteSOneway:bs1];
        test(_count == 2);
        [batch opByteSOneway:bs1];
        test(_count == 3);

        [batch opByteSOneway:bs1]; // This should trigger the flush
        [batch ice_ping];
        test(_count == 2);

        [ic destroy];
    }

}
