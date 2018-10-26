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
#import <objects/TestI.h>

#if defined(__clang__)
// For 'Ice::Communicator::addObjectFactory()' deprecation
#   pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif

#if defined(__clang__) && __has_feature(objc_arc)
ICEValueFactory factory = ^ICEObject* (NSString* type) NS_RETURNS_RETAINED
#else
ICEValueFactory factory = ^ICEObject* (NSString* type)
#endif
{
    if([type isEqualToString:@"::Test::B"])
    {
        return [[TestObjectsBI alloc] init];
    }
    else if([type isEqualToString:@"::Test::C"])
    {
        return [[TestObjectsCI alloc] init];
    }
    else if([type isEqualToString:@"::Test::D"])
    {
        return [[TestObjectsDI alloc] init];
    }
    else if([type isEqualToString:@"::Test::E"])
    {
        return [[TestObjectsEI alloc] init];
    }
    else if([type isEqualToString:@"::Test::F"])
    {
        return [[TestObjectsFI alloc] init];
    }
    else if([type isEqualToString:@"::Test::I"])
    {
        return [[TestObjectsI alloc] init];
    }
    else if([type isEqualToString:@"::Test::J"])
    {
        return [[TestObjectsJI alloc] init];
    }
    else if([type isEqualToString:@"::Test::H"])
    {
        return [[TestObjectsHI alloc] init];
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

-(ICEObject*) create:(NSString*)__unused type
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
    ICEregisterIceWS(YES);
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
            [initData.properties setProperty:@"Ice.Warn.Dispatch" value:@"0"];
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
