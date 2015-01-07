// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice Touch is licensed to you under the terms described in the
// ICE_TOUCH_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/Ice.h>
#import <TestCommon.h>
#import <DefaultValueTest.h>
#ifdef ICE_OBJC_GC
#   import <Foundation/NSGarbageCollector.h>
#endif

static int
run()
{
    void defaultValueAllTests();
    defaultValueAllTests();
    return EXIT_SUCCESS;
}

#if TARGET_OS_IPHONE
#  define main defaultValueClient
#endif

int
main(int argc, char* argv[])
{
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
#ifdef ICE_OBJC_GC
        [[NSGarbageCollector defaultCollector] collectExhaustively];
#endif
        return status;
    }
}
