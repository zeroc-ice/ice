// Copyright (c) ZeroC, Inc.

#import "include/Process.h"
#import "Convert.h"

@implementation ICEProcess
{
@private
    Ice::ProcessPtr _cppProcess;
}

- (instancetype)initWithCppProcess:(Ice::ProcessPtr)cppProcess
{
    assert(cppProcess);
    self = [super init];
    if (!self)
    {
        return nil;
    }

    _cppProcess = std::move(cppProcess);
    return self;
}

- (void)shutdown
{
    // This function does not use current so we do not pass it from Swift
    self->_cppProcess->shutdown(Ice::Current{});
}

- (void)writeMessage:(NSString*)message fd:(int32_t)fd
{
    // This function does not use current so we do not pass it from Swift
    self->_cppProcess->writeMessage(fromNSString(message), fd, Ice::Current{});
}

@end
