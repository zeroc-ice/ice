// Copyright (c) ZeroC, Inc.
#import "LocalObject.h"

NS_ASSUME_NONNULL_BEGIN

ICEIMPL_API @interface ICEProperties : ICELocalObject
- (NSString*)getProperty:(NSString*)key;
- (NSString*)getIceProperty:(NSString*)key;
- (NSString*)getPropertyWithDefault:(NSString*)key value:(NSString*)value;
- (BOOL)getPropertyAsInt:(NSString*)key value:(int32_t*)value error:(NSError* _Nullable* _Nullable)error;
- (BOOL)getIcePropertyAsInt:(NSString*)key
                      value:(int32_t*)value
                      error:(NSError* _Nullable* _Nullable)error NS_SWIFT_NAME(getIcePropertyAsInt(key:value:));
- (BOOL)getPropertyAsIntWithDefault:(NSString*)key
                       defaultValue:(int32_t)defaultValue
                              value:(int32_t*)value
                              error:(NSError* _Nullable* _Nullable)error
    NS_SWIFT_NAME(getPropertyAsIntWithDefault(key:defaultValue:value:));
- (NSArray<NSString*>*)getPropertyAsList:(NSString* _Nonnull)key;
- (NSArray<NSString*>*)getIcePropertyAsList:(NSString* _Nonnull)key NS_SWIFT_NAME(getIcePropertyAsList(_:));
- (NSArray<NSString*>*)getPropertyAsListWithDefault:(NSString* _Nonnull)key
                                              value:(NSArray<NSString*>* _Nonnull)value
    NS_SWIFT_NAME(getPropertyAsListWithDefault(key:value:));
- (NSDictionary<NSString*, NSString*>*)getPropertiesForPrefix:(NSString* _Nonnull)prefix
    NS_SWIFT_NAME(getPropertiesForPrefix(_:));
- (BOOL)setProperty:(NSString*)key value:(NSString*)value error:(NSError* _Nullable* _Nullable)error;
- (NSArray<NSString*>*)getCommandLineOptions;
- (nullable NSArray<NSString*>*)parseCommandLineOptions:(NSString*)prefix
                                                options:(NSArray<NSString*>*)options
                                                  error:(NSError* _Nullable* _Nullable)error;
- (nullable NSArray<NSString*>*)parseIceCommandLineOptions:(NSArray<NSString*>*)options
                                                     error:(NSError* _Nullable* _Nullable)error;
- (BOOL)load:(NSString*)file error:(NSError* _Nullable* _Nullable)error;
- (ICEProperties*)clone;
@end

#ifdef __cplusplus

@interface ICEProperties ()
@property(nonatomic, readonly) std::shared_ptr<Ice::Properties> properties;
@end

#endif

NS_ASSUME_NONNULL_END
