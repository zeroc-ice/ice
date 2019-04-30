//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#import "LocalObject.h"

NS_ASSUME_NONNULL_BEGIN

@protocol ICELoggerProtocol
-(void) print:(NSString*)message NS_SWIFT_NAME(print(_:));
-(void) trace:(NSString*)category message:(NSString*)message NS_SWIFT_NAME(trace(category:message:));
-(void) warning:(NSString*)message NS_SWIFT_NAME(warning(_:));
-(void) error:(NSString*)message NS_SWIFT_NAME(error(_:));
-(NSString*) getPrefix;
-(id) cloneWithPrefix:(NSString*)prefix NS_SWIFT_NAME(cloneWithPrefix(_:));
@end

@interface ICELogger: ICELocalObject<ICELoggerProtocol>
-(void) print:(NSString*)message;
-(void) trace:(NSString*)category message:(NSString*)message;
-(void) warning:(NSString*)message;
-(void) error:(NSString*)message;
-(NSString*) getPrefix;
-(id) cloneWithPrefix:(NSString*)prefix;
@end

#ifdef __cplusplus

@interface ICELogger()
@property (nonatomic, readonly) std::shared_ptr<Ice::Logger> logger;
@end

#endif

NS_ASSUME_NONNULL_END
