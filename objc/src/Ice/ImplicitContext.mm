// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice Touch is licensed to you under the terms described in the
// ICE_TOUCH_LICENSE file included in this distribution.
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

-(NSString*) get:(NSString*)key
{
    if(implicitContext__->containsKey(fromNSString(key)))
    {
        return [toNSString(implicitContext__->get(fromNSString(key))) autorelease];
    }
    else
    {
        return nil;
    }
}

-(NSString*) put:(NSString*)key value:(NSString*)value
{
    return [toNSString(implicitContext__->put(fromNSString(key), fromNSString(value))) autorelease];
}

-(NSString*) remove:(NSString*)key
{
    return [toNSString(implicitContext__->remove(fromNSString(key))) autorelease];
}

@end
