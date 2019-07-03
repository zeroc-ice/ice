//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#import "ImplicitContext.h"
#import "Convert.h"

@implementation ICEImplicitContext

-(std::shared_ptr<Ice::ImplicitContext>) implicitContext
{
    return std::static_pointer_cast<Ice::ImplicitContext>(self.cppObject);
}

-(NSDictionary<NSString*, NSString*>*) getContext
{
    return toNSDictionary(self.implicitContext->getContext());
}

-(void) setContext:(NSDictionary<NSString*, NSString*>*)context
{
    Ice::Context c;
    fromNSDictionary(context, c);
    self.implicitContext->setContext(c);
}

-(bool) containsKey:(NSString*)string
{
    return self.implicitContext->containsKey(fromNSString(string));
}

-(NSString*) get:(NSString*)key
{
    return toNSString(self.implicitContext->get(fromNSString(key)));
}

-(NSString*) put:(NSString*)key value:(NSString*)value
{
    return toNSString(self.implicitContext->put(fromNSString(key), fromNSString(value)));
}

-(NSString*) remove:(NSString*)key
{
    return toNSString(self.implicitContext->remove(fromNSString(key)));
}

@end
