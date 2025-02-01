// Copyright (c) ZeroC, Inc.

#import "Config.h"

NS_ASSUME_NONNULL_BEGIN

// The implementation of Ice.CtrlCHandler, which wraps the C++ class Ice::CtrlCHandler (macOS only).
ICEIMPL_API @interface ICECtrlCHandler : NSObject
- (void)catchSignal:(void (^)(int))callback;
@end

NS_ASSUME_NONNULL_END
