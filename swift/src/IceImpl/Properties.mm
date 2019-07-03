//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#import "Properties.h"

#import "Convert.h"

@implementation ICEProperties

-(std::shared_ptr<Ice::Properties>) properties
{
    return std::static_pointer_cast<Ice::Properties>(self.cppObject);
}

-(NSString*) getProperty:(NSString*)key
{
     return toNSString(self.properties->getProperty(fromNSString(key)));
}

-(NSString*) getPropertyWithDefault:(NSString*)key value:(NSString*)value
{
    return toNSString(self.properties->getPropertyWithDefault(fromNSString(key), fromNSString(value)));
}

-(int32_t) getPropertyAsInt:(NSString*)key
{
    return self.properties->getPropertyAsInt(fromNSString(key));
}

-(int32_t) getPropertyAsIntWithDefault:(NSString*)key value:(int32_t)value
{
    return self.properties->getPropertyAsIntWithDefault(fromNSString(key), value);
}

-(NSArray<NSString*>*) getPropertyAsList:(NSString*)key
{
    return toNSArray(self.properties->getPropertyAsList(fromNSString(key)));
}

-(NSArray<NSString*>*) getPropertyAsListWithDefault:(NSString*)key value:(NSArray<NSString*>*)value
{
    std::vector<std::string> s;
    fromNSArray(value, s);
    return toNSArray(self.properties->getPropertyAsListWithDefault(fromNSString(key), s));
}

-(NSDictionary<NSString*, NSString*>*) getPropertiesForPrefix:(NSString*)prefix
{
    return toNSDictionary(self.properties->getPropertiesForPrefix(fromNSString(prefix)));
}

-(BOOL) setProperty:(NSString*)key value:(NSString*)value error:(NSError**)error;
{
    try
    {
        self.properties->setProperty(fromNSString(key), fromNSString(value));
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
    return toNSArray(self.properties->getCommandLineOptions());
}

-(NSArray<NSString*>*) parseCommandLineOptions:(NSString*)prefix options:(NSArray<NSString*>*)options error:(NSError**)error;
{
    try
    {
        std::vector<std::string> s;
        fromNSArray(options, s);
        return toNSArray(self.properties->parseCommandLineOptions(fromNSString(prefix), s));
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
        return toNSArray(self.properties->parseIceCommandLineOptions(s));
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
        self.properties->load(fromNSString(file));
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
    auto props = self.properties->clone();
    return [ICEProperties getHandle:props];
}

@end
