//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#import "IceObjcLocalObject.h"

NS_ASSUME_NONNULL_BEGIN

@interface ICEPropertiesAdmin: ICELocalObject
-(nullable NSString*) getProperty:(NSString*)key error:(NSError**)error;
-(nullable NSDictionary<NSString*, NSString*>*) getPropertiesForPrefix:(NSString*)prefix error:(NSError**)error;
-(BOOL) setProperties:(NSDictionary<NSString*, NSString*>*)newProperties error:(NSError**)error;
-(void (^)(void)) addUpdateCallback:(void (^)(NSDictionary<NSString*, NSString*>*))cb;
@end

#ifdef __cplusplus

@interface ICEPropertiesAdmin()
@property (nonatomic, readonly) std::shared_ptr<Ice::PropertiesAdmin> propertiesAdmin;
-(instancetype) initWithCppPropertiesAdmin:(std::shared_ptr<Ice::PropertiesAdmin>)propertiesAdmin;
@end

#endif

NS_ASSUME_NONNULL_END
