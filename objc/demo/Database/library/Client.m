// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/Ice.h>

#import <stdio.h>


int runParser(int, char**, id<ICECommunicator>);

int
main(int argc, char* argv[])
{
    int status = 0;
    @autoreleasepool
    {
        id<ICECommunicator> communicator = nil;
        @try
        {
            ICEInitializationData* initData = [ICEInitializationData initializationData];
            initData.properties = [ICEUtil createProperties ];
            [initData.properties load: @"config.client" ];
            communicator = [ICEUtil createCommunicator:&argc argv:argv initData:initData];
            if(argc > 1)
            {
                fprintf(stderr, "%s: too many arguments\n", argv[0]);
                return 1;
            }
            runParser(argc, argv, communicator);
        }
        @catch(ICELocalException* ex)
        {
            fprintf(stderr, "%s\n", [[ex description] UTF8String]);
            status = 1;
        }

        if(communicator != nil)
        {
            @try
            {
                [communicator destroy];
            }
            @catch(ICELocalException* ex)
            {
                fprintf(stderr, "%s\n", [[ex description] UTF8String]);
                status = 1;
            }
        }
    }
    return status;
}
