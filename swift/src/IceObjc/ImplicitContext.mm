// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import "IceObjcImplicitContext.h"

#include "IceObjcUtil.h"

@implementation ICEImplicitContext

-(instancetype) initWithCppImplicitContext:(std::shared_ptr<Ice::ImplicitContext>)implicitContext
{
    self = [super initWithLocalObject:implicitContext.get()];
    if(!self)
    {
        return nil;
    }

    self->_implicitContext = implicitContext;
    return self;
}

-(NSDictionary<NSString*, NSString*>*) getContext
{
    return toNSDictionary(_implicitContext->getContext());
}

-(void) setContext:(NSDictionary<NSString*, NSString*>*)context
{
    Ice::Context c;
    fromNSDictionary(context, c);
    _implicitContext->setContext(c);
}

-(bool) containsKey:(NSString*)string
{
    return _implicitContext->containsKey(fromNSString(string));
}

-(NSString*) get:(NSString*)key
{
    return toNSString(_implicitContext->get(fromNSString(key)));
}

-(NSString*) put:(NSString*)key value:(NSString*)value
{
    return toNSString(_implicitContext->put(fromNSString(key), fromNSString(value)));
}

-(NSString*) remove:(NSString*)key
{
    return toNSString(_implicitContext->remove(fromNSString(key)));
}

@end
