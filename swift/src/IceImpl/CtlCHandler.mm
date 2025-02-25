// Copyright (c) ZeroC, Inc.

#import "include/CtrlCHandler.h"

#if TARGET_OS_OSX != 0
@implementation ICECtrlCHandler
{
@private
    Ice::CtrlCHandler _cppObject;
}
- (BOOL)catchSignal:(void (^)(int))callback
{
    Ice::CtrlCHandlerCallback previousCallback = self->_cppObject.setCallback(
        [callback, self](int signal)
        {
            // This callback executes in the C++ CtrlCHandler thread.

            // We need an autorelease pool in case the callback creates autorelease objects.
            @autoreleasepool
            {
                callback(signal);
            }

            // Then remove callback
            self->_cppObject.setCallback(nullptr);
        });

    return previousCallback == nullptr;
}

- (void)setCallback:(void (^)(int))callback
{
    self->_cppObject.setCallback(
        [callback, self](int signal)
        {
            // This callback executes in the C++ CtrlCHandler thread.

            // We need an autorelease pool in case the callback creates autorelease objects.
            @autoreleasepool
            {
                callback(signal);
            }
        });
}
@end
#else
@implementation ICECtrlCHandler
- (BOOL)catchSignal:(void (^)(int))callback
{
    assert(false); // CtrlCHandler is not implemented on this platform
}
- (void)setCallback:(void (^)(int))callback
{
    assert(false); // CtrlCHandler is not implemented on this platform
}
@end
#endif
