// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/Ice.h>
#import <TestCommon.h>
#import <HashTest.h>

#import <Foundation/Foundation.h>

void
hashAllTests()
{
    int maxCollisions = 10;
    int maxIterations = 10000;

    {
        tprintf("testing proxy hash algorithm collisions... ");
        int collisions = 0;
        ICEInitializationData* initData = [ICEInitializationData initializationData];
        initData.properties = [ICEUtil createProperties];
        id<ICECommunicator> communicator = [ICEUtil createCommunicator:initData];
        NSMutableDictionary* seenObject = [[NSMutableDictionary alloc] init];
        for(int i = 0; collisions < maxCollisions && i < maxIterations; ++i)
        {
            NSString* s = [NSString stringWithFormat:@"%i:tcp -p %i -t 10%i:udp -p %i -h %i",
                                         i, arc4random() % 65536, arc4random() % 1000000,
                                         arc4random() % 65536, arc4random() % 100];
            ICEObjectPrx* obj = [communicator stringToProxy:s];

            NSNumber* hash = [NSNumber numberWithUnsignedInteger:[obj hash]];

            if([seenObject objectForKey:hash])
            {
                if([obj isEqual:[seenObject objectForKey:hash]])
                {
                    continue;
                }
                ++collisions;
            }
            else
            {
                [seenObject setObject:obj forKey:hash];
            }
            //
            // Test the same object produce always the same hash.
            //
            test([obj hash] == [obj hash]);
            test(collisions < maxCollisions);
        }
        [seenObject removeAllObjects];
        [communicator destroy];
        ICE_RELEASE(seenObject);
        tprintf("ok\n");
    }

    tprintf("testing struct hash algorithm collisions... ");
    {
        int collisions = 0;
        NSMutableDictionary* seenObject = [[NSMutableDictionary alloc] init];
        for(int i = 0; collisions < maxCollisions && i < maxIterations; ++i)
        {
            TestHashPoint* obj = [TestHashPoint point:(arc4random() % 1000)
                                                    y:(arc4random() % 1000)];
            NSNumber* hash = [NSNumber numberWithUnsignedInteger:[obj hash]];
            if([seenObject objectForKey:hash])
            {
                if([obj isEqual:[seenObject objectForKey:hash]])
                {
                    continue;
                }
                ++collisions;
            }
            else
            {
                [seenObject setObject:obj forKey:hash];
            }
            //
            // Test the same object produce always the same hash.
            //
            test([obj hash] == [obj hash]);
            test(collisions < maxCollisions);
        }
        [seenObject removeAllObjects];
        ICE_RELEASE(seenObject);
    }
    {
        int collisions = 0;
        NSMutableDictionary* seenObject = [[NSMutableDictionary alloc] init];
        for(int i = 0; collisions < maxCollisions && i < maxIterations; ++i)
        {
            TestHashPointF* obj = [TestHashPointF pointF:(arc4random() % 1000)/3
                                                       y:(arc4random() % 1000)/5
                                                       z:(arc4random() % 1000)/7];

            NSNumber* hash = [NSNumber numberWithUnsignedInteger:[obj hash]];
            if([seenObject objectForKey:hash])
            {
                if([obj isEqual:[seenObject objectForKey:hash]])
                {
                    continue;
                }
                ++collisions;
            }
            else
            {
                [seenObject setObject:obj forKey:hash];
            }
            //
            // Test the same object produce always the same hash.
            //
            test([obj hash] == [obj hash]);
            test(collisions < maxCollisions);
        }
        [seenObject removeAllObjects];
        ICE_RELEASE(seenObject);
    }

    {
        int collisions = 0;
        NSMutableDictionary* seenObject = [[NSMutableDictionary alloc] init];
        for(int i = 0; collisions < maxCollisions && i < maxIterations; ++i)
        {
            TestHashPointD* obj = [TestHashPointD pointD:(arc4random() % 1000)/3
                                                       y:(arc4random() % 1000)/5
                                                       z:(arc4random() % 1000)/7];
            NSNumber* hash = [NSNumber numberWithUnsignedInteger:[obj hash]];
            if([seenObject objectForKey:hash])
            {
                if([obj isEqual:[seenObject objectForKey:hash]])
                {
                    continue;
                }
                ++collisions;
            }
            else
            {
                [seenObject setObject:obj forKey:hash];
            }
            //
            // Test the same object produce always the same hash.
            //
            test([obj hash] == [obj hash]);
            test(collisions < maxCollisions);
        }
        [seenObject removeAllObjects];
        ICE_RELEASE(seenObject);
    }
    tprintf("ok\n");
}
