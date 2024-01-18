//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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

@implementation ICENativePropertiesAdmin
-(void) addUpdateCallback:(id<ICEPropertiesAdminUpdateCallback>)cb
{
    std::lock_guard lock(mutex_);

    std::function<void()> remover = std::dynamic_pointer_cast<Ice::NativePropertiesAdmin>(object_)->addUpdateCallback(
        [cb](const Ice::PropertyDict& properties)
        {
            NSException* ex = nil;
            @autoreleasepool
            {
                @try
                {
                    [cb updated:[toNSDictionary(properties) autorelease]];
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
        });

    callbacks_.push_back(std::make_pair(cb, remover));
    [cb retain];
}

-(void) removeUpdateCallback:(id<ICEPropertiesAdminUpdateCallback>)cb
{
    std::lock_guard lock(mutex_);

    // Each removeUpdateCallback only removes the first occurrence
    auto p = std::find_if(callbacks_.begin(), callbacks_.end(), [cb](const auto& q) { return q.first == cb; });
    if (p != callbacks_.end())
    {
        p->second();
        [cb release];
        callbacks_.erase(p);
    }
}
@end
