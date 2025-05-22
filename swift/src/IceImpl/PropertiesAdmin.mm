// Copyright (c) ZeroC, Inc.

#import "include/PropertiesAdmin.h"
#import "Convert.h"

@implementation ICEPropertiesAdmin
{
@private
    Ice::NativePropertiesAdminPtr _cppPropertiesAdmin;
}

- (instancetype)initWithCppPropertiesAdmin:(Ice::NativePropertiesAdminPtr)cppPropertiesAdmin
{
    assert(cppPropertiesAdmin);
    self = [super init];
    if (!self)
    {
        return nil;
    }

    _cppPropertiesAdmin = std::move(cppPropertiesAdmin);
    return self;
}

- (nullable NSString*)getProperty:(NSString*)key error:(NSError**)error
{
    try
    {
        // This function does not use current so we do not pass it from Swift
        return toNSString(self->_cppPropertiesAdmin->getProperty(fromNSString(key), Ice::Current{}));
    }
    catch (...)
    {
        *error = convertException(std::current_exception());
        return nil;
    }
}

- (nullable NSDictionary<NSString*, NSString*>*)getPropertiesForPrefix:(NSString*)prefix error:(NSError**)error
{
    try
    {
        // This function does not use current so we do not pass it from Swift
        return toNSDictionary(self->_cppPropertiesAdmin->getPropertiesForPrefix(fromNSString(prefix), Ice::Current{}));
    }
    catch (...)
    {
        *error = convertException(std::current_exception());
        return nil;
    }
}

- (BOOL)setProperties:(NSDictionary<NSString*, NSString*>*)newProperties error:(NSError**)error
{
    try
    {
        // This function does not use current so we do not pass it from Swift
        Ice::PropertyDict props;
        fromNSDictionary(newProperties, props);
        self->_cppPropertiesAdmin->setProperties(props, Ice::Current{});
        return YES;
    }
    catch (...)
    {
        *error = convertException(std::current_exception());
        return NO;
    }
}

- (void (^)(void))addUpdateCallback:(void (^)(NSDictionary<NSString*, NSString*>*))cb
{
    auto removeCb = self->_cppPropertiesAdmin->addUpdateCallback([cb](const Ice::PropertyDict& props)
                                                                 { cb(toNSDictionary(props)); });

    return ^{
      removeCb();
    };
}

@end
