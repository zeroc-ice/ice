// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice Touch is licensed to you under the terms described in the
// ICE_TOUCH_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/Ice.h>
#import <Hello.h>
#import <Router.h>

#import <stdio.h>


void
menu()
{
    printf("usage:\n"
           "t: send greeting as twoway\n"
           "o: send greeting as oneway\n"
           "s: shutdown server\n"
           "x: exit\n"
           "?: help\n");
}

int
run(int argc, char* argv[], id<ICECommunicator> communicator)
{
    if(argc > 1)
    {
        fprintf(stderr, "%s: too many arguments\n", argv[0]);
        return EXIT_FAILURE;
    }

    id<DemoRouterPrx> router = [DemoRouterPrx checkedCast:[communicator getDefaultRouter]];
    [router createSession];

    id<DemoHelloPrx> twoway = [DemoHelloPrx checkedCast:[communicator propertyToProxy:@"Hello.Proxy"]];
    if(!twoway)
    {
        fprintf(stderr, "%s: invalid proxy\n", argv[0]);
        return EXIT_FAILURE;
    }
    id<DemoHelloPrx> oneway = [twoway ice_oneway];

    menu();

    char c = 0;
    do
    {
        @try
        {
            printf("==> ");
            do
            {
                c = getchar();
            }
            while(c != EOF && c == '\n');
            if(c == 't')
            {
                [twoway sayHello:0];
            }
            else if(c == 'o')
            {
                [oneway sayHello:0];
            }
            else if(c == 's')
            {
                [twoway shutdown];
            }
            else if(c == 'x')
            {
                // Nothing to do
            }
            else if(c == '?')
            {
                menu();
            }
            else
            {
                printf("unknown command `%c'\n", c);
                menu();
            }
        }
        @catch(ICEException* ex)
        {
            NSLog(@"%@", ex);
        }
    }
    while(c != EOF && c != 'x');

    return EXIT_SUCCESS;
}

int
main(int argc, char* argv[])
{
    int status = EXIT_SUCCESS;
    @autoreleasepool
    {
        id<ICECommunicator> communicator = nil;
        @try
        {
            ICEInitializationData* initData = [ICEInitializationData initializationData];
            initData.properties = [ICEUtil createProperties];
            [initData.properties load:@"config.client"];

            communicator = [ICEUtil createCommunicator:&argc argv:argv initData:initData];
            status = run(argc, argv, communicator);
        }
        @catch(ICELocalException* ex)
        {
            NSLog(@"%@", ex);
            status = EXIT_FAILURE;
        }

        if(communicator != nil)
        {
            @try
            {
                [communicator destroy];
            }
            @catch(ICELocalException* ex)
            {
                NSLog(@"%@", ex);
                status = EXIT_FAILURE;
            }
        }
    }
    return status;
}
