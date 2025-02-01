// Copyright (c) ZeroC, Inc.

#import "Config.h"

#if TARGET_OS_OSX != 0
NS_ASSUME_NONNULL_BEGIN

// The implementation of Ice.CtrlCHandler, which wraps the C++ class Ice::CtrlCHandler.
ICEIMPL_API @interface ICECtrlCHandler : NSObject
- (void)catchSignal:(void (^)(int))callback;
@end

#    ifdef __cplusplus

@interface ICECtrlCHandler ()
{
    Ice::CtrlCHandler cppObject;
}
@end

#    endif

NS_ASSUME_NONNULL_END
#endif
