// Copyright (c) ZeroC, Inc.

#import "Config.h"

NS_ASSUME_NONNULL_BEGIN

// The implementation of Ice.CtrlCHandler, which wraps the C++ class Ice::CtrlCHandler (macOS only).
ICEIMPL_API @interface ICECtrlCHandler : NSObject
- (BOOL)catchSignal:(void (^)(int))callback;
- (void)setCallback:(void (^)(int))callback;
@end

NS_ASSUME_NONNULL_END
