//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#import <objc/Ice.h>
#import <TestCommon.h>
#import <DefaultValueTest.h>

static int
run()
{
    void defaultValueAllTests(void);
    defaultValueAllTests();
    return EXIT_SUCCESS;
}

#if TARGET_OS_IPHONE
#  define main defaultValueClient
#endif

int
main(int __unused argc, char* __unused argv[])
{
#ifdef ICE_STATIC_LIBS
    ICEregisterIceSSL(YES);
    ICEregisterIceWS(YES);
#if TARGET_OS_IPHONE && !TARGET_IPHONE_SIMULATOR
    ICEregisterIceIAP(YES);
#endif
#endif

    @autoreleasepool
    {
        int status;
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
        return status;
    }
}
