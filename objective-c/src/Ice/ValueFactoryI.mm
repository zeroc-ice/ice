// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/ObjectFactory.h>

#import <ValueFactoryI.h>
#import <LocalObjectI.h>
#import <StreamI.h>
#import <Util.h>

#import <objc/Ice/ObjectFactory.h>
#import <objc/Ice/SlicedData.h>

#import <objc/runtime.h>

namespace IceObjC
{

class UnknownSlicedValueFactoryI : public Ice::ValueFactory
{
public:

    virtual Ice::ObjectPtr
    create(const std::string&)
    {
        ICEUnknownSlicedValue* obj = [[ICEUnknownSlicedValue alloc] init];
        Ice::ObjectPtr o = [ICEInputStream createObjectReader:obj];
        [obj release];
        return o;
    }
};

class ValueFactoryI : public Ice::ValueFactory
{
public:

    // We must explicitely CFRetain/CFRelease so that the garbage
    // collector does not trash the dictionaries.
    ValueFactoryI(NSDictionary* factories, NSDictionary* prefixTable) :
        _factories(factories), _prefixTable(prefixTable)
    {
        CFRetain(_factories);
        CFRetain(_prefixTable);
    }

    ~ValueFactoryI()
    {
        CFRelease(_factories);
        CFRelease(_prefixTable);
    }

    virtual Ice::ObjectPtr
    create(const std::string& type)
    {
        NSString* sliceId = [[NSString alloc] initWithUTF8String:type.c_str()];
        @try
        {
            ICEValueFactory factory = nil;
            @synchronized(_factories)
            {
                factory = [_factories objectForKey:sliceId];
                if(factory == nil)
                {
                    factory = [_factories objectForKey:@""];
                }
            }

            ICEObject* obj = nil;
            if(factory != nil)
            {
                obj = [factory(sliceId) retain];
            }

            if(obj == nil)
            {
                std::string tId = toObjCSliceId(type, _prefixTable);
                Class c = objc_lookUpClass(tId.c_str());
                if(c == nil)
                {
                    return 0; // No value factory.
                }
                if([c isSubclassOfClass:[ICEObject class]])
                {
                    obj = (ICEObject*)[[c alloc] init];
                }
            }

            Ice::ObjectPtr o;
            if(obj != nil)
            {
                o = [ICEInputStream createObjectReader:obj];
                [obj release];
            }
            return o;
        }
        @catch(id ex)
        {
            rethrowCxxException(ex);
        }
        @finally
        {
            [sliceId release];
        }
        return nil; // Keep the compiler happy.
    }

private:

    NSDictionary* _factories;
    NSDictionary* _prefixTable;
};

}

@implementation ICEValueFactoryManager
-(id) init:(Ice::Communicator*)com prefixTable:(NSDictionary*)prefixTable
{
    self = [super init];
    if(!self)
    {
        return nil;
    }

    valueFactories_ = [[NSMutableDictionary alloc] init];

    com->getValueFactoryManager()->add(new IceObjC::UnknownSlicedValueFactoryI, "::Ice::Object");
    com->getValueFactoryManager()->add(new IceObjC::ValueFactoryI(valueFactories_, prefixTable), "");

    return self;
}

-(void) dealloc
{
    [valueFactories_ release];
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
