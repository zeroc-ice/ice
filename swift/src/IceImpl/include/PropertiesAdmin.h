// Copyright (c) ZeroC, Inc.
#import "Config.h"

NS_ASSUME_NONNULL_BEGIN

ICEIMPL_API @interface ICEPropertiesAdmin : NSObject
- (nullable NSString*)getProperty:(NSString*)key error:(NSError**)error;
- (nullable NSDictionary<NSString*, NSString*>*)getPropertiesForPrefix:(NSString*)prefix error:(NSError**)error;
- (BOOL)setProperties:(NSDictionary<NSString*, NSString*>*)newProperties error:(NSError**)error;
- (void (^)(void))addUpdateCallback:(void (^)(NSDictionary<NSString*, NSString*>*))cb;
@end

#ifdef __cplusplus

@interface ICEPropertiesAdmin ()
- (nonnull instancetype)initWithCppPropertiesAdmin:(Ice::NativePropertiesAdminPtr)cppPropertiesAdmin;
@end

#endif

NS_ASSUME_NONNULL_END
