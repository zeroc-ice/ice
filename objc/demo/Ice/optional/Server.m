// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice Touch is licensed to you under the terms described in the
// ICE_TOUCH_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/Ice.h>
#import <ContactDBI.h>


int
run(int argc, char* argv[], id<ICECommunicator> communicator)
{
    if(argc > 1)
    {
        NSLog(@"%s: too many arguments", argv[0]);
        return EXIT_FAILURE;
    }
    id<ICEObjectAdapter> adapter = [communicator createObjectAdapter:@"ContactDB"];
    [adapter add:[ContactDBI contactDBI] identity:[communicator stringToIdentity:@"contactdb"]];
    [adapter activate];
    [communicator waitForShutdown];
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
            [initData.properties load:@"config.server"];

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
