//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#import <objc/Ice.h>
#import <TestCommon.h>
#import <objects/TestI.h>

#if !TARGET_OS_IPHONE
#   import <Foundation/NSGarbageCollector.h>
#endif

#if defined(__clang__)
// For 'Ice::Communicator::addObjectFactory()' deprecation
#   pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif

#if defined(__clang__) && __has_feature(objc_arc)
static ICEValueFactory factory = ^ICEObject* (NSString* type) NS_RETURNS_RETAINED
#else
static ICEValueFactory factory = ^ICEObject* (NSString* type)
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

@interface ClientMyObjectFactory : NSObject<ICEObjectFactory>
@end

@implementation ClientMyObjectFactory

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
            initData.properties = defaultClientProperties(&argc, argv);
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
        @catch(TestFailedException* ex)
        {
            status = EXIT_FAILURE;
        }

        if(communicator)
        {
            [communicator destroy];
        }
    }
    return status;
}
