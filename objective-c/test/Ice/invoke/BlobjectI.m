// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/Ice.h>
#import <invoke/BlobjectI.h>

@implementation BlobjectI
-(BOOL) ice_invoke:(NSData*)inEncaps outEncaps:(NSMutableData**)outEncaps current:(ICECurrent*)current
{
    id<ICECommunicator> communicator = [current.adapter getCommunicator];
    id<ICEInputStream> inS = [ICEUtil createInputStream:communicator data:inEncaps];
    id<ICEOutputStream> outS = [ICEUtil createOutputStream:communicator];
    [outS startEncapsulation];
    if([current.operation isEqualToString:@"opOneway"])
    {
        return YES;
    }
    else if([current.operation isEqualToString:@"opString"])
    {
        [inS startEncapsulation];
        NSString* s = [inS readString];
        [inS endEncapsulation];
        [outS writeString:s];
        [outS writeString:s];
        [outS endEncapsulation];
        *outEncaps = [outS finished];
        return YES;
    }
    else if([current.operation isEqualToString:@"opException"])
    {
        if([current.ctx objectForKey:@"raise"] != nil)
        {
            @throw [TestInvokeMyException myException];
        }
        TestInvokeMyException* ex = [TestInvokeMyException myException];
        [outS writeException:ex];
        [outS endEncapsulation];
        *outEncaps = [outS finished];
        return NO;
    }
    else if([current.operation isEqualToString:@"shutdown"])
    {
        [outS endEncapsulation];
        *outEncaps = [outS finished];
        [communicator shutdown];
        return YES;
    }
    else if([current.operation isEqualToString:@"ice_isA"])
    {
        [inS startEncapsulation];
        NSString* s = [inS readString];
        [inS endEncapsulation];
        if([s isEqualToString:[TestInvokeMyClass ice_staticId]])
        {
            [outS writeBool:YES];
        }
        else
        {
            [outS writeBool:NO];
        }
        [outS endEncapsulation];
        *outEncaps = [outS finished];
        return YES;
    }
    else
    {
        @throw [ICEOperationNotExistException operationNotExistException:__FILE__
                                              line:__LINE__
                                              id:current.id_
                                              facet:current.facet
                                              operation:current.operation];
    }
}
@end
