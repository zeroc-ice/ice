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
#import <objects/TestI.h>

#if !TARGET_OS_IPHONE
#   import <Foundation/NSGarbageCollector.h>
#endif

// Note that the factory must not autorelease the
// returned objects.
static ICEValueFactory factory = ^ICEObject* (NSString* type)
{
    if([type isEqualToString:@"::Test::B"])
    {
        return  ICE_AUTORELEASE([[TestObjectsBI alloc] init]);
    }
    else if([type isEqualToString:@"::Test::C"])
    {
        return ICE_AUTORELEASE([[TestObjectsCI alloc] init]);
    }
    else if([type isEqualToString:@"::Test::D"])
    {
        return ICE_AUTORELEASE([[TestObjectsDI alloc] init]);
    }
    else if([type isEqualToString:@"::Test::E"])
    {
        return ICE_AUTORELEASE([[TestObjectsEI alloc] init]);
    }
    else if([type isEqualToString:@"::Test::F"])
    {
        return ICE_AUTORELEASE([[TestObjectsFI alloc] init]);
    }
    else if([type isEqualToString:@"::Test::I"])
    {
        return ICE_AUTORELEASE([[TestObjectsI alloc] init]);
    }
    else if([type isEqualToString:@"::Test::J"])
    {
        return ICE_AUTORELEASE([[TestObjectsJI alloc] init]);
    }
    else if([type isEqualToString:@"::Test::H"])
    {
        return ICE_AUTORELEASE([[TestObjectsHI alloc] init]);
    }
    else
    {
        test(NO);
    }
    return nil;
};

@interface ClientMyObjectFactory : NSObject<ICEObjectFactory>
@end

@implementation ClientMyObjectFactory

-(ICEObject*) create:(NSString*)type
{
    return nil;
}

-(void) destroy
{
    // Nothing to do
}
@end

static int
run(id<ICECommunicator> communicator)
{
    id<ICEValueFactoryManager> manager = [communicator getValueFactoryManager];
    [manager add:factory sliceId:@"::Test::B"];
    [manager add:factory sliceId:@"::Test::C"];
    [manager add:factory sliceId:@"::Test::D"];
    [manager add:factory sliceId:@"::Test::E"];
    [manager add:factory sliceId:@"::Test::F"];
    [manager add:factory sliceId:@"::Test::I"];
    [manager add:factory sliceId:@"::Test::J"];
    [manager add:factory sliceId:@"::Test::H"];

    id<ICEObjectFactory> objectFactory = ICE_AUTORELEASE([[ClientMyObjectFactory alloc] init]);
    [communicator addObjectFactory:objectFactory sliceId:@"TestOF" ];

    id<TestObjectsInitialPrx> objectsAllTests(id<ICECommunicator>, bool);
    id<TestObjectsInitialPrx> initial = objectsAllTests(communicator, NO);
    [initial shutdown];
    return EXIT_SUCCESS;
}

#if TARGET_OS_IPHONE
#  define main objectsClient
#endif

int
main(int argc, char* argv[])
{
#ifdef ICE_STATIC_LIBS
    ICEregisterIceSSL(YES);
#if TARGET_OS_IPHONE
    ICEregisterIceIAP(YES);
#endif
#endif

    int status;
    @autoreleasepool
    {
        id<ICECommunicator> communicator = nil;
        @try
        {
            ICEInitializationData* initData = [ICEInitializationData initializationData];
            initData.properties = defaultClientProperties(&argc, argv);
#if TARGET_OS_IPHONE
            initData.prefixTable__ = [NSDictionary dictionaryWithObjectsAndKeys:
                                      @"TestObjects", @"::Test",
                                      nil];
#endif
            communicator = [ICEUtil createCommunicator:&argc argv:argv initData:initData];
            status = run(communicator);
        }
        @catch(ICEException* ex)
        {
            tprintf("%@\n", ex);
            status = EXIT_FAILURE;
        }
        @catch(TestFailedException* ex)
        {
            status = EXIT_FAILURE;
        }

        if(communicator)
        {
            @try
            {
                [communicator destroy];
            }
            @catch(ICEException* ex)
            {
                tprintf("%@\n", ex);
                status = EXIT_FAILURE;
            }
        }
    }
    return status;
}
