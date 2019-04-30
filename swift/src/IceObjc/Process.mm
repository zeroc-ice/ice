//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#import "Process.h"
#import "Util.h"

@implementation ICEProcess

-(std::shared_ptr<Ice::Process>) process
{
    return std::static_pointer_cast<Ice::Process>(self.cppObject);
}

-(void) shutdown
{
    // This fuction does not use current so we do not pass it from Swift
    self.process->shutdown(Ice::Current{});
}

-(void) writeMessage:(NSString*)message fd:(int32_t)fd
{
    // This function does not use current so we do not pass it from Swift
    self.process->writeMessage(fromNSString(message), fd, Ice::Current{});
}

@end
