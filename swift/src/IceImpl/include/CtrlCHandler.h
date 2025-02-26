// Copyright (c) ZeroC, Inc.

#import "Config.h"

// The implementation of Ice.CtrlCHandler, which wraps the C++ class Ice::CtrlCHandler (macOS only).
ICEIMPL_API @interface ICECtrlCHandler : NSObject
- (void)setCallback:(void (^)(int))callback;
@end
