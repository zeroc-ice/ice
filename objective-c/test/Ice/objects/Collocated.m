// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/Ice.h>
#import <TestCommon.h>
#import <objects/TestI.h>

#if defined(__clang__)
// For 'Ice::Communicator::addObjectFactory()' deprecation
#   pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif

// Note that the factory must not autorelease the
// returned objects.
ICEValueFactory factory = ^ICEObject* (NSString* type)
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

@interface CollocatedMyObjectFactory : NSObject<ICEObjectFactory>
@end

@implementation CollocatedMyObjectFactory

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

    id<ICEObjectFactory> objectFactory = ICE_AUTORELEASE([[CollocatedMyObjectFactory alloc] init]);
    [communicator addObjectFactory:objectFactory sliceId:@"TestOF" ];

    [[communicator getProperties] setProperty:@"TestAdapter.Endpoints" value:@"default -p 12010"];
    id<ICEObjectAdapter> adapter = [communicator createObjectAdapter:@"TestAdapter"];
    TestObjectsInitialI* initial = [TestObjectsInitialI initial];
    [adapter add:initial identity:[ICEUtil stringToIdentity:@"initial"]];

    ICEObject* testObj = ICE_AUTORELEASE([[TestObjectsTestIntfI alloc] init]);
    [adapter add:testObj identity:[ICEUtil stringToIdentity:@"test"]];

    ICEObject* uoet = ICE_AUTORELEASE([[UnexpectedObjectExceptionTestI alloc] init]);
    [adapter add:uoet identity:[ICEUtil stringToIdentity:@"uoet"]];

    id<TestObjectsInitialPrx> objectsAllTests(id<ICECommunicator>, bool);
    objectsAllTests(communicator, NO);

    // We must call shutdown even in the collocated case for cyclic dependency cleanup
    [initial shutdown:nil];

    return EXIT_SUCCESS;
}

#if TARGET_OS_IPHONE
#  define main objectsCollocated
#endif

int
main(int argc, char* argv[])
{
#ifdef ICE_STATIC_LIBS
    ICEregisterIceSSL(YES);
#if TARGET_OS_IPHONE && !TARGET_IPHONE_SIMULATOR
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
            initData.properties = defaultServerProperties(&argc, argv);
#if TARGET_OS_IPHONE
            initData.prefixTable_ = [NSDictionary dictionaryWithObjectsAndKeys:
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

        if(communicator)
        {
            [communicator destroy];
        }
    }
    return status;
}
