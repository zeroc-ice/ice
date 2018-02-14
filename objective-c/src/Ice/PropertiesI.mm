// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <PropertiesI.h>
#import <Util.h>
#import <LocalObjectI.h>

#include <Ice/NativePropertiesAdmin.h>

@implementation ICEProperties
-(id) initWithCxxObject:(IceUtil::Shared*)cxxObject
{
    self = [super initWithCxxObject:cxxObject];
    if(!self)
    {
        return nil;
    }
    properties_ = dynamic_cast<Ice::Properties*>(cxxObject);
    return self;
}
-(Ice::Properties*) properties
{
    return (Ice::Properties*)properties_;
}

// @protocol ICEProperties methods.

-(NSMutableString*) getProperty:(NSString*)key
{
    NSException* nsex = nil;
    try
    {
        return [toNSMutableString(properties_->getProperty(fromNSString(key))) autorelease];
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
    return nil; // Keep the compiler happy.
}
-(NSMutableString*) getPropertyWithDefault:(NSString*)key value:(NSString*)value
{
    NSException* nsex = nil;
    try
    {
        return [toNSMutableString(properties_->getPropertyWithDefault(fromNSString(key),
                                                                      fromNSString(value))) autorelease];
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
    return nil; // Keep the compiler happy.
}
-(int) getPropertyAsInt:(NSString*)key
{
    NSException* nsex = nil;
    try
    {
        return properties_->getPropertyAsInt(fromNSString(key));
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
    return 0; // Keep the compiler happy.
}
-(int) getPropertyAsIntWithDefault:(NSString*)key value:(int)value
{
    NSException* nsex = nil;
    try
    {
        return properties_->getPropertyAsIntWithDefault(fromNSString(key), value);
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
    return 0; // Keep the compiler happy.
}
-(ICEMutableStringSeq*) getPropertyAsList:(NSString*)key
{
    NSException* nsex = nil;
    try
    {
        return [toNSArray(properties_->getPropertyAsList(fromNSString(key))) autorelease];
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
    return nil; // Keep the compiler happy.
}
-(ICEMutableStringSeq*) getPropertyAsListWithDefault:(NSString*)key value:(NSArray*)value
{
    NSException* nsex = nil;
    try
    {
        std::vector<std::string> s;
        fromNSArray(value, s);
        return [toNSArray(properties_->getPropertyAsListWithDefault(fromNSString(key), s)) autorelease];
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
    return nil; // Keep the compiler happy.
}
-(ICEMutablePropertyDict*) getPropertiesForPrefix:(NSString*)prefix
{
    NSException* nsex = nil;
    try
    {
        return [toNSDictionary(properties_->getPropertiesForPrefix(fromNSString(prefix))) autorelease];
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
    return nil; // Keep the compiler happy.
}
-(void) setProperty:(NSString*)key value:(NSString*)value
{
    NSException* nsex = nil;
    try
    {
        properties_->setProperty(fromNSString(key), fromNSString(value));
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    if(nsex != nil)
    {
        @throw nsex;
    }
}
-(ICEMutableStringSeq*) getCommandLineOptions
{
    NSException* nsex = nil;
    try
    {
        return [toNSArray(properties_->getCommandLineOptions()) autorelease];
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
    return nil; // Keep the compiler happy.
}
-(ICEMutableStringSeq*) parseCommandLineOptions:(NSString*)prefix options:(NSArray*)options
{
    NSException* nsex = nil;
    try
    {
        std::vector<std::string> o;
        fromNSArray(options, o);
        return [toNSArray(properties_->parseCommandLineOptions(fromNSString(prefix), o)) autorelease];
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
    return nil; // Keep the compiler happy.
}
-(ICEMutableStringSeq*) parseIceCommandLineOptions:(NSArray*)options
{
    NSException* nsex = nil;
    try
    {
        std::vector<std::string> o;
        fromNSArray(options, o);
        return [toNSArray(properties_->parseIceCommandLineOptions(o)) autorelease];
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
    return nil; // Keep the compiler happy.
}
-(void) load:(NSString*)file
{
    NSException* nsex = nil;
    try
    {
        properties_->load(fromNSString(file));
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    if(nsex != nil)
    {
        @throw nsex;
    }
}
-(id<ICEProperties>) clone
{
    NSException* nsex = nil;
    try
    {
        return [ICEProperties localObjectWithCxxObject:properties_->clone().get()];
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
    return nil; // Keep the compiler happy.
}

@end

namespace
{

class UpdateCallbackI : public Ice::PropertiesAdminUpdateCallback
{
public:

    UpdateCallbackI(id<ICEPropertiesAdminUpdateCallback> callback) : _callback(callback)
    {
        [_callback retain];
    }

    ~UpdateCallbackI()
    {
        [_callback release];
    }

    void
    updated(const Ice::PropertyDict& properties)
    {
        NSException* ex = nil;
        @autoreleasepool
        {
            @try
            {
                [_callback updated:[toNSDictionary(properties) autorelease]];
            }
            @catch(id e)
            {
                ex = [e retain];
            }
        }
        if(ex != nil)
        {
            rethrowCxxException(ex, true); // True = release the exception.
        }
    }

    id<ICEPropertiesAdminUpdateCallback>
    callback()
    {
        return _callback;
    }

private:

    id<ICEPropertiesAdminUpdateCallback> _callback;
};
typedef IceUtil::Handle<UpdateCallbackI> UpdateCallbackIPtr;

}

@implementation ICEPropertiesAdminUpdateCallback
@end

#define NATIVEPROPERTIESADMIN dynamic_cast<Ice::NativePropertiesAdmin*>(object__)

@implementation ICENativePropertiesAdmin
-(void) addUpdateCallback:(id<ICEPropertiesAdminUpdateCallback>)cb
{
    IceUtil::Mutex::Lock sync(mutex_);
    callbacks_.push_back(new UpdateCallbackI(cb));
    assert(Ice::NativePropertiesAdminPtr::dynamicCast(object__));
    NATIVEPROPERTIESADMIN->addUpdateCallback(callbacks_.back());
}

-(void) removeUpdateCallback:(id<ICEPropertiesAdminUpdateCallback>)cb
{
    IceUtil::Mutex::Lock sync(mutex_);
    for(std::vector<Ice::PropertiesAdminUpdateCallbackPtr>::iterator p = callbacks_.begin(); p != callbacks_.end(); ++p)
    {
        if(UpdateCallbackIPtr::dynamicCast(*p)->callback() == cb)
        {
            NATIVEPROPERTIESADMIN->removeUpdateCallback(*p);
            callbacks_.erase(p);
            return;
        }
    }
}
@end
