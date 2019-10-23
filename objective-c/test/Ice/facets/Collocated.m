//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#import <objc/Ice.h>
#import <facets/TestI.h>
#import <TestCommon.h>

static int
run(id<ICECommunicator> communicator)
{
    [[communicator getProperties] setProperty:@"TestAdapter.Endpoints" value:getTestEndpoint(communicator, 0)];
    id<ICEObjectAdapter> adapter = [communicator createObjectAdapter:@"TestAdapter"];
    ICEObject* d = [TestFacetsDI d];
    ICEObject* f = [TestFacetsFI f];
    ICEObject* h = [TestFacetsHI h];
    [adapter add:d identity:[ICEUtil stringToIdentity:@"d"]];
    [adapter addFacet:d identity:[ICEUtil stringToIdentity:@"d"] facet:@"facetABCD"];
    [adapter addFacet:f identity:[ICEUtil stringToIdentity:@"d"] facet:@"facetEF"];
    [adapter addFacet:h identity:[ICEUtil stringToIdentity:@"d"] facet:@"facetGH"];

    id<TestFacetsGPrx> facetsAllTests(id<ICECommunicator>);
    facetsAllTests(communicator);

    return EXIT_SUCCESS;
}

#if TARGET_OS_IPHONE
#  define main facetsCollocated
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
#if TARGET_OS_IPHONE
            initData.prefixTable_ = [NSDictionary dictionaryWithObjectsAndKeys:
                                      @"TestFacets", @"::Test",
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
