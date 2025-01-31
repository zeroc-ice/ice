// Copyright (c) ZeroC, Inc.

#import "include/CtrlCHandler.h"

@implementation ICECtrlCHandler

- (void)receiveSignal:(void(^)(int))callback
{
    self.cppObject.setCallback(
        [callback, self](int signal)
        {
            // We need an autorelease pool in case the callback creates auto release objects.
            @autoreleasepool
            {
                callback(signal);
            }

            // Then remove callback
            self.cppObject.setCallback(nullptr);
        });
}
@end
