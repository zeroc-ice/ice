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
#import <FaultToleranceTest.h>
#import <stdio.h>

void
usage(const char* n)
{
    printf("Usage: %s port\n", n);
}

int
run(int argc, char* argv[], id<ICECommunicator> communicator)
{
    NSMutableArray* ports = [NSMutableArray array];
    int i;
    for(i = 1; i < argc; ++i)
    {
        if(argv[i][0] == '-')
        {
            fprintf(stderr, "%s: unknown option `%s'", argv[0], argv[i]);
            usage(argv[0]);
            return EXIT_FAILURE;
        }

        [ports addObject:[NSNumber numberWithInt:(atoi(argv[i]) + 12010)]];
    }

    if([ports count] == 0)
    {
        fprintf(stderr, "%s: no ports specified", argv[0]);
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    @try
    {
        void allTests(id<ICECommunicator>, NSArray*);
        allTests(communicator, ports);
    }
    @catch(ICEException* ex)
    {
        NSLog(@"%@", ex);
        test(NO);
    }

    return EXIT_SUCCESS;
}

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
            initData.properties = defaultClientProperties(&argc, argv);

            //
            // This test aborts servers, so we don't want warnings.
            //
            [initData.properties setProperty:@"Ice.Warn.Connections" value:@"0"];

            communicator = [ICEUtil createCommunicator:&argc argv:argv initData:initData];

            status = run(argc, argv, communicator);
        }
        @catch(ICEException* ex)
        {
            NSLog(@"%@", ex);
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
                NSLog(@"%@", ex);
                status = EXIT_FAILURE;
            }
        }
    }
    return status;
}
