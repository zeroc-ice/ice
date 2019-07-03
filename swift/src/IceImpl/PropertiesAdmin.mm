//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#import "PropertiesAdmin.h"
#import "Convert.h"

@implementation ICEPropertiesAdmin

-(std::shared_ptr<Ice::PropertiesAdmin>) propertiesAdmin
{
    return std::static_pointer_cast<Ice::PropertiesAdmin>(self.cppObject);
}

-(nullable NSString*) getProperty:(NSString*)key error:(NSError**)error
{
    try
    {
        // This function does not use current so we do not pass it from Swift
        return toNSString(self.propertiesAdmin->getProperty(fromNSString(key), Ice::Current{}));
    }
    catch(const std::exception& ex)
    {
        *error = convertException(ex);
        return nil;
    }
}

-(nullable NSDictionary<NSString*, NSString*>*) getPropertiesForPrefix:(NSString*)prefix error:(NSError**)error
{
    try
    {
        // This function does not use current so we do not pass it from Swift
        return toNSDictionary(self.propertiesAdmin->getPropertiesForPrefix(fromNSString(prefix), Ice::Current{}));
    }
    catch(const std::exception& ex)
    {
        *error = convertException(ex);
        return nil;
    }
}

-(BOOL) setProperties:(NSDictionary<NSString*, NSString*>*)newProperties error:(NSError**)error
{
    try
    {
        // This function does not use current so we do not pass it from Swift
        Ice::PropertyDict props;
        fromNSDictionary(newProperties, props);
        self.propertiesAdmin->setProperties(props, Ice::Current{});
        return YES;
    }
    catch(const std::exception& ex)
    {
        *error = convertException(ex);
        return NO;
    }
}

-(void (^)(void)) addUpdateCallback:(void (^)(NSDictionary<NSString*, NSString*>*))cb
{
    auto facet = std::dynamic_pointer_cast<Ice::NativePropertiesAdmin>(self.propertiesAdmin);
    assert(facet);

    auto removeCb = facet->addUpdateCallback([cb] (const Ice::PropertyDict& props)
                                             {
                                                 cb(toNSDictionary(props));
                                             });

    return ^
    {
        removeCb();
    };

}

@end
