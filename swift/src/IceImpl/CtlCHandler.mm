// Copyright (c) ZeroC, Inc.

#import "include/CtrlCHandler.h"

@implementation ICECtrlCHandler

- (void)catchSignal:(void (^)(int))callback
{
    [[maybe_unused]] Ice::CtrlCHandlerCallback previousCallback = self->cppObject.setCallback(
        [callback, self](int signal)
        {
            // This callback executes in the C++ CtrlCHandler thread.

            // We need an autorelease pool in case the callback creates autorelease objects.
            @autoreleasepool
            {
                callback(signal);
            }

            // Then remove callback
            self->cppObject.setCallback(nullptr);
        });

    assert(previousCallback == nullptr);
}
@end
