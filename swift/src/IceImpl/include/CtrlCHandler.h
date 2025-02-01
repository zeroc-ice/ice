// Copyright (c) ZeroC, Inc.

#import "Config.h"

NS_ASSUME_NONNULL_BEGIN

// The implementation of Ice.CtrlCHandler, which wraps the C++ class Ice::CtrlCHandler.
ICEIMPL_API @interface ICECtrlCHandler : NSObject
- (void)receiveSignal:(void (^)(int))callback;
@end

#ifdef __cplusplus

@interface ICECtrlCHandler ()
{
    Ice::CtrlCHandler cppObject;
}
@end

#endif

NS_ASSUME_NONNULL_END
