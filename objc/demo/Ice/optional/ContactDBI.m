// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/Ice.h>

#import <ContactDBI.h>

#include <stdio.h>

@implementation ContactDBI
-(id) init
{
    self = [super init];
    if(self)
    {
        self->contacts_ = [[NSMutableDictionary alloc] init];
    }
    return self;
}

#if defined(__clang__) && !__has_feature(objc_arc)
-(void) dealloc
{
    [self->contacts_ release];
    [super dealloc];
}
#endif

+(id) contactDBI
{
    id instance = [[ContactDBI alloc] init];
#if defined(__clang__) && !__has_feature(objc_arc)
    [instance autorelease];
#endif
    return instance;
}

-(void) addContact:(NSMutableString*)name type:(id)type number:(id)number dialGroup:(id)dialGroup
           current:(ICECurrent*)current
{
    DemoContact* contact = [DemoContact contact];
    contact.name = name;
    if(type != ICENone)
    {
    	contact.type = [type intValue];
    }
    if(number != ICENone)
    {
    	contact.number = number;
    }
    if(dialGroup != ICENone)
    {
    	contact.dialGroup = [dialGroup intValue];
    }
    [contacts_ setObject:contact forKey:name];
}

-(void) updateContact:(NSMutableString*)name type:(id)type number:(id)number dialGroup:(id)dialGroup 
              current:(ICECurrent *)current
{
    DemoContact* contact = [contacts_ objectForKey:name];
    if(contact != nil)
    {
        if(type != ICENone)
        {
            contact.type = [type intValue];
        }
        if(number != ICENone)
        {
            contact.number = number;
        }
        if(dialGroup != ICENone)
        {
            contact.dialGroup = [dialGroup intValue];
        }
    }
}

-(DemoContact*) query:(NSMutableString*)name current:(ICECurrent *)current
{
    return [contacts_ objectForKey:name];
}

-(id) queryNumber:(NSMutableString*)name current:(ICECurrent *)current
{
    DemoContact* contact = [contacts_ objectForKey:name];
    if(contact != nil && [contact hasNumber])
    {
    	return contact.number;
    }
    return ICENone;
}

-(void) queryDialgroup:(NSMutableString*)name dialGroup:(id*)dialGroup current:(ICECurrent *)current
{
    DemoContact* contact = [contacts_ objectForKey:name];
    if(contact != nil && [contact hasDialGroup])
    {
    	*dialGroup = @(contact.dialGroup);
    }
}

-(void) shutdown:(ICECurrent *)current
{
    printf("Shutting down...\n");
    [[current.adapter getCommunicator] shutdown];
}
@end
