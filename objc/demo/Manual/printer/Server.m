// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/Ice.h>
#import <Printer.h>
#import <stdio.h>

@interface PrinterI : DemoPrinter <DemoPrinter>
@end

@implementation PrinterI
+(id) printerI
{
    id instance = [[PrinterI alloc] init];
#if defined(__clang__) && !__has_feature(objc_arc)
    [instance autorelease];
#endif
    return instance;
}
-(void) printString:(NSMutableString *)s current:(ICECurrent *)current
{
    printf("%s\n", [s UTF8String]);
    fflush(stdout);
}
@end

int
main(int argc, char* argv[])
{   
    int status = EXIT_FAILURE;
    @autoreleasepool
    {
        id<ICECommunicator> communicator = nil;
        @try
        {
            communicator = [ICEUtil createCommunicator:&argc argv:argv];
            id<ICEObjectAdapter> adapter = [communicator createObjectAdapterWithEndpoints:@"SimplePrinterAdapter"
                                                                                endpoints:@"default -p 10000"];
            [adapter add:[PrinterI printerI] identity:[communicator stringToIdentity:@"SimplePrinter"]];
            [adapter activate];

            [communicator waitForShutdown];

            status = EXIT_SUCCESS;
        }
        @catch (NSException* ex)
        {
            NSLog(@"%@", ex);
        }

        @try
        {
            [communicator destroy];
        }
        @catch (NSException* ex)
        {
            NSLog(@"%@", ex);
        }

    }
    return status;
}
