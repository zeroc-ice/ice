// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <ImplicitContextI.h>
#import <Util.h>

@implementation ICEImplicitContext

-(id) init:(Ice::ImplicitContext*)implicitContext
{
    self = [super init];
    if(self)
    {
        self->implicitContext__ = implicitContext;
        self->implicitContext__->__incRef();
    }
    return self;
}

+(id) implicitContextWithImplicitContext:(Ice::ImplicitContext*)implicitContext
{
    if(!implicitContext)
    {
        return nil;
    }
    else
    {
        return [[[ICEImplicitContext alloc] init:implicitContext] autorelease];
    }
}

-(void) dealloc
{
    self->implicitContext__->__decRef();
    [super dealloc];
}

-(ICEContext*) getContext
{
    return [toNSDictionary(implicitContext__->getContext()) autorelease];
}

-(void) setContext:(ICEContext*)context
{
    Ice::Context ctx;
    fromNSDictionary(context, ctx);
    implicitContext__->setContext(ctx);
}

-(BOOL) containsKey:(NSString*)key
{
    return implicitContext__->containsKey(fromNSString(key));
}

-(NSMutableString*) get:(NSString*)key
{
    if(implicitContext__->containsKey(fromNSString(key)))
    {
        return [toNSMutableString(implicitContext__->get(fromNSString(key))) autorelease];
    }
    else
    {
        return nil;
    }
}

-(NSMutableString*) put:(NSString*)key value:(NSString*)value
{
    return [toNSMutableString(implicitContext__->put(fromNSString(key), fromNSString(value))) autorelease];
}

-(NSMutableString*) remove:(NSString*)key
{
    return [toNSMutableString(implicitContext__->remove(fromNSString(key))) autorelease];
}

@end
