//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#import "IceObjcProcess.h"
#import "IceObjcUtil.h"

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

-(BOOL) shutdown:(NSError**)error
{
    try
    {
        // This fuction does not use current so we do not pass it from Swift
        _process->shutdown(Ice::Current{});
        return YES;
    }
    catch(const std::exception& ex)
    {
        *error = convertException(ex);
        return NO;
    }
}

-(BOOL) writeMessage:(NSString*)message fd:(int32_t)fd error:(NSError**)error
{
    try
    {
        // This function does not use current so we do not pass it from Swift
        _process->writeMessage(fromNSString(message), fd, Ice::Current{});
        return YES;
    }
    catch(const std::exception& ex)
    {
        *error = convertException(ex);
        return NO;
    }
}

@end
