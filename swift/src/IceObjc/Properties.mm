
// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import "IceObjcProperties.h"

#import "IceObjcUtil.h"

@implementation ICEProperties

-(instancetype) initWithCppProperties:(std::shared_ptr<Ice::Properties>)properties
{
    self = [super initWithLocalObject:properties.get()];
    if(self)
    {
        self->_properties = properties;
    }
    return self;
}

-(NSString*) getProperty:(NSString*)key
{
     return toNSString(_properties->getProperty(fromNSString(key)));
}

-(NSString*) getPropertyWithDefault:(NSString*)key value:(NSString*)value
{
    return toNSString(_properties->getPropertyWithDefault(fromNSString(key), fromNSString(value)));
}

-(int32_t) getPropertyAsInt:(NSString*)key
{
    return _properties->getPropertyAsInt(fromNSString(key));
}

-(int32_t) getPropertyAsIntWithDefault:(NSString*)key value:(int32_t)value
{
    return _properties->getPropertyAsIntWithDefault(fromNSString(key), value);
}

-(NSArray<NSString*>*) getPropertyAsList:(NSString*)key
{
    return toNSArray(_properties->getPropertyAsList(fromNSString(key)));
}

-(NSArray<NSString*>*) getPropertyAsListWithDefault:(NSString*)key value:(NSArray<NSString*>*)value
{
    std::vector<std::string> s;
    fromNSArray(value, s);
    return toNSArray(_properties->getPropertyAsListWithDefault(fromNSString(key), s));
}

-(NSDictionary<NSString*, NSString*>*) getPropertiesForPrefix:(NSString*)prefix
{
    return toNSDictionary(_properties->getPropertiesForPrefix(fromNSString(prefix)));
}

-(BOOL) setProperty:(NSString*)key value:(NSString*)value error:(NSError**)error;
{
    try
    {
        _properties->setProperty(fromNSString(key), fromNSString(value));
        return YES;
    }
    catch(const std::exception& ex)
    {
        *error = convertException(ex);
        return NO;
    }
}

-(NSArray<NSString*>*) getCommandLineOptions
{
    return toNSArray(_properties->getCommandLineOptions());
}

-(NSArray<NSString*>*) parseCommandLineOptions:(NSString*)prefix options:(NSArray<NSString*>*)options error:(NSError**)error;
{
    try
    {
        std::vector<std::string> s;
        fromNSArray(options, s);
        return toNSArray(_properties->parseCommandLineOptions(fromNSString(prefix), s));
    }
    catch(const std::exception& ex)
    {
        *error = convertException(ex);
        return nil;
    }
}

-(NSArray<NSString*>*) parseIceCommandLineOptions:(NSArray<NSString*>*)options error:(NSError**)error;
{
    try
    {
        std::vector<std::string> s;
        fromNSArray(options, s);
        return toNSArray(_properties->parseIceCommandLineOptions(s));
    }
    catch(const std::exception& ex)
    {
        *error = convertException(ex);
        return nil;
    }
}

-(BOOL) load:(NSString*)file error:(NSError**)error
{
    try
    {
        _properties->load(fromNSString(file));
        return YES;
    }
    catch(const std::exception& ex)
    {
        *error = convertException(ex);
        return NO;
    }
}

-(ICEProperties*) clone
{
    auto props = _properties->clone();
    return [[ICEProperties alloc] initWithCppProperties:props];
}

@end
