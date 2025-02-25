// Copyright (c) ZeroC, Inc.

#import "include/CtrlCHandler.h"

#if TARGET_OS_OSX != 0
@implementation ICECtrlCHandler
{
@private
    Ice::CtrlCHandler _cppObject;
}
- (void)setCallback:(void (^)(int))callback
{
    if (callback)
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
    else
    {
        self->_cppObject.setCallback(nullptr);
    }
}
@end
#else
@implementation ICECtrlCHandler
- (void)setCallback:(void (^)(int))callback
{
    assert(false); // CtrlCHandler is not implemented on this platform
}
@end
#endif
