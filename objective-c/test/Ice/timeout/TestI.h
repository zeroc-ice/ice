//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#import <TimeoutTest.h>

@interface TimeoutI : TestTimeoutTimeout<TestTimeoutTimeout>
-(void) op:(ICECurrent *)current;
-(void) sendData:(TestTimeoutMutableByteSeq *)seq current:(ICECurrent *)current;
-(void) sleep:(ICEInt)to current:(ICECurrent *)current;
@end

@interface TimeoutControllerI : TestTimeoutController<TestTimeoutController>
{
    id<ICEObjectAdapter> adapter_;
}
+(id) controller:(id<ICEObjectAdapter>)adapter;
-(void) holdAdapter:(ICEInt)to current:(ICECurrent *)current;
-(void) resumeAdapter:(ICECurrent *)current;
-(void) shutdown:(ICECurrent *)current;
@end
