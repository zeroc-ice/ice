// Copyright (c) ZeroC, Inc.

#import "include/Properties.h"
#import "Convert.h"

@implementation ICEProperties

- (std::shared_ptr<Ice::Properties>)properties
{
    return std::static_pointer_cast<Ice::Properties>(self.cppObject);
}

- (NSString*)getProperty:(NSString*)key
{
    return toNSString(self.properties->getProperty(fromNSString(key)));
}

- (NSString*)getIceProperty:(NSString*)key;
{
    // We don't catch exceptions on purpose; in particular, we want Ice::PropertyException to terminate
    // the application.
    return toNSString(self.properties->getIceProperty(fromNSString(key)));
}

- (NSString*)getPropertyWithDefault:(NSString*)key value:(NSString*)value
{
    return toNSString(self.properties->getPropertyWithDefault(fromNSString(key), fromNSString(value)));
}

- (BOOL)getPropertyAsInt:(NSString*)key value:(int32_t*)value error:(NSError**)error
{
    assert(value != nullptr);
    try
    {
        *value = self.properties->getPropertyAsInt(fromNSString(key));
        return YES;
    }
    catch (...)
    {
        *error = convertException(std::current_exception());
        return NO;
    }
}

- (BOOL)getIcePropertyAsInt:(NSString*)key value:(int32_t*)value error:(NSError**)error
{
    assert(value != nullptr);
    try
    {
        *value = self.properties->getIcePropertyAsInt(fromNSString(key));
        return YES;
    }
    catch (...)
    {
        *error = convertException(std::current_exception());
        return NO;
    }
}

- (BOOL)getPropertyAsIntWithDefault:(NSString*)key
                       defaultValue:(int32_t)defaultValue
                              value:(int32_t*)value
                              error:(NSError**)error
{
    try
    {
        *value = self.properties->getPropertyAsIntWithDefault(fromNSString(key), defaultValue);
        return YES;
    }
    catch (...)
    {
        *error = convertException(std::current_exception());
        return NO;
    }
}

- (NSArray<NSString*>*)getPropertyAsList:(NSString*)key
{
    return toNSArray(self.properties->getPropertyAsList(fromNSString(key)));
}

- (NSArray<NSString*>*)getIcePropertyAsList:(NSString*)key
{
    return toNSArray(self.properties->getIcePropertyAsList(fromNSString(key)));
}

- (NSArray<NSString*>*)getPropertyAsListWithDefault:(NSString*)key value:(NSArray<NSString*>*)value
{
    std::vector<std::string> s;
    fromNSArray(value, s);
    return toNSArray(self.properties->getPropertyAsListWithDefault(fromNSString(key), s));
}

- (NSDictionary<NSString*, NSString*>*)getPropertiesForPrefix:(NSString*)prefix
{
    return toNSDictionary(self.properties->getPropertiesForPrefix(fromNSString(prefix)));
}

- (BOOL)setProperty:(NSString*)key value:(NSString*)value error:(NSError**)error;
{
    try
    {
        self.properties->setProperty(fromNSString(key), fromNSString(value));
        return YES;
    }
    catch (...)
    {
        *error = convertException(std::current_exception());
        return NO;
    }
}

- (NSArray<NSString*>*)getCommandLineOptions
{
    return toNSArray(self.properties->getCommandLineOptions());
}

- (NSArray<NSString*>*)parseCommandLineOptions:(NSString*)prefix
                                       options:(NSArray<NSString*>*)options
                                         error:(NSError**)error;
{
    try
    {
        std::vector<std::string> s;
        fromNSArray(options, s);
        return toNSArray(self.properties->parseCommandLineOptions(fromNSString(prefix), s));
    }
    catch (...)
    {
        *error = convertException(std::current_exception());
        return nil;
    }
}

- (NSArray<NSString*>*)parseIceCommandLineOptions:(NSArray<NSString*>*)options error:(NSError**)error;
{
    try
    {
        std::vector<std::string> s;
        fromNSArray(options, s);
        return toNSArray(self.properties->parseIceCommandLineOptions(s));
    }
    catch (...)
    {
        *error = convertException(std::current_exception());
        return nil;
    }
}

- (BOOL)load:(NSString*)file error:(NSError**)error
{
    try
    {
        self.properties->load(fromNSString(file));
        return YES;
    }
    catch (...)
    {
        *error = convertException(std::current_exception());
        return NO;
    }
}

- (ICEProperties*)clone
{
    auto props = self.properties->clone();
    return [ICEProperties getHandle:props];
}

@end
