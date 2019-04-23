//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#import "Process.h"
#import "Util.h"

@implementation ICEProcess

-(instancetype) initWithCppProcess:(std::shared_ptr<Ice::Process>)process
{
    self = [super initWithLocalObject:process.get()];
    if(!self)
    {
        return nil;
    }
    _process = process;
    return self;
}

-(void) shutdown
{
    // This fuction does not use current so we do not pass it from Swift
    _process->shutdown(Ice::Current{});
}

-(void) writeMessage:(NSString*)message fd:(int32_t)fd
{
    // This function does not use current so we do not pass it from Swift
    _process->writeMessage(fromNSString(message), fd, Ice::Current{});
}

@end
