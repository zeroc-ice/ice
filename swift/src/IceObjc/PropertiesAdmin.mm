//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#import "IceObjcPropertiesAdmin.h"
#import "IceObjcUtil.h"

@implementation ICEPropertiesAdmin

-(instancetype) initWithCppPropertiesAdmin:(std::shared_ptr<Ice::PropertiesAdmin>)propertiesAdmin
{
    self = [super initWithLocalObject:propertiesAdmin.get()];
    if(!self)
    {
        return nil;
    }
    _propertiesAdmin = propertiesAdmin;
    return self;
}

-(nullable NSString*) getProperty:(NSString*)key error:(NSError**)error
{
    try
    {
        // This function does not use current so we do not pass it from Swift
        return toNSString(_propertiesAdmin->getProperty(fromNSString(key), Ice::Current{}));
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
        return toNSDictionary(_propertiesAdmin->getPropertiesForPrefix(fromNSString(prefix), Ice::Current{}));
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
        _propertiesAdmin->setProperties(props, Ice::Current{});
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
    auto facet = std::dynamic_pointer_cast<Ice::NativePropertiesAdmin>(_propertiesAdmin);
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
