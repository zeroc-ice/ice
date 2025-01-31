// Copyright (c) ZeroC, Inc.

#import "Config.h"

NS_ASSUME_NONNULL_BEGIN

ICEIMPL_API @interface ICECtrlCHandler : NSObject
- (void)receiveSignal:(void(^)(int))callback;
@end

#ifdef __cplusplus

@interface ICECtrlCHandler ()
@property(nonatomic, readonly) Ice::CtrlCHandler cppObject;
@end

#endif

NS_ASSUME_NONNULL_END
