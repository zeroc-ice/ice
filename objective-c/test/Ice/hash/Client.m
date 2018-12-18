// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#import <objc/Ice.h>
#import <TestCommon.h>
#import <HashTest.h>

static int
run()
{
    void hashAllTests(void);
    hashAllTests();
    return EXIT_SUCCESS;
}

#if TARGET_OS_IPHONE
#  define main hashClient
#endif

int
main(int __unused argc, char* __unused argv[])
{
#ifdef ICE_STATIC_LIBS
    ICEregisterIceSSL(YES);
    ICEregisterIceWS(YES);
    ICEregisterIceUDP(YES);
#if TARGET_OS_IPHONE && !TARGET_IPHONE_SIMULATOR
    ICEregisterIceIAP(YES);
#endif
#endif

    int status;
    @autoreleasepool
    {
        @try
        {
            status = run();
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
    }
    return status;
}
