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
#import <HashTest.h>

static int
run()
{
    void hashAllTests();
    hashAllTests();
    return EXIT_SUCCESS;
}

#if TARGET_OS_IPHONE
#  define main hashClient
#endif

int
main(int argc, char* argv[])
{
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
