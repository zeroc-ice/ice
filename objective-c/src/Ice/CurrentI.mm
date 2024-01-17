//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#import <CurrentI.h>
#import <VersionI.h>
#import <ObjectAdapterI.h>
#import <ConnectionI.h>
#import <IdentityI.h>
#import <Util.h>
#import <LocalObjectI.h>

@implementation ICECurrent (ICEInternal)

-(ICECurrent*) initWithCurrent:(const Ice::Current&)current
{
    self = [super init];
    if(!self)
    {
        return nil;
    }

    adapter = [ICEObjectAdapter objectAdapterWithCxxObjectNoAutoRelease:current.adapter];
    con = [ICEConnection localObjectWithCxxObjectNoAutoRelease:current.con.get()];
    id_ = [[ICEIdentity alloc] initWithIdentity:current.id];
    facet = [[NSString alloc] initWithUTF8String:current.facet.c_str()];
    operation = [[NSString alloc] initWithUTF8String:current.operation.c_str()];
    mode = (ICEOperationMode)current.mode;
    ctx = toNSDictionary(current.ctx);
    requestId = current.requestId;
    encoding = [[ICEEncodingVersion encodingVersionWithEncodingVersion:current.encoding] retain];
    return self;
}
@end
