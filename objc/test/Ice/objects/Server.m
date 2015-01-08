// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice Touch is licensed to you under the terms described in the
// ICE_TOUCH_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/Ice.h>
#import <TestCommon.h>
#import <objects/TestI.h>
#ifdef ICE_OBJC_GC
#   import <Foundation/NSGarbageCollector.h>
#endif

@interface ServerMyObjectFactory : NSObject<ICEObjectFactory>
@end

@implementation ServerMyObjectFactory
// Note that the object factory must not autorelease the
// returned objects.
-(ICEObject*) create:(NSString*)type
{
    if([type isEqualToString:@"::Test::I"])
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
        test(NO); // Should never be reached
    }
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
#if defined(__clang__) && !__has_feature(objc_arc)
    id<ICEObjectFactory> factory = [[[ServerMyObjectFactory alloc] init] autorelease];
#else
    id<ICEObjectFactory> factory = [[ServerMyObjectFactory alloc] init];
#endif
    [communicator addObjectFactory:factory sliceId:@"::Test::I"];
    [communicator addObjectFactory:factory sliceId:@"::Test::J"];
    [communicator addObjectFactory:factory sliceId:@"::Test::H"];
    
    [[communicator getProperties] setProperty:@"TestAdapter.Endpoints" value:@"default -p 12010"];
    id<ICEObjectAdapter> adapter = [communicator createObjectAdapter:@"TestAdapter"];
#if defined(__clang__) && !__has_feature(objc_arc)
    ICEObject* initial = [[[TestObjectsInitialI alloc] init] autorelease];
#else
    ICEObject* initial = [[TestObjectsInitialI alloc] init];
#endif
    [adapter add:initial identity:[communicator stringToIdentity:@"initial"]];
    
#if defined(__clang__) && !__has_feature(objc_arc)
    ICEObject* uoet = [[[UnexpectedObjectExceptionTestI alloc] init] autorelease];
#else
    ICEObject* uoet = [[UnexpectedObjectExceptionTestI alloc] init];
#endif
    [adapter add:uoet identity:[communicator stringToIdentity:@"uoet"]];
    [adapter activate];

    serverReady(communicator);

    [communicator waitForShutdown];
    return EXIT_SUCCESS;
}

#if TARGET_OS_IPHONE
#  define main objectsServer
#endif

int
main(int argc, char* argv[])
{
    int status;
    @autoreleasepool
    {
        id<ICECommunicator> communicator = nil;
        @try
        {
            ICEInitializationData* initData = [ICEInitializationData initializationData];
            initData.properties = defaultServerProperties(&argc, argv);
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
#ifdef ICE_OBJC_GC
    [[NSGarbageCollector defaultCollector] collectExhaustively];
#endif
    return status;
}
