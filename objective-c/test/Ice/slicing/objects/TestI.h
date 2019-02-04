//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#import <SlicingObjectsTestServer.h>

@interface TestSlicingObjectsServerI : TestSlicingObjectsServerTestIntf<TestSlicingObjectsServerTestIntf>
{
@private
    NSMutableArray* objects_;
}
+(id) serverI;
@end
