// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/Ice.h>
#import <objc/Glacier2.h>
#import <objc/IceStorm.h>
#import <objc/IceGrid.h>
#import <TestCommon.h>
#import <ServicesTest.h>

#import <Foundation/Foundation.h>

@interface ClockI : TestServicesClock<TestServicesClock>
-(id) init;
-(void)tick:(NSString*)time current:(ICECurrent*)current;
@end

@implementation ClockI
-(id) init
{
    self = [super init];
    return self;
}
-(void)tick:(NSString*)time current:(ICECurrent*)__unused current
{
    NSLog(@"%@", time);
}
@end

void
servicesAllTests(id<ICECommunicator> communicator)
{
    {
        tprintf("testing Glacier2 stub... ");
        @try
        {
            id<GLACIER2RouterPrx> router = [GLACIER2RouterPrx checkedCast:[communicator getDefaultRouter]];
            [router createSession:@"" password:@""];
        }
        @catch(ICEException* ex)
        {
            test(NO);
        }
        tprintf("ok\n");
    }

    {
        tprintf("testing IceStorm stub... ");
        ICESTORMTopicManagerPrx* manager =
                    [ICESTORMTopicManagerPrx uncheckedCast:[communicator stringToProxy:@"test:default -p 12010"]];

        ICESTORMQoS* qos;
        ICESTORMTopicPrx* topic;
        NSString* topicName = @"time";

        id<ICEObjectAdapter> adapter = [communicator createObjectAdapterWithEndpoints:@"subscriber" endpoints:@"tcp"];
        ICEObjectPrx* subscriber = [adapter addWithUUID:[ClockI clock]];
        [adapter activate];

        @try
        {
            topic = [manager retrieve:topicName];
            [topic subscribeAndGetPublisher:qos subscriber:subscriber];
        }
        @catch(ICESTORMNoSuchTopic*)
        {
            test(NO);
        }
        @catch(ICESTORMAlreadySubscribed*)
        {
            test(NO);
        }
        @catch(ICELocalException*)
        {
        }
        tprintf("ok\n");
    }

    {
        tprintf("testing IceGrid stub... ");
        ICEObjectPrx* base = [communicator stringToProxy:@"test:default -p 12010"];
        ICEGRIDRegistryPrx* registry = [ICEGRIDRegistryPrx uncheckedCast:base];
        ICEGRIDAdminSessionPrx* session;
        ICEGRIDAdminPrx* admin;
        @try
        {
            session = [registry createAdminSession:@"username" password:@"password"];
            test(NO);
            admin = [session getAdmin];
        }
        @catch(ICEGRIDPermissionDeniedException*)
        {
            test(NO);
        }
        @catch(ICELocalException*)
        {
        }
        tprintf("ok\n");
    }
}
