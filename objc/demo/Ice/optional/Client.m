// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice Touch is licensed to you under the terms described in the
// ICE_TOUCH_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/Ice.h>
#import <Contact.h>

#import <stdio.h>

int
run(int argc, char* argv[], id<ICECommunicator> communicator)
{
    if(argc > 1)
    {
        fprintf(stderr, "%s: too many arguments\n", argv[0]);
        return EXIT_FAILURE;
    }

    id<DemoContactDBPrx> contactdb = [DemoContactDBPrx checkedCast:[communicator propertyToProxy:@"ContactDB.Proxy"]];
    if(contactdb == nil)
    {
        fprintf(stderr, "%s: invalid proxy\n", argv[0]);
        return EXIT_FAILURE;
    }

    //
    // Add a contact for "john". All parameters are provided.
    //
    NSString* johnNumber = @"123-456-7890";
    [contactdb addContact:@"john" type:@(DemoHOME) number:johnNumber dialGroup:@0];

    printf("Checking john... ");

    //
    // Find the phone number for "john"
    //
    id number = [contactdb queryNumber:@"john"];

    //
    // Compare number to ICENone to check if the optional value is set.
    //
    if(number == ICENone)
    {
    	printf("number is incorrect ");
    }

    //
    // If not ICENone, the optional is the number NSString.
    //
    if(![number isEqual:johnNumber])
    {
    	printf("number is incorrect ");
    }

    // Optional can also be used in an out parameter.
    id dialgroup;
    [contactdb queryDialgroup:@"john" dialGroup:&dialgroup];
    if(dialgroup == ICENone || [dialgroup intValue] != 0)
    {
        printf("dialgroup is incorrect ");
    }

    DemoContact* info = [contactdb query:@"john"];
    //
    // All of the info parameters should be set.
    //
    if(![info hasType] || ![info hasNumber] || ![info hasDialGroup])
    {
    	printf("info is incorrect ");
    }
    else if(info.type != DemoHOME || ![info.number isEqual:johnNumber] || info.dialGroup != 0)
    {
    	printf("info is incorrect ");
    }
    printf("ok\n");

    //
    // Add a contact for "steve". The behavior of the server is to
    // default construct the Contact, and then assign  all set parameters.
    // Since the default value of NumberType in the slice definition
    // is HOME and in this case the NumberType is unset it will take
    // the default value.
    //
    NSString* steveNumber = @"234-567-8901";
    [contactdb addContact:@"steve" type:ICENone number:steveNumber dialGroup:@1];

    printf("Checking steve... ");
    number = [contactdb queryNumber:@"steve"];
    if(![number isEqual:steveNumber])
    {
    	printf("number is incorrect ");
    }

    info = [contactdb query:@"steve"];
    //
    // Check the value for the NumberType.
    //
    if(![info hasType] || info.type != DemoHOME)
    {
    	printf("info is incorrect ");
    }

    if(![info.number isEqual:steveNumber] || info.dialGroup != 1)
    {
    	printf("info is incorrect ");
    }

    [contactdb queryDialgroup:@"steve" dialGroup:&dialgroup];
    if(dialgroup == ICENone || [dialgroup intValue] != 1)
    {
    	printf("dialgroup is incorrect ");
    }

    printf("ok\n");

    //
    // Add a contact from "frank". Here the dialGroup field isn't set.
    //
    NSString* frankNumber = @"345-678-9012";
    [contactdb addContact:@"frank" type:@(DemoCELL) number:frankNumber dialGroup:ICENone];

    printf("Checking frank... ");

    number = [contactdb queryNumber:@"frank"];
    if(![number isEqual:frankNumber])
    {
    	printf("number is incorrect ");
    }

    info = [contactdb query:@"frank"];
    //
    // The dial group field should be unset.
    //
    if([info hasDialGroup])
    {
    	printf("info is incorrect ");
    }
    if(info.type != DemoCELL || ![info.number isEqual:frankNumber])
    {
    	printf("info is incorrect ");
    }

    [contactdb queryDialgroup:@"frank" dialGroup:&dialgroup];
    if(dialgroup != ICENone)
    {
    	printf("dialgroup is incorrect ");
    }
    printf("ok\n");

    //
    // Add a contact from "anne". The number field isn't set.
    //
    [contactdb addContact:@"anne" type:@(DemoOFFICE) number:ICENone dialGroup:@2];

    printf("Checking anne... ");
    number = [contactdb queryNumber:@"anne"];
    if(number != ICENone)
    {
	printf("number is incorrect ");
    }

    info = [contactdb query:@"anne"];
    //
    // The number field should be unset.
    //
    if([info hasNumber])
    {
    	printf("info is incorrect ");
    }
    if(info.type != DemoOFFICE || info.dialGroup != 2)
    {
    	printf("info is incorrect ");
    }
    
    [contactdb queryDialgroup:@"anne" dialGroup:&dialgroup];
    if(dialgroup == ICENone || [dialgroup intValue] != 2)
    {
    	printf("dialgroup is incorrect ");
    }

    //
    // The optional fields can be used to determine what fields to
    // update on the contact.  Here we update only the number for anne,
    // the remainder of the fields are unchanged.
    //
    NSString* anneNumber = @"456-789-0123";
    [contactdb updateContact:@"anne" type:ICENone number:anneNumber dialGroup:ICENone];
    number = [contactdb queryNumber:@"anne"];
    if(![number isEqual:anneNumber])
    {
    	printf("number is incorrect ");
    }
    info = [contactdb query:@"anne"];
    if(![info.number isEqual:anneNumber] || info.type != DemoOFFICE || info.dialGroup != 2)
    {
    	printf("info is incorrect ");
    }
    printf("ok\n");

    [contactdb shutdown];

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
