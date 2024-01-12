//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#import <ValueFactoryI.h>
#import <LocalObjectI.h>
#import <StreamI.h>
#import <Util.h>

#import <objc/Ice/SlicedData.h>
#import <objc/runtime.h>

@implementation ICEValueFactoryManager
-(id) init:(Ice::Communicator*)com prefixTable:(NSDictionary*)prefixTable
{
    self = [super init];
    if(!self)
    {
        return nil;
    }

    valueFactories_ = [[NSMutableDictionary alloc] init];
    prefixTable_ = [prefixTable retain];

    com->getValueFactoryManager()->add(
        [](const std::string&) -> std::shared_ptr<Ice::Value>
        {
            ICEUnknownSlicedValue* obj = [[ICEUnknownSlicedValue alloc] init];
            std::shared_ptr<Ice::Value> v([ICEInputStream createObjectReader:obj]);
            [obj release];
            return v;
        },
        "::Ice::Object");

    com->getValueFactoryManager()->add(
        [factories = valueFactories_, prefixTable = prefixTable_](const std::string& type) -> std::shared_ptr<Ice::Value>
        {
            NSString* sliceId = [[NSString alloc] initWithUTF8String:type.c_str()];
            NSException* ex = nil;
            @try
            {
                ICEValueFactory factory = nil;
                @synchronized(factories)
                {
                    factory = [factories objectForKey:sliceId];
                    if(factory == nil)
                    {
                        factory = [factories objectForKey:@""];
                    }
                }

                ICEValue* obj = nil;
                if(factory != nil)
                {
                    obj = factory(sliceId);
                }

                if(obj == nil)
                {
                    std::string tId = toObjCSliceId(type, prefixTable);
                    Class c = objc_lookUpClass(tId.c_str());
                    if(c == nil)
                    {
                        return 0; // No value factory.
                    }
                    if([c isSubclassOfClass:[ICEValue class]])
                    {
                        obj = (ICEValue*)[[c alloc] init];
                    }
                }

                std::shared_ptr<Ice::Value> v;
                if(obj != nil)
                {
                    v.reset([ICEInputStream createObjectReader:obj]);
                    [obj release];
                }
                return v;
            }
            @catch(id exc)
            {
                ex = exc;
            }
            @finally
            {
                [sliceId release];
            }
            if(ex != nil)
            {
                rethrowCxxException(ex);
            }
            return std::shared_ptr<Ice::Value>(nullptr); // Keep the compiler happy.

        },
        "");

    return self;
}

-(void) dealloc
{
    [valueFactories_ release];
    [prefixTable_ release];
    [super dealloc];
}

-(void) add:(ICEValueFactory)factory sliceId:(NSString*)sliceId
{
    @synchronized(valueFactories_)
    {
        [valueFactories_ setObject:ICE_AUTORELEASE([factory copy]) forKey:sliceId];
    }
}

-(ICEValueFactory) find:(NSString*)sliceId
{
    @synchronized(valueFactories_)
    {
        return [valueFactories_ objectForKey:sliceId];
    }
    return nil; // Keep the compiler happy.
}

@end
